#include "html_parse.hpp"
#include <iostream>
int main()
{
	try {
		std::wcout.imbue(std::locale(""));
		const std::vector<std::wstring> result = html_extract(L"sample.html", L"div.inner");
		for (auto& r : result) {
			std::wcout << r << std::endl;//結果表示
		}
	}
	catch (const std::exception& er) {
		std::cerr << er.what() << std::endl;
	}
	return 0;
}
