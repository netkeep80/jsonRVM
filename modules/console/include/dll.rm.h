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
#include "windows.h"
#include "string_utils.h"

namespace rm
{
	//	Поддержка загрузки DLL

	struct DLL
	{
		HMODULE		handle;
		InitDict	Init;
		DLL() : handle(nullptr), Init(nullptr) {}
	};


	class DLLs : public map<string, DLL>
	{
	public:
		DLLs() = default;
		~DLLs()
		{
			for each (auto dll in *this) if (dll.second.handle)
			{
				FreeLibrary(dll.second.handle);
				dll.second.handle = nullptr;
				dll.second.Init = nullptr;
			}
		}

		void	LoadDict(const string& LibName)
		{
			DLLs& it = *this;
			if (find(LibName) == end())	it[LibName] = DLL();

			if (!it[LibName].handle)
			{
				//	перезагружаем либу
				it[LibName].handle = LoadLibrary(utf8_to_wstring(LibName).c_str());

				if (it[LibName].handle)
				{
					(FARPROC&)it[LibName].Init = GetProcAddress(it[LibName].handle, IMPORT_RELATIONS_MODEL);
					if (!it[LibName].Init)
						throw json({ { __FUNCTION__, LibName + " does't has function "s + IMPORT_RELATIONS_MODEL} });
				}
				else
				{
					it[LibName].Init = nullptr;
					throw json({ { __FUNCTION__, "can't load '" + LibName + "' dictionary"s} });
				}
			}
		}
	} LoadedDLLs;


	const string&	LoadAndInitDict(const string& LibName, vm& Ent)
	{
		LoadedDLLs.LoadDict(LibName);
		return LoadedDLLs[LibName].Init(Ent);
	}

	//////////////////////////	base dictionary  ///////////////////////////////////

	void  jsonLoadDLL(vm& rmvm, vm_ctx& $)
	{
		try
		{
			if ($.obj.is_object())
			{
				if ($.obj.count("PathFolder") && $.obj.count("FileName"))
				{
					string	FullFileName = $.obj["PathFolder"].get<string>() + $.obj["FileName"].get<string>();
					$.sub = LoadAndInitDict(FullFileName, rmvm);
					return;
				}
			}
		}
		catch (json& j) { $.throw_json(__FUNCTION__, j); }
		catch (json::exception& e) { $.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { $.throw_json(__FUNCTION__, "std::exception: "s + e.what()); }
		catch (...) { $.throw_json(__FUNCTION__, "unknown exception"s); }

		$.rel = false;
		$.throw_json(__FUNCTION__, "$obj must be json object with PathFolder, FileName properties!"s);
	}

	////////////////////////////////////////////////////////////////////////////////

	const string&  ImportLoadDLLEntity(vm& rmvm)
	{
		json&	ent = rmvm.add_base_entity(rmvm["rmvm"]["load"], "dll"s, jsonLoadDLL, "Loads compiled entity vocabulary from dll library");
		ent["$obj"] = json::object();
		ent["$obj"]["PathFolder"] = "input: string with path to compiled to dll vocabulary";
		ent["$obj"]["FileName"] = "input: string with filename of compiled to dll vocabulary";
		ent["$sub"] = "output: string with version of compiled to dll vocabulary";
		return vm::version;
	}

}