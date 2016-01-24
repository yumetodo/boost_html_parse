#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <codecvt>
#include <stdexcept>
#include <type_traits>
#include <cstdint>
#include <algorithm>
#include <sstream>
namespace detail {
	using boost::property_tree::ptree;
	enum class specifier_type : uint8_t {
		none_,
		class_,
		id_
	};
	struct node_select_piece {
		node_select_piece() = default;
		node_select_piece(const node_select_piece&) = default;
		node_select_piece(node_select_piece&&) = default;
		node_select_piece& operator=(const node_select_piece&) = default;
		node_select_piece& operator=(node_select_piece&&) = default;
		explicit node_select_piece(const std::string& str)
			: tag(), type(specifier_type::none_), attr()
		{
			const auto split_pos = str.find_first_of("#.");
			if (std::string::npos == split_pos) {
				tag = str.substr(0, str.find_first_of("#."));
			}
			else {
				type = ('#' == str[split_pos]) ? specifier_type::id_ : specifier_type::class_;
				attr = str.substr(split_pos + 1);
			}
		}
		boost::optional<std::string> tag;
		specifier_type type;
		boost::optional<std::string> attr;
	};
	std::vector<node_select_piece> parse_path(const std::string& path) {
		std::vector<node_select_piece> re;
		std::size_t current = 0;
		for (std::size_t found; std::string::npos != (found = path.find_first_of(' ', current)); current = found + 1) {
			re.emplace_back(std::string(path, current, found - current));//call std::string->node_select_piece convert constructor
		}
		re.emplace_back(std::string(path, current, path.size() - current));
		return re;//expect NRVO
	}
	bool has_attribute(const ptree& p, const std::string& attr_name, const std::string& attr) {
		auto a = p.get_optional<std::string>(u8"<xmlattr>." + attr_name);
		return static_cast<bool>(a) && *a == attr;
	}
	bool has_attribute(const ptree& p, specifier_type attr_name, const std::string& attr) {//path[layer_level - 1].typeがnoneのときtrue
		return (specifier_type::none_ == attr_name) || has_attribute(p, (specifier_type::class_ == attr_name) ? "class" : "id", attr);
	}
	boost::optional<std::size_t> tag_search(const std::string& key, const std::vector<node_select_piece>& path, std::size_t layer_level) {
		for (auto i = layer_level; 0 <= i; --i) if (path[i].tag == key) return i;
		return{};//none
	}
	void html_extract_impl(std::vector<std::string>& re, const ptree& pt, const std::vector<node_select_piece>& path, std::size_t layer_level = 0U) {//1~
		if (pt.size()) {//childがあるか
			for (auto& p : pt) {
				const auto search_re = tag_search(p.first, path, layer_level);
				if (search_re) {
					if (!path[*search_re].attr || has_attribute(p.second, path[*search_re].type, path[*search_re].attr.get())) {
						if (path.size() == layer_level + 1) {
							re.push_back(p.first);
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
}
std::vector<std::string> html_extract(const std::string& filename, const std::string& path_str) {
	if (!path_str.length()) return std::vector<std::string>();
	std::vector<std::string> re;
	boost::property_tree::ptree pt;
	read_xml(filename, pt, boost::property_tree::xml_parser::no_comments);
	const auto path = detail::parse_path(path_str);
	auto& body = pt.get_child(u8"body");//HTML has body tag. if not exist, exception will be thrown.
	detail::html_extract_impl(re, body, path);//analyse
	return re;
}
