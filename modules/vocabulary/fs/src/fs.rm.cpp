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
#include <iostream>
#include <fstream>
#include "windows.h"
#include "fs.rm.h"
#include "string_utils.h"
#include "vm.rm.h"

#define MAX_NAME	1024
using namespace rm;

#define FROM_JSON_FIELD(jname,sname) if (jsonValue.count(#jname)) data.##sname = jsonValue[#jname];

namespace nlohmann
{
	namespace detail
	{
		void to_json(json& jsonValue, const SYSTEMTIME& data)
		{
			jsonValue["Year"] = data.wYear;
			jsonValue["Month"] = data.wMonth;
			jsonValue["DayOfWeek"] = data.wDayOfWeek;
			jsonValue["Day"] = data.wDay;
			jsonValue["Hour"] = data.wHour;
			jsonValue["Minute"] = data.wMinute;
			jsonValue["Second"] = data.wSecond;
			jsonValue["Milliseconds"] = data.wMilliseconds;
		}

		void from_json(const json& jsonValue, SYSTEMTIME& data)
		{
			memset(&data, 0, sizeof(SYSTEMTIME));
			if (jsonValue.is_object())
			{
				FROM_JSON_FIELD(Year, wYear);
				FROM_JSON_FIELD(Month, wMonth);
				FROM_JSON_FIELD(DayOfWeek, wDayOfWeek);
				FROM_JSON_FIELD(Day, wDay);
				FROM_JSON_FIELD(Hour, wHour);
				FROM_JSON_FIELD(Minute, wMinute);
				FROM_JSON_FIELD(Second, wSecond);
				FROM_JSON_FIELD(Milliseconds, wMilliseconds);
			}
		}

		void to_json(json& jsonValue, const WIN32_FIND_DATA& data)
		{
			SYSTEMTIME SystemTime;
			jsonValue["FileAttributes"] = data.dwFileAttributes;
			if (FileTimeToSystemTime(&data.ftCreationTime, &SystemTime)) jsonValue["CreationTime"] = SystemTime;
			if (FileTimeToSystemTime(&data.ftLastAccessTime, &SystemTime)) jsonValue["LastAccessTime"] = SystemTime;
			if (FileTimeToSystemTime(&data.ftLastWriteTime, &SystemTime)) jsonValue["LastWriteTime"] = SystemTime;
			jsonValue["FileSizeHigh"] = data.nFileSizeHigh;
			jsonValue["FileSizeLow"] = data.nFileSizeLow;
			jsonValue["FileName"] = wstring_to_utf8(data.cFileName);
		}

		/*void from_json(const json& jsonValue, WIN32_FIND_DATA& data)
		{
			memset(&data, 0, sizeof(WIN32_FIND_DATA));
			if (jsonValue.is_object())
			{
				FROM_JSON_FIELD(ArterialStiffness);
			}
		}*/
	}
}

////////////////////////////////////////////////////////////////////////////////

void  fs_dir_scan(vm& rmvm, vm_ctx& $)
{
	if (!$.obj.is_object())
		$.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileNameFormat properties!" );

	if (!$.obj.count("PathFolder") || !$.obj.count("FileNameFormat"))
		$.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileNameFormat properties!" );

	string	FileNameFormat = $.obj["FileNameFormat"];
	string	PathFolder = $.obj["PathFolder"];
	json	FileInfo;
	HANDLE	handle;
	WIN32_FIND_DATA search_data;
	unsigned int i = 0;

	//	�������������� �������� ��������
	$.sub = json::array();	//	���� ����� �������� ��������� � ��������� ��������, �� �������� ������ � ���������
	memset(&search_data, 0, sizeof(WIN32_FIND_DATA));
	handle = FindFirstFile((utf8_to_wstring(PathFolder) + utf8_to_wstring(FileNameFormat)).c_str(), &search_data);

	while (handle != INVALID_HANDLE_VALUE)
	{
		FileInfo = search_data;
		FileInfo["PathFolder"] = PathFolder;
		$.sub[i++] = FileInfo;

		if (FindNextFile(handle, &search_data) == FALSE)
		{
			FindClose(handle);
			break;
		}
	}

}

void  fs_dir_create(vm& rmvm, vm_ctx& $)
{
	if (!$.obj.is_string())
		$.throw_json(__FUNCTION__, "$obj must be json string with PathName!" );

	string	PathName = utf8_to_cp1251($.obj);
	$.sub = bool(CreateDirectoryA(PathName.c_str(), nullptr));
}

void  fs_dir_delete(vm& rmvm, vm_ctx& $)
{
	if (!$.obj.is_string())
		$.throw_json(__FUNCTION__, "$obj must be json string with PathName!" );

	string	PathName = utf8_to_cp1251($.obj);
	$.sub = bool(RemoveDirectoryA(PathName.c_str()));
}

void  fs_file_load_rm(vm& rmvm, vm_ctx& $)
{
	if (!$.obj.is_object())
		$.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileName properties!" );

	if (!$.obj.count("PathFolder") || !$.obj.count("FileName"))
		$.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileName properties!" );

	string	PathName = utf8_to_cp1251($.obj["PathFolder"].get<string>() + $.obj["FileName"].get<string>());
	std::ifstream in(PathName.c_str());

	if (!in.good())
		$.throw_json(__FUNCTION__, "Can't load json from the "s + $.obj["PathFolder"].get<string>() + $.obj["FileName"].get<string>() + " file!" );

	json	rm;
	in >> rm;

	if (rm.is_object())
	{
		$.rel = true;
		for (auto& p : rm.items())
			$.sub[p.key()] = p.value();
	}
	else
		$.rel = false;
}

void  fs_file_read_json(vm& rmvm, vm_ctx& $)
{
	if (!$.obj.is_object())
		$.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileName properties!" );

	if (!$.obj.count("PathFolder") || !$.obj.count("FileName"))
		$.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileName properties!" );

	string	PathName = utf8_to_cp1251($.obj["PathFolder"].get<string>() + $.obj["FileName"].get<string>());
	std::ifstream in(PathName.c_str());

	if (!in.good())
		$.throw_json(__FUNCTION__, "Can't load json from the "s + $.obj["PathFolder"].get<string>() + $.obj["FileName"].get<string>() + " file!" );

	$.rel = true;
	in >> $.sub;
	if ($.sub.is_object())
		$.sub["FileInfo"] = $.obj;

}

void  jsonFileToString(vm& rmvm, vm_ctx& $)
{
	$.sub = json();

	if ($.obj.is_object())
	{
		if ($.obj.count("PathFolder") && $.obj.count("FileName"))
		{
			string	PathName = utf8_to_cp1251($.obj["PathFolder"].get<string>() + $.obj["FileName"].get<string>());
			std::ifstream in(PathName.c_str(), ios_base::binary);

			if (in.good())
			{
				size_t size = size_t(in.seekg(0, ios_base::end).tellg().seekpos());
				in.seekg(0, ios_base::beg);
				char*	buf = new char[size + 1];
				in.read(buf, size);
				buf[size] = 0;
				$.sub = json(utf8_to_wstring(string(buf)));
				delete[] buf;
				$.rel = true;
				return;
			}
		}
	}

	$.rel = false;
}

void  jsonFileToStringArray(vm& rmvm, vm_ctx& $)
{
	if ($.obj.is_object())
	{
		if ($.obj.count("PathFolder") && $.obj.count("FileName"))
		{
			string	PathName = utf8_to_cp1251($.obj["PathFolder"].get<string>() + $.obj["FileName"].get<string>());
			std::ifstream in(PathName.c_str(), ios::binary | ios::ate);
			
			if (in.good())
			{
				$.rel = true;
				size_t size = in.tellg();
				in.seekg(0, ios::beg);
				char*	buf = new char[size + 1];
				in.read(buf, size);
				buf[size] = 0;
				string str(buf);
				string delim("\r\n");
				size_t prev = 0, pos = 0, i = 0;
				$.sub = json::array();

				do
				{
					pos = str.find(delim, prev);
					if (pos == string::npos) pos = str.length();
					string token = str.substr(prev, pos - prev);
					if (!token.empty())	$.sub.push_back(token);
					prev = pos + delim.length();
				} while (pos < str.length() && prev < str.length());

				delete[] buf;
				return;
			}

			$.throw_json(__FUNCTION__, "Can't load string array from the "s + $.obj["PathFolder"].get<string>() + $.obj["FileName"].get<string>() + " file!" );
		}
	}

	$.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileName property!" );
}

void  jsonStringArrayToFile(vm& rmvm, vm_ctx& $)
{
	if ($.obj.is_object() && $.sub.is_array())
	{
		if ($.obj.count("PathFolder") && $.obj.count("FileName"))
		{
			string	PathName = utf8_to_cp1251($.obj["PathFolder"].get<string>() + $.obj["FileName"].get<string>());
			std::ofstream out(PathName.c_str());
			if (out.good())
			{
				for (auto& it : $.sub)
				{
					if (it.is_string())
						out << it.get_ref<string&>() << endl;
					else
						out << it.dump() << endl;
				}
				return;
			}
			else
				$.throw_json(__FUNCTION__, "Can't create the "s + $.obj["PathFolder"].get<string>() + $.obj["FileName"].get<string>() + " file!" );
		}
		else
			$.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileName property!" );
	}
	else
		$.throw_json(__FUNCTION__, "$obj must be object and $sub must be array!" );
}

void  fs_file_write_json(vm& rmvm, vm_ctx& $)
{
	if ($.obj.is_object())
	{
		if ($.obj.count("PathFolder") && $.obj.count("FileName"))
		{
			string	PathName = utf8_to_cp1251($.obj["PathFolder"].get<string>() + $.obj["FileName"].get<string>());
			std::ofstream out(PathName.c_str());

			if (out.good())
			{
				out << $.sub;
				$.rel = true;
				return;
			}

			$.rel = false;
			$.throw_json(__FUNCTION__, "Can't open "s + PathName + " file."s );
		}
	}

	$.rel = false;
	$.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileName property!" );
}


void  jsonToFiles(vm& rmvm, vm_ctx& $)
{
	$.rel = json::array();

	/*if ($.sub.is_array())	//	� $.sub ������ �������� FileInfo, ������ �� ������� ��������� ���� � ������� ���� ���������
	{
	size_t i = 0;
	for (auto& it : $.sub.as_array())
	jsonToFiles(json(jref(EV["ctx"]), jref(EV["ent"]), it, json(), $.obj), Model, (View)[i++]);
	}
	else*/
	if ($.sub.is_object())
	{
		if ($.obj.is_array())
		{
			size_t i = 0;
			for (auto& it : $.obj)
			{
				auto&	FileNameFormat = $.sub["FileNameFormat"];
				auto&	PathFolder = $.sub["PathFolder"];
				string	FileInfoPathFolder = ".\\";
				string	FileInfoFileName = to_string(i) + ".json";

				if (FileNameFormat.is_null()) FileNameFormat = json("%s"s);
				if (PathFolder.is_null()) PathFolder = json("%s"s);

				if (it.is_object())
				{
					auto &FileInfoVal = it["FileInfo"];

					if (FileInfoVal.is_object())
					{
						FileInfoPathFolder = utf8_to_cp1251(FileInfoVal["PathFolder"]);
						FileInfoFileName = utf8_to_cp1251(FileInfoVal["FileName"]);
					}
				}

				char	PathName[MAX_NAME];
				char	FileName[MAX_NAME];
				sprintf_s(PathName, utf8_to_cp1251(PathFolder).c_str(), FileInfoPathFolder.c_str());
				sprintf_s(FileName, utf8_to_cp1251(FileNameFormat).c_str(), FileInfoFileName.c_str());

				json	FileInfoVal, FileInfo;
				FileInfoVal["PathFolder"] = cp1251_to_wstring(PathName);
				FileInfoVal["FileName"] = cp1251_to_wstring(FileName);
				FileInfo["FileInfo"] = FileInfoVal;
				//jsonToFile(json(jref(EV["ctx"]), jref(EV["ent"]), FileInfo, json(), it), Model, View[i++]);
			}
		}
		else
		{
			json&	FileNameFormat = $.sub["FileNameFormat"];
			json&	PathFolder = $.sub["PathFolder"];
			string	FileInfoPathFolder = ".\\";
			string	FileInfoFileName = ".json";

			if (FileNameFormat.is_null()) FileNameFormat = "%s"s;
			if (PathFolder.is_null()) PathFolder = "%s"s;

			if ($.obj.is_object())
			{
				json const&FileInfoVal = $.obj["FileInfo"];

				if (FileInfoVal.is_object())
				{
					FileInfoPathFolder = utf8_to_cp1251(FileInfoVal["PathFolder"]);
					FileInfoFileName = utf8_to_cp1251(FileInfoVal["FileName"]);
				}
			}

			char	PathName[MAX_NAME];
			char	FileName[MAX_NAME];
			sprintf_s(PathName, utf8_to_cp1251(PathFolder).c_str(), FileInfoPathFolder.c_str());
			sprintf_s(FileName, utf8_to_cp1251(FileNameFormat).c_str(), FileInfoFileName.c_str());

			json	FileInfoVal, FileInfo;
			FileInfoVal["PathFolder"] = cp1251_to_utf8(PathName);
			FileInfoVal["FileName"] = cp1251_to_utf8(FileName);
			FileInfo["FileInfo"] = FileInfoVal;
			//jsonToFile(json(jref(EV["ctx"]), jref(EV["ent"]), FileInfo, json(), $.obj), Model, View);
		}
	}
}


void	rmvm_dump(vm& rmvm, vm_ctx& $)
{
	string filename = "rmvm.dump.json";
	std::ofstream out(filename);
	if (out.good())
		out << rmvm.dump();
	else
		cerr << "Can't store rmvm dump in the " << filename << " file.\n";
}


namespace rm
{
	
	FSRM_API const string&	import_relations_model_to(vm& rmvm)
	{
		rmvm.add_binary_view(rmvm["dir"], "scan"s, fs_dir_scan, "Scanning filesystem directory, $obj must be json object with PathFolder and FileNameFormat properties"s);
		rmvm.add_binary_view(rmvm["dir"], "create"s, fs_dir_create, "Create new directory, $obj must be string"s);
		rmvm.add_binary_view(rmvm["dir"], "delete"s, fs_dir_delete, "Delete directory, $obj must be string"s);

		rmvm.add_binary_view(rmvm["file"]["load"], "rm"s, fs_file_load_rm, "Loads rm from json"s);

		rmvm.add_binary_view(rmvm["file"]["read"], "json"s, fs_file_read_json, "Reads json file and parse it"s);
		rmvm.add_binary_view(rmvm["file"]["read"], "string"s, jsonFileToString, ""s);
		rmvm.add_binary_view(rmvm["file"]["read"]["array"], "string"s, jsonFileToStringArray, ""s);

		rmvm.add_binary_view(rmvm["file"]["write"]["array"], "string"s, jsonStringArrayToFile, ""s);
		rmvm.add_binary_view(rmvm["file"]["write"], "json"s, fs_file_write_json, "Write json to file"s);
		rmvm.add_binary_view(rmvm["files"]["write"], "json"s, jsonToFiles, ""s);

		rmvm.add_binary_view(rmvm["rmvm"], "dump"s, rmvm_dump, "Dump rmvm current state"s);

		return rmvm_version;
	}
	
}
