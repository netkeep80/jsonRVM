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
#include <fstream>
#include "string_utils.h"
#include "database_api.h"

namespace rm
{
	using namespace std;
	using namespace nlohmann;

	/**
	 * @class file_database_t
	 * @brief Реализация файловой базы данных для хранения сущностей в формате JSON
	 *
	 * Класс предоставляет методы для работы с сущностями:
	 * - Загрузка сущностей из файлов .json
	 * - Сохранение сущностей в файлы .json
	 * - Базовые операции поиска (заглушка)
	 *
	 * Особенности:
	 * - Автоматическое преобразование кодировок UTF-8 <-> CP-1251 для имен файлов
	 * - Генерация уникальных ID на основе адресов объектов в памяти
	 * - Хранение данных в директории, заданной при инициализации
	 * - Бросает исключения при ошибках ввода/вывода
	 *
	 * @warning Использует хэши адресов объектов для генерации ID, что может приводить
	 *          к коллизиям при сериализации разных объектов в одном процессе
	 *
	 * Пример использования:
	 * @code
	 * file_database_t db("data/");
	 * json entity;
	 * string id;
	 * db.add_entity(entity, id); // Сохранит в data/<id>.json
	 * db.get_entity(entity, id); // Загрузит из data/<id>.json
	 * @endcode
	 */
	class file_database_t : public database_api
	{
		string PathFolder;

	public:
		file_database_t(const string &path)
			: PathFolder(path)
		{
		}

		virtual void get_entity(json &ent, const string &ent_id) override
		{
			string PathName = PathFolder + utf8_to_cp1251(ent_id) + ".json";
			std::ifstream in(PathName.c_str());

			if (!in.good())
				throw runtime_error(__func__ + ": Can't load json from the "s + PathFolder + ent_id + " file!");

			in >> ent;
		}

		virtual void add_entity(const json &ent, string &ent_id) override
		{
			ent_id = to_string(reinterpret_cast<size_t>(&ent));

			string PathName = PathFolder + utf8_to_cp1251(ent_id) + ".json";
			std::ofstream out(PathName.c_str());

			if (!out.good())
				throw runtime_error(__func__ + ": Can't open "s + PathName + " file."s);

			out << ent;
		}

		virtual void query_entity(json &ent, const json &query) override
		{
			cout << __func__ << endl;
		}
	};
}
