#include "html_parse.hpp"
#include <iostream>
int main()
{
	try {
		const std::vector<std::string> result = html_extract(u8"sample.html", u8"div.inner");
		for (auto& r : result) {
			std::cout << r << std::endl;//結果表示
		}
	}
	catch (const std::exception& er) {
		std::cerr << er.what() << std::endl;
	}
	return 0;
}
