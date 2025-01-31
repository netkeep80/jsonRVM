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
#pragma once

/**
 * @file string_meta.h
 * @brief Метапрограммные средства для работы со строками на этапе компиляции
 * 
 * Реализация типобезопасного представления строковых литералов в виде типов.
 * Позволяет использовать строки как параметры шаблонов и в compile-time контекстах.
 *
 * Основные компоненты:
 * - string_type: Хранит строку как пакет символов char
 * - string_type_maker: Рекурсивно собирает символы в тип string_type
 * - Макросы as_type/to_sv: Преобразуют строковые литералы в типы
 *
 * @struct string_type
 * @tparam chars... Пакет символов строки
 * @brief Представляет строку как тип с фиксированным размером
 * @var str  Статический массив символов
 * @var size Длина строки (без учета завершающего нуля)
 *
 * @struct string_type_maker
 * @brief Рекурсивно конструирует тип string_type из символов строки
 * @tparam N    Требуемый размер строки
 * @tparam T    Аккумулятор символов (string_type<...>)
 * @tparam tail Оставшиеся символы для обработки
 *
 * @note Особенности:
 * - Максимальная длина строки: 64 символа (ограничение макроса selch)
 * - Поддерживает только строковые литералы (const char[N])
 * - Все вычисления выполняются на этапе компиляции
 *
 * @warning Использование индексов за пределами длины строки приводит 
 *          к подстановке нуль-терминатора (selch)
 *
 * Пример использования:
 * @code
 * using MyString = as_type("Hello");
 * constexpr auto str = to_sv("Meta");
 * static_assert(MyString::size == 5, "");
 * @endcode
 */
namespace rm
{
	template <char... chars>
	struct string_type
	{
		constexpr static inline char str[]{ chars... };
		constexpr static inline size_t size{ sizeof...(chars) };
	};

	template <size_t N, typename T, char...>
	struct string_type_maker
	{
		using make = string_type<>;
	};

	template <size_t N, char... chars, char first, char... tail>
	struct string_type_maker<N, string_type<chars...>, first, tail...>
	{
		using make = std::conditional_t<(sizeof...(chars) == N), string_type<chars...>, typename string_type_maker<N, string_type<chars..., first>, tail...>::make>;
	};

	template <size_t size>
	constexpr auto str_size(const char(&str)[size]) { return size; }

	template <size_t size>
	constexpr auto selch(const char(&str)[size], const size_t index) { return str[index >= size ? size - 1 : index]; }

#define as_type(s) string_type_maker<str_size(s), string_type<>, selch(s,0), selch(s,1), selch(s,2), selch(s,3), selch(s,4), selch(s,5), selch(s,6), selch(s,7), selch(s,8), selch(s,9), selch(s,10), selch(s,11), selch(s,12), selch(s,13), selch(s,14), selch(s,15), selch(s,16), selch(s,17), selch(s,18), selch(s,19), selch(s,20), selch(s,21), selch(s,22), selch(s,23), selch(s,24), selch(s,25), selch(s,26), selch(s,27), selch(s,28), selch(s,29), selch(s,30), selch(s,31), selch(s,32), selch(s,33), selch(s,34), selch(s,35), selch(s,36), selch(s,37), selch(s,38), selch(s,39), selch(s,40), selch(s,41), selch(s,42), selch(s,43), selch(s,44), selch(s,45), selch(s,46), selch(s,47), selch(s,48), selch(s,49), selch(s,50), selch(s,51), selch(s,52), selch(s,53), selch(s,54), selch(s,55), selch(s,56), selch(s,57), selch(s,58), selch(s,59), selch(s,60), selch(s,61), selch(s,62), selch(s,63), selch(s,64)>::make
#define to_sv(s)   as_type(s)::str
}
