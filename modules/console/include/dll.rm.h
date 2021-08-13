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
Copyright � 2016 Vertushkin Roman Pavlovich <https://vk.com/earthbirthbook>.

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
#include "JsonRVM.h"
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
		DLLs() {}
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


	const string&	LoadAndInitDict(const string& LibName, jsonRVM& Ent)
	{
		LoadedDLLs.LoadDict(LibName);
		return LoadedDLLs[LibName].Init(Ent);
	}

	//////////////////////////	base dictionary  ///////////////////////////////////

	void  jsonLoadDLL(jsonRVM& rmvm, EntContext& ec)
	{
		try
		{
			if (ec.obj.is_object())
			{
				if (ec.obj.count("PathFolder") && ec.obj.count("FileName"))
				{
					string	FullFileName = ec.obj["PathFolder"].get<string>() + ec.obj["FileName"].get<string>();
					ec.sub = LoadAndInitDict(FullFileName, rmvm);
					return;
				}
			}
		}
		catch (json& j) { ec.throw_json(__FUNCTION__, j); }
		catch (json::exception& e) { ec.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { ec.throw_json(__FUNCTION__, "std::exception: "s + e.what()); }
		catch (...) { ec.throw_json(__FUNCTION__, "unknown exception"s); }

		ec.its = false;
		ec.throw_json(__FUNCTION__, "$obj must be json object with PathFolder, FileName properties!"s);
	}

	////////////////////////////////////////////////////////////////////////////////

	const string&  ImportLoadDLLEntity(jsonRVM& rmvm)
	{
		json&	ent = rmvm.AddBaseEntity(rmvm["rmvm"]["load"], "dll"s, jsonLoadDLL, "Loads compiled entity vocabulary from dll library");
		ent["$obj"] = json::object();
		ent["$obj"]["PathFolder"] = "input: string with path to compiled to dll vocabulary";
		ent["$obj"]["FileName"] = "input: string with filename of compiled to dll vocabulary";
		ent["$sub"] = "output: string with version of compiled to dll vocabulary";
		return rmvm_version;
	}

}