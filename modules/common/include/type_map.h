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

namespace rm
{

	struct type_pair_null
	{
		using key = type_pair_null;
	};

	template <typename key_t, typename value_t>
	struct type_pair
	{
		using key = key_t;
		using type = value_t;
	};

	template<typename key_t, typename ...pairs_t>
	struct type_pair_list
	{
		constexpr static size_t size{ 0 };
		using pair = type_pair_null;
		using find = pair;
	};

	template <typename key_t, typename first, typename ...tail>
	struct type_pair_list<key_t, first, tail...>
	{
		constexpr static size_t size{ sizeof...(tail) + 1 };
		using pair = first;
		template<typename key_t>
		using next = type_pair_list<key_t, tail...>;
		using find = std::conditional_t<	is_same<key_t, typename pair::key>::value, pair, typename next<key_t>::find >;
	};

	template<typename ...pairs_t>
	struct type_map
	{
		constexpr static size_t size{ sizeof...(pairs_t) };
		template<typename key_t>
		using pair_list = type_pair_list<key_t, pairs_t...>;

		template<typename key_t>
		using find = typename pair_list<key_t>::find;
	};

}
/*

// using template arg as config name

template<typename config_key_t>
struct custom_struct
{
	using my_map32 = type_map<
		type_pair<as_type("плавающее"), float>,
		type_pair<as_type("положительное"), unsigned>,
		type_pair<as_type("целое"), int>
	>;

	using my_map64 = type_map<
		type_pair<as_type("плавающее"), double>,
		type_pair<as_type("положительное"), unsigned long long>,
		type_pair<as_type("целое"), long long>
	>;

	using my_config = type_map<
		type_pair<as_type("32 бита"), my_map32>,
		type_pair<as_type("64 бита"), my_map64>
	>;

	template<typename key_t>
	using select = typename my_config::find<config_key_t>::type::template find<key_t>;

	select<as_type("плавающее")>		a;
	select<as_type("положительное")>	b;
	select<as_type("целое")>			c;
};


int main()
{
	cout << to_sv("hello world") << endl;
	cout << typeid(as_type("hello world")).name() << endl;

	custom_struct<as_type("32 бита")> themap32;
	custom_struct<as_type("64 бита")> themap64;

	cout << typeid(themap32.a).name() << endl;
	cout << typeid(themap32.b).name() << endl;
	cout << typeid(themap32.c).name() << endl;

	cout << typeid(themap64.a).name() << endl;
	cout << typeid(themap64.b).name() << endl;
	cout << typeid(themap64.c).name() << endl;
}

*/