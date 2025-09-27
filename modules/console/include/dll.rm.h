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
#include "vm.rm.h"
#include "string_utils.h"

#ifdef _WIN32
#include "windows.h"
#endif

namespace rm
{
	//	Поддержка загрузки DLL

	struct DLL
	{
#ifdef _WIN32
		HMODULE handle;
#else
		void* handle;
#endif
		InitDict Init;
		DLL() : handle(nullptr), Init(nullptr) {}
	};

	/**
	 * @class DLLs
	 * @brief Менеджер загрузки и управления DLL-библиотеками для расширения функционала RVM
	 *
	 * Класс обеспечивает:
	 * - Динамическую загрузку/выгрузку DLL
	 * - Кэширование загруженных модулей
	 * - Автоматическую очистку ресурсов при разрушении объекта
	 * - Поиск точек входа в словари отношений
	 *
	 * Особенности:
	 * - Использует Windows API для работы с DLL (LoadLibrary/FreeLibrary)
	 * - Хранит хэндлы и функции инициализации в map-структуре
	 * - Генерирует JSON-исключения при ошибках загрузки
	 * - Поддерживает кодировку UTF-8 для путей к библиотекам
	 *
	 * @warning Все операции потокобезопасны только если внешний код синхронизирует доступ
	 *
	 * Пример использования:
	 * @code
	 * DLLs loader;
	 * loader.LoadDict("my_library.dll");
	 * auto& dict = loader["my_library.dll"].Init(my_vm);
	 * @endcode
	 */
	class DLLs : public map<string, DLL>
	{
	public:
		DLLs() = default;
		~DLLs()
		{
#ifdef _WIN32
			for each (auto dll in *this)
				if (dll.second.handle)
				{
					FreeLibrary(dll.second.handle);
					dll.second.handle = nullptr;
					dll.second.Init = nullptr;
				}
#else
			for (auto& dll : *this)
				if (dll.second.handle)
				{
					dll.second.handle = nullptr;
					dll.second.Init = nullptr;
				}
#endif
		}

		void LoadDict(const string &LibName)
		{
			DLLs &it = *this;
			if (find(LibName) == end())
				it[LibName] = DLL();

			if (!it[LibName].handle)
			{
#ifdef _WIN32
				//	перезагружаем либу
				it[LibName].handle = LoadLibrary(utf8_to_wstring(LibName).c_str());

				if (it[LibName].handle)
				{
					(FARPROC &)it[LibName].Init = GetProcAddress(it[LibName].handle, IMPORT_RELATIONS_MODEL);
					if (!it[LibName].Init)
						throw json({{__func__, LibName + " does't has function "s + IMPORT_RELATIONS_MODEL}});
				}
				else
				{
					it[LibName].Init = nullptr;
					throw json({{__func__, "can't load '" + LibName + "' dictionary"s}});
				}
#else
				throw json({{__func__, "DLL loading not supported on this platform"}});
#endif
			}
		}
	} LoadedDLLs;

	const string &LoadAndInitDict(const string &LibName, vm &Ent)
	{
		LoadedDLLs.LoadDict(LibName);
		return LoadedDLLs[LibName].Init(Ent);
	}

	/**
	 * @brief Контроллер загрузки DLL-словаря в виртуальную машину
	 *
	 * Функциональность:
	 * 1. Принимает JSON-объект с параметрами загрузки
	 * 2. Валидирует обязательные поля "PathFolder" и "FileName"
	 * 3. Формирует полный путь к библиотеке
	 * 4. Инициирует загрузку через DLLs-менеджер
	 * 5. Возвращает версию загруженного словаря через $.sub
	 *
	 * @param rmvm Ссылка на виртуальную машину отношений
	 * @param $ Контекст выполнения с параметрами:
	 *   - $obj: входные параметры {PathFolder:string, FileName:string}
	 *   - $sub: выходное значение (версия словаря)
	 *
	 * @throws json::exception При:
	 * - Отсутствии обязательных полей во входных данных
	 * - Ошибках загрузки DLL
	 * - Отсутствии точки входа в словаре
	 * - Системных ошибках (например, проблемы с доступом к файлу)
	 *
	 * @note Формирует трассируемые исключения с детальной диагностикой через vm_ctx::throw_json
	 */
	void jsonLoadDLL(vm &rmvm, vm_ctx &$)
	{
		try
		{
			if ($.obj.is_object())
			{
				if ($.obj.count("PathFolder") && $.obj.count("FileName"))
				{
					string FullFileName = $.obj["PathFolder"].get<string>() + $.obj["FileName"].get<string>();
					$.sub = LoadAndInitDict(FullFileName, rmvm);
					return;
				}
			}
		}
		catch (json &j)
		{
			$.throw_json(__func__, j);
		}
		catch (json::exception &e)
		{
			$.throw_json(__func__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id));
		}
		catch (std::exception &e)
		{
			$.throw_json(__func__, "std::exception: "s + e.what());
		}
		catch (...)
		{
			$.throw_json(__func__, "unknown exception"s);
		}

		$.rel = false;
		$.throw_json(__func__, "$obj must be json object with PathFolder, FileName properties!"s);
	}

	////////////////////////////////////////////////////////////////////////////////

	const string &ImportLoadDLLEntity(vm &rmvm)
	{
		json &ent = rmvm.add_base_entity(rmvm["rmvm"]["load"], "dll"s, jsonLoadDLL, "Loads compiled entity vocabulary from dll library");
		ent["$obj"] = json::object();
		ent["$obj"]["PathFolder"] = "input: string with path to compiled to dll vocabulary";
		ent["$obj"]["FileName"] = "input: string with filename of compiled to dll vocabulary";
		ent["$sub"] = "output: string with version of compiled to dll vocabulary";
		return vm::version;
	}

}