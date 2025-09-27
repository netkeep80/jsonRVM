/*        R
	   S__|__O
	 O   _|_   S
  R__|__/_|_\__|__R  jsonRVM
	 |  \_|_/  |     json Relations (Model) Virtual Machine
	 S    |    O     https://github.com/netkeep80/jsonRVM
		__|__
	   /  |  \
	  /___|___\
Fractal Triune Entity

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
Copyright (c) 2016-2021 Vertushkin Roman Pavlovich <https://vk.com/earthbirthbook>.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once
#include <string>
#ifdef _WIN32
#include "windows.h"
#endif

using namespace std;

// ToDo: refactor to std::locale

#ifdef _WIN32
template<UINT CodePage>
wstring	_to_wstring(const string& data)
{
	size_t			nLengthW = data.length() + 1;
	wchar_t* str = new wchar_t[nLengthW];
	memset(str, 0, nLengthW * sizeof(wchar_t));
	MultiByteToWideChar(CodePage, 0, data.c_str(), -1, str, (int)nLengthW);
	wstring	res(str);
	delete[] str;
	return res;
}

template<UINT CodePage>
string	wstring_to_(const wstring& data)
{
	size_t	size = data.length();
	if (size)
	{
		char* str = new char[2 * size + 1];	//	for russian utf8 string case
		memset(str, 0, 2 * size + 1);
		WideCharToMultiByte(CodePage, 0, data.c_str(), -1, str, 2 * (int)size + 1, NULL, NULL);
		string		res(str);
		delete[] str;
		return res;
	}
	else
		return string();
}

inline wstring cp1251_to_wstring(const string& data) { return _to_wstring<CP_ACP>(data); }
inline wstring utf8_to_wstring(const string& data) { return _to_wstring<CP_UTF8>(data); }
inline string  wstring_to_cp1251(const wstring& data) { return wstring_to_<CP_ACP>(data); }
inline string  wstring_to_utf8(const wstring& data) { return wstring_to_<CP_UTF8>(data); }

inline string cp1251_to_utf8(const string& data) { return wstring_to_utf8(_to_wstring<CP_ACP>(data)); }
inline string utf8_to_cp1251(const string& data) { return wstring_to_cp1251(_to_wstring<CP_UTF8>(data)); }
#else
inline wstring cp1251_to_wstring(const string& data) { return wstring(data.begin(), data.end()); }
inline wstring utf8_to_wstring(const string& data) { return wstring(data.begin(), data.end()); }
inline string  wstring_to_cp1251(const wstring& data) { return string(data.begin(), data.end()); }
inline string  wstring_to_utf8(const wstring& data) { return string(data.begin(), data.end()); }

inline string cp1251_to_utf8(const string& data) { return data; }
inline string utf8_to_cp1251(const string& data) { return data; }
#endif
