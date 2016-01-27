#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <codecvt>
#include <stdexcept>
#include <type_traits>
#include <cstdint>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <vector>
#include "make_array.hpp"
#include "html_to_xml.hpp"
#include "overload.hpp"
namespace detail {
	using std::size_t;
	using std::vector;
	using std::basic_string;
	using boost::property_tree::ptree;
	using boost::property_tree::wptree;
	using boost::property_tree::basic_ptree;
	template<typename char_type>
	using ptree_c = boost::property_tree::basic_ptree<std::basic_string<char_type>, std::basic_string<char_type>>;
	enum class specifier_type : std::uint8_t {
		none_,
		class_,
		id_
	};
	template<typename char_type, std::enable_if_t<is_char_type<char_type>::value, std::nullptr_t> = nullptr>
	struct node_select_piece {
		node_select_piece() = default;
		node_select_piece(const node_select_piece&) = default;
		node_select_piece(node_select_piece&&) = default;
		node_select_piece& operator=(const node_select_piece&) = default;
		node_select_piece& operator=(node_select_piece&&) = default;
		explicit node_select_piece(const std::basic_string<char_type>& str)
			: tag(), type(specifier_type::none_), attr()
		{
			auto f1 = std_future::overload(
				[](const std::string& str) { return str.find_first_of("#."); },
				[](const std::wstring& str) { return str.find_first_of(L"#."); }
			);
			const auto split_pos = f1(str);
			if (basic_string<char_type>::npos == split_pos) {
				tag = str;
			}
			else {
				auto is_sharp = std_future::overload(
					[](const char c) { return '#' == c; },
					[](const wchar_t c) { return L'#' == c; }
				);
				tag = str.substr(0, split_pos);
				type = (is_sharp(str[split_pos])) ? specifier_type::id_ : specifier_type::class_;
				attr = str.substr(split_pos + 1);
			}
		}
		boost::optional<basic_string<char_type>> tag;
		specifier_type type;
		boost::optional<basic_string<char_type>> attr;
	};
	template<typename T> using node_piece_v = vector<node_select_piece<T>>;
	template<typename char_type, std::enable_if_t<is_char_type<char_type>::value, std::nullptr_t> = nullptr>
	node_piece_v<char_type> parse_path(const basic_string<char_type>& path) {
		node_piece_v<char_type> re;
		size_t current = 0;
		for (size_t found; basic_string<char_type>::npos != (found = path.find_first_of(' ', current)); current = found + 1) {
			//call basic_string<char_type> ->node_select_piece<char_type> convert constructor
			re.emplace_back(std::basic_string<char_type>(path, current, found - current));
		}
		auto&& str = path.substr(current, path.size() - current);
		re.emplace_back(std::move(str));
		return re;//expect NRVO
	}
	bool has_attribute(const ptree& p, const std::string& attr_name, const std::string& attr) {
		auto a = p.get_optional<std::string>(u8"<xmlattr>." + attr_name);
		return static_cast<bool>(a) && *a == attr;
	}
	bool has_attribute(const wptree& p, const std::wstring& attr_name, const std::wstring& attr) {
		auto a = p.get_optional<std::wstring>(L"<xmlattr>." + attr_name);
		return static_cast<bool>(a) && *a == attr;
	}
	bool has_attribute(const ptree& p, specifier_type attr_name, const std::string& attr) {//path[layer_level - 1].typeがnoneのときtrue
		return (specifier_type::none_ == attr_name) || has_attribute(p, (specifier_type::class_ == attr_name) ? "class" : "id", attr);
	}
	bool has_attribute(const wptree& p, specifier_type attr_name, const std::wstring& attr) {//path[layer_level - 1].typeがnoneのときtrue
		return (specifier_type::none_ == attr_name) || has_attribute(p, (specifier_type::class_ == attr_name) ? L"class" : L"id", attr);
	}
	template<typename char_type, std::enable_if_t<is_char_type<char_type>::value, std::nullptr_t> = nullptr>
	boost::optional<size_t> tag_search(const basic_string<char_type>& key, const node_piece_v<char_type>& path, const std::size_t layer_level) {
		for (auto i = layer_level; 0 < i; --i) {
			if (path[i].tag == key) return i;
		}
		return (path.front().tag == key) ? size_t{} : boost::optional<size_t>{};//none
	}
	template<typename char_type, std::enable_if_t<is_char_type<char_type>::value, std::nullptr_t> = nullptr>
	void html_extract_impl(vector<basic_string<char_type>>& re, const ptree_c<char_type>& pt, const node_piece_v<char_type>& path, size_t layer_level = 0U) {//1~
		if (pt.size()) {//childがあるか
			for (auto& p : pt) {
				const boost::optional<size_t> search_re = tag_search(p.first, path, layer_level);
				if (search_re) {
					const detail::node_select_piece<char_type>& n = path[*search_re];
					if (!n.attr || has_attribute(p.second, n.type, n.attr.get())) {
						if (path.size() == layer_level + 1) {
							re.push_back(p.second.data());
						}
						else {
							html_extract_impl(re, p.second, path, *search_re + 1);//recursion
						}
					}
				}
				else {
					html_extract_impl(re, p.second, path, layer_level);//recursion
				}
			}
		}
	}
	template<typename CharType>void skip_utf8_bom(std::basic_ifstream<CharType>& fs) {
		int dst[3];
		for (auto& i : dst) i = fs.get();
		constexpr int utf8[] = { 0xEF, 0xBB, 0xBF };
		if (std::equal(std::begin(dst), std::end(dst), utf8)) fs.seekg(0);
	}
}
std::vector<std::string> html_extract(const std::string& filename, const std::string& path_str) {
	if (!path_str.length()) return std::vector<std::string>();
	std::vector<std::string> re;
	boost::property_tree::ptree pt;
	std::stringstream ss;
	std::ifstream file(filename);
	if (!file) throw std::runtime_error("cannot open file");
	file.imbue(std::locale());
	detail::skip_utf8_bom(file);
	convert_html_to_xml(file, ss);
	read_xml(ss, pt, boost::property_tree::xml_parser::no_comments);
	const auto path = detail::parse_path(path_str);
	auto& body = pt.get_child(u8"body");//HTML has body tag. if not exist, exception will be thrown.
	detail::html_extract_impl(re, body, path);//analyse
	return re;
}
std::vector<std::wstring> html_extract(const std::wstring& filename, const std::wstring& path_str) {
	if (!path_str.length()) return std::vector<std::wstring>();
	std::vector<std::wstring> re;
	boost::property_tree::wptree pt;
	std::wstringstream ss;
	std::wifstream file(filename);
	if (!file) throw std::runtime_error("cannot open file");
	static_assert(sizeof(wchar_t) == 2, "In function html_extract, wchar_t is not UTF16.");
	file.imbue(std::locale(std::locale(), new std::codecvt_utf8_utf16<wchar_t>()));//UTF-8 -> UTF16(wchar_t in Windows.)
	convert_html_to_xml(file, ss);
	read_xml(ss, pt, boost::property_tree::xml_parser::no_comments);
	const auto path = detail::parse_path(path_str);
	auto& body = pt.get_child(L"html.body");//HTML has body tag. if not exist, exception will be thrown.
	detail::html_extract_impl(re, body, path);//analyse
	return re;
}
