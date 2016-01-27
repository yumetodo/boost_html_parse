#pragma once
#include <vector>
#include <string>
std::vector<std::string> html_extract(const std::string& filename, const std::string& path_str);
#ifndef __MINGW32__
std::vector<std::wstring> html_extract(const std::wstring& filename, const std::wstring& path_str);
#endif
