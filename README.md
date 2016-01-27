parse HTML by using Boost.PropertyTree

for 
https://teratail.com/questions/25158

# Install

Require [boost](http://www.boost.org/).
When you use VS-Solution file, please set Environment variable　``$(BOOST_ROOT)`` and ``$(BOOST_LIB)``.

# Support Compiler

using C++11(constexpr, noexcept).

- Visual Studio 2015 Upadte 1
- Visual Studio 2015 Clang with Microsoft CodeGen clang3.7
- msys2 mingw clang++ 3.7.0
- msys2 mingw g++ 5.3.0

# Usage

Watch ``Source.cpp``.

```cpp
const std::vector<std::wstring> result = html_extract(L"sample.html", L"div.inner");
```

call ``html_extract`` like this.
First argument is filename to analyse. specify relative path from current directory.
Secound argument is search key. The format is same as css's selector(See below).

Don't forget to call ``std::wcout.imbue`` before call ``html_extract`` when you want to use ``std::wcout``.

## Support selector

| Selector                                                                   | Example    | Example description                     |
|----------------------------------------------------------------------------|------------|-----------------------------------------|
| [.class](http://www.w3schools.com/cssref/sel_class.asp)                    | .intro     | Selects all elements with class="intro" |
| [#id](http://www.w3schools.com/cssref/sel_id.asp)                          | #firstname | Selects the element with id="firstname" |
| [element](http://www.w3schools.com/cssref/sel_element.asp)                 | p          | Selects all,elements                    |
| [element element](http://www.w3schools.com/cssref/sel_element_element.asp) | div p      | Selects all,elements inside,elements    |

