#include "html_parse.hpp"
#include <iostream>
int main()
{
	try {
#if !defined __MINGW32__
		std::wcout.imbue(std::locale(""));
		const std::vector<std::wstring> result = html_extract(L"sample.html", L"div.inner");
		for (auto& r : result) {
			std::wcout << r << std::endl;//結果表示
		}
#else
		const std::vector<std::string> result = html_extract(u8"sample.html", u8"div.inner");
		for (auto& r : result) {
			std::cout << r << std::endl;
		}
#endif
	}
	catch (const std::exception& er) {
		std::cerr << er.what() << std::endl;
	}
	return 0;
}
