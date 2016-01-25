#pragma once
#include <iostream>
#include <type_traits>
void convert_html_to_xml(std::istream& is, std::ostream& os);
void convert_html_to_xml(std::wistream& is, std::wostream& os);
template<typename T> struct is_char_type : std::false_type {};
template<> struct is_char_type<char> : std::true_type {};
template<> struct is_char_type<wchar_t> : std::true_type {};
