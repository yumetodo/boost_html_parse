#include "html_to_xml.hpp"
#include "make_array.hpp"
#include <utility>
#include <cctype>
#include <numeric>
#include <string>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <cwctype>
#include <cwchar>
#include "overload.hpp"
namespace detail {
	using std::size_t;
	size_t get_tag_open_pos(const std::string& str, size_t offset = 0U){ return str.find_first_of('<', offset); }
	size_t get_tag_open_pos(const std::wstring& str, size_t offset = 0U) { return str.find_first_of(L'<', offset); }
	size_t get_tag_close_pos(const std::string& str, size_t offset = 0U) { return str.find_first_of('>', offset); }
	size_t get_tag_close_pos(const std::wstring& str, size_t offset = 0U) { return str.find_first_of(L'>', offset); }
	bool is_open_tag_mark(const char c) { return '<' == c; }
	bool is_open_tag_mark(const wchar_t c) { return L'<' == c; }
	bool is_slash(const char c) { return '/' == c; }
	bool is_slash(const wchar_t c) { return L'/' == c; }
	void convert_to_lower(std::string& str) { for (auto& c : str) c = std::tolower(c); }
	void convert_to_lower(std::wstring& str) { for (auto& c : str) c = std::towlower(c); }

	template<typename char_type, std::enable_if_t<is_char_type<char_type>::value, std::nullptr_t> = nullptr>
	void remove_space_in_tag(std::basic_string<char_type>& str) {
		for (
			size_t open_tag_open_pos = get_tag_open_pos(str);
			std::basic_string<char_type>::npos != open_tag_open_pos;
			open_tag_open_pos = get_tag_open_pos(str, open_tag_open_pos)
		) {
			size_t space_end_pos = open_tag_open_pos + 1;
			while (std::isspace(str[space_end_pos])) ++space_end_pos;
			const size_t space_num = space_end_pos - open_tag_open_pos - 1;
			if (space_num) str.erase(open_tag_open_pos + 1, space_num);
			auto f = std_future::overload(
				[](const std::string& str, size_t offset) -> size_t { return str.find_first_of("/>", offset); },
				[](const std::wstring& str, size_t offset) -> size_t { return str.find_first_of(L"/>", offset); }
			);
			const size_t pos = f(str, space_end_pos);
			if (is_slash(str[pos])) {
				auto add_space = std_future::overload(
					[](std::string& str, size_t pos) { str.insert(pos, 1U, ' '); },
					[](std::wstring& str, size_t pos) { str.insert(pos, 1U, L' '); }
				);
				if (!std::isspace(str[pos - 1]) && !is_open_tag_mark(str[pos - 1])) add_space(str, pos);
				const size_t open_tag_close_pos = get_tag_close_pos(str, pos);
				if (1U < open_tag_close_pos - pos) str.erase(pos + 1, open_tag_close_pos - pos - 1);
				open_tag_open_pos = open_tag_close_pos;
			}
			else {
				open_tag_open_pos = pos;
			}
		}
	}
	template<typename char_type> bool string_match_at_first(const char_type* key, size_t key_len, const std::basic_string<char_type>& str, size_t offset = 0U) noexcept {
		size_t i, j;
		for (i = offset, j = 0; j < key_len && str[i] == key[j]; ++i, ++j);
		return j == key_len;
	}
	bool string_match_at_first(const char* key, const std::string& str, size_t offset = 0U) noexcept {
		return string_match_at_first(key, std::strlen(key), str, offset);
	}
	bool string_match_at_first(const wchar_t* key, const std::wstring& str, size_t offset = 0U) noexcept {
		return string_match_at_first(key, std::wcslen(key), str, offset);
	}

	template<typename char_type> 
	bool string_match_at_first(const std::basic_string<char_type>& key, const std::basic_string<char_type>& str, size_t offset = 0U) noexcept {
		return string_match_at_first(key.c_str(), key.length(), str, offset);
	}
}
namespace html_to_xml {
	using std::size_t;
	using std::string;
	using detail::get_tag_open_pos;
	using detail::get_tag_close_pos;
	using detail::is_open_tag_mark;
	using detail::is_slash;
	void insert_space_slash(std::string& str, size_t pos){ str.insert(pos, " /"); }
	void insert_space_slash(std::wstring& str, size_t pos) { str.insert(pos, L" /"); }

	template<typename char_type, size_t N, std::enable_if_t<is_char_type<char_type>::value, std::nullptr_t> = nullptr> 
	bool tag_match(const std::array<const char_type*, N>& key_arr, const std::basic_string<char_type>& str, size_t offset) noexcept {
		typedef const char_type* c_str;
		return std::any_of(key_arr.begin(), key_arr.end(), [&str, offset](const c_str& s) { return detail::string_match_at_first(s, str, offset); });
	}
	bool tag_match(const std::string& str, size_t offset) noexcept {
		static constexpr auto rep_tag_list = std_future::make_array(
			"area",
			"base", "basefont", "bgsound", "br",
			"col", "command",
			"embed",
			"frame",
			"hr",
			"img", "input", "isindex",
			"link",
			"keygen",
			"meta",
			"param",
			"spacer",
			"wbr"
		);
		return tag_match(rep_tag_list, str, offset);
	}
	bool tag_match(const std::wstring& str, size_t offset) noexcept {
		static constexpr auto rep_tag_list = std_future::make_array(
			L"area",
			L"base", L"basefont", L"bgsound", L"br",
			L"col", L"command",
			L"embed",
			L"frame",
			L"hr",
			L"img", L"input", L"isindex",
			L"link",
			L"keygen",
			L"meta",
			L"param",
			L"spacer",
			L"wbr"
		);
		return tag_match(rep_tag_list, str, offset);
	}

	template<typename char_type, size_t N, std::enable_if_t<is_char_type<char_type>::value, std::nullptr_t> = nullptr>
	void ruby_replace(std::basic_string<char_type>& str, const std::array<const char_type*, N>& in_ruby_tag) {
		auto get_ruby_open_pos = std_future::overload(
			[](const std::string& str, size_t offset = 0U) { return str.find("<ruby", offset); },
			[](const std::wstring& str, size_t offset = 0U) { return str.find(L"<ruby", offset); }
		);
		auto get_ruby_close_pos = std_future::overload(
			[](const std::string& str, size_t offset = 0U) { return str.find("</ruby", offset); },
			[](const std::wstring& str, size_t offset = 0U) { return str.find(L"</ruby", offset); }
		);
		for (
			size_t begin = get_ruby_open_pos(str), end = get_ruby_close_pos(str);
			std::basic_string<char_type>::npos != begin && std::basic_string<char_type>::npos != end;
		) {
			for (auto s : in_ruby_tag) {
				for (size_t tag_pos = str.find(s, begin, end - begin); tag_pos != std::basic_string<char_type>::npos; tag_pos = str.find(s, tag_pos, end - tag_pos)) {
					const size_t open_tag_close_pos = get_tag_close_pos(str, tag_pos);
					if (is_slash(str.at(open_tag_close_pos - 1))) continue;
					auto f = std_future::overload(
						[](const std::string& str, const std::string& key_str, size_t offset) -> size_t { return str.find("</" + key_str, offset); },
						[](const std::wstring& str, const std::wstring& key_str, size_t offset) -> size_t { return str.find(L"</" + key_str, offset); }
					);
					const size_t close_tag_open_pos = std::min({ get_tag_open_pos(str, open_tag_close_pos), f(str, s, open_tag_close_pos) , end });
					if (close_tag_open_pos == end || !is_slash(str[close_tag_open_pos + 1])) insert_space_slash(str, open_tag_close_pos);//<** ~> -> <** ~ />
				}
			}
			begin = get_ruby_open_pos(str, end);
			end = get_ruby_close_pos(str, begin);
		}
	}
	void ruby_replace(std::string& str) {
		static constexpr auto in_ruby_tag = std_future::make_array("<rb", "<rp", "<rt");
		ruby_replace(str, in_ruby_tag);
	}
	void ruby_replace(std::wstring& str) {
		static constexpr auto in_ruby_tag = std_future::make_array(L"<rb", L"<rp", L"<rt");
		ruby_replace(str, in_ruby_tag);
	}
	template<typename char_type, std::enable_if_t<is_char_type<char_type>::value, std::nullptr_t> = nullptr>
	void close_tag(std::basic_string<char_type>& str) {
		for (
			size_t open_tag_open_pos = get_tag_open_pos(str);
			std::basic_string<char_type>::npos != open_tag_open_pos;
			open_tag_open_pos = get_tag_open_pos(str, open_tag_open_pos + 1)
		) {
			if (tag_match(str, open_tag_open_pos + 1)) {
				const size_t open_tag_close_pos = get_tag_close_pos(str, open_tag_open_pos);
				auto cmp = std_future::overload(
					[](const std::string& str, size_t pos) { return std::strcmp("</", str.c_str() + pos); },
					[](const std::wstring& str, size_t pos) { return std::wcscmp(L"</", str.c_str() + pos); }
				);
				if (is_slash(str.at(open_tag_close_pos - 1)) || cmp(str, open_tag_close_pos + 1)) continue;
				insert_space_slash(str, open_tag_close_pos);//<** ~> -> <** ~ />
				open_tag_open_pos = open_tag_close_pos;
			}
		}
	}
	void remove_declaration(std::string& str) {
		//HTML has only 1 declaration before <head>
		size_t declaration_begin_pos = str.find("<!doctype html");
		str.erase(declaration_begin_pos, str.find_first_of('>', declaration_begin_pos));
	}
	void remove_declaration(std::wstring& str) {
		//HTML has only 1 declaration before <head>
		size_t declaration_begin_pos = str.find(L"<!doctype html");
		str.erase(declaration_begin_pos, str.find_first_of(L'>', declaration_begin_pos));
	}
}
void convert_html_to_xml(std::istream& is, std::ostream& os) {
	using std::endl;
	std::string buf;
	//read and remove newline
	for (std::string tmp; std::getline(is, buf);) 
		buf += tmp;//cat
	//convert
	detail::remove_space_in_tag(buf);
	detail::convert_to_lower(buf);
	html_to_xml::remove_declaration(buf);
	html_to_xml::close_tag(buf);
	html_to_xml::ruby_replace(buf);
	//write
	os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;//xml header
	os << buf << std::endl;
}
void convert_html_to_xml(std::wistream& is, std::wostream& os) {
	using std::endl;
	std::wstring buf;
	//read and remove newline
	for (std::wstring tmp; std::getline(is, tmp);)
		buf += tmp;//cat
				   //convert
	detail::remove_space_in_tag(buf);
	detail::convert_to_lower(buf);
	html_to_xml::remove_declaration(buf);
	html_to_xml::close_tag(buf);
	html_to_xml::ruby_replace(buf);
	//write
	os << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;//xml header
	os << buf << std::endl;
}
