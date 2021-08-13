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
#include <iostream>
#include <fstream>
#include "windows.h"
#include "fs.rm.h"
#include "string_utils.h"
#include "jsonRVM.h"

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

void  fs_dir_scan(jsonRVM& rmvm, EntContext& ec)
{
	if (!ec.obj.is_object())
		ec.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileNameFormat properties!" );

	if (!ec.obj.count("PathFolder") || !ec.obj.count("FileNameFormat"))
		ec.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileNameFormat properties!" );

	string	FileNameFormat = ec.obj["FileNameFormat"];
	string	PathFolder = ec.obj["PathFolder"];
	json	FileInfo;
	HANDLE	handle;
	WIN32_FIND_DATA search_data;
	unsigned int i = 0;

	//	�������������� �������� ��������
	ec.sub = json::array();	//	���� ����� �������� ��������� � ��������� ��������, �� �������� ������ � ���������
	memset(&search_data, 0, sizeof(WIN32_FIND_DATA));
	handle = FindFirstFile((utf8_to_wstring(PathFolder) + utf8_to_wstring(FileNameFormat)).c_str(), &search_data);

	while (handle != INVALID_HANDLE_VALUE)
	{
		FileInfo = search_data;
		FileInfo["PathFolder"] = PathFolder;
		ec.sub[i++] = FileInfo;

		if (FindNextFile(handle, &search_data) == FALSE)
		{
			FindClose(handle);
			break;
		}
	}

}

void  fs_dir_create(jsonRVM& rmvm, EntContext& ec)
{
	if (!ec.obj.is_string())
		ec.throw_json(__FUNCTION__, "$obj must be json string with PathName!" );

	string	PathName = utf8_to_cp1251(ec.obj);
	ec.sub = bool(CreateDirectoryA(PathName.c_str(), nullptr));
}

void  fs_dir_delete(jsonRVM& rmvm, EntContext& ec)
{
	if (!ec.obj.is_string())
		ec.throw_json(__FUNCTION__, "$obj must be json string with PathName!" );

	string	PathName = utf8_to_cp1251(ec.obj);
	ec.sub = bool(RemoveDirectoryA(PathName.c_str()));
}

void  fs_file_load_rm(jsonRVM& rmvm, EntContext& ec)
{
	if (!ec.obj.is_object())
		ec.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileName properties!" );

	if (!ec.obj.count("PathFolder") || !ec.obj.count("FileName"))
		ec.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileName properties!" );

	string	PathName = utf8_to_cp1251(ec.obj["PathFolder"].get<string>() + ec.obj["FileName"].get<string>());
	std::ifstream in(PathName.c_str());

	if (!in.good())
		ec.throw_json(__FUNCTION__, "Can't load json from the "s + ec.obj["PathFolder"].get<string>() + ec.obj["FileName"].get<string>() + " file!" );

	json	rm;
	in >> rm;

	if (rm.is_object())
	{
		ec.its = true;
		for (auto& p : rm.items())
			ec.sub[p.key()] = p.value();
	}
	else
		ec.its = false;
}

void  fs_file_read_json(jsonRVM& rmvm, EntContext& ec)
{
	if (!ec.obj.is_object())
		ec.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileName properties!" );

	if (!ec.obj.count("PathFolder") || !ec.obj.count("FileName"))
		ec.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileName properties!" );

	string	PathName = utf8_to_cp1251(ec.obj["PathFolder"].get<string>() + ec.obj["FileName"].get<string>());
	std::ifstream in(PathName.c_str());

	if (!in.good())
		ec.throw_json(__FUNCTION__, "Can't load json from the "s + ec.obj["PathFolder"].get<string>() + ec.obj["FileName"].get<string>() + " file!" );

	ec.its = true;
	in >> ec.sub;
	if (ec.sub.is_object())
		ec.sub["FileInfo"] = ec.obj;

}

void  jsonFileToString(jsonRVM& rmvm, EntContext& ec)
{
	ec.sub = json();

	if (ec.obj.is_object())
	{
		if (ec.obj.count("PathFolder") && ec.obj.count("FileName"))
		{
			string	PathName = utf8_to_cp1251(ec.obj["PathFolder"].get<string>() + ec.obj["FileName"].get<string>());
			std::ifstream in(PathName.c_str(), ios_base::binary);

			if (in.good())
			{
				size_t size = size_t(in.seekg(0, ios_base::end).tellg().seekpos());
				in.seekg(0, ios_base::beg);
				char*	buf = new char[size + 1];
				in.read(buf, size);
				buf[size] = 0;
				ec.sub = json(utf8_to_wstring(string(buf)));
				delete[] buf;
				ec.its = true;
				return;
			}
		}
	}

	ec.its = false;
}

void  jsonFileToStringArray(jsonRVM& rmvm, EntContext& ec)
{
	if (ec.obj.is_object())
	{
		if (ec.obj.count("PathFolder") && ec.obj.count("FileName"))
		{
			string	PathName = utf8_to_cp1251(ec.obj["PathFolder"].get<string>() + ec.obj["FileName"].get<string>());
			std::ifstream in(PathName.c_str(), ios::binary | ios::ate);
			
			if (in.good())
			{
				ec.its = true;
				size_t size = in.tellg();
				in.seekg(0, ios::beg);
				char*	buf = new char[size + 1];
				in.read(buf, size);
				buf[size] = 0;
				string str(buf);
				string delim("\r\n");
				size_t prev = 0, pos = 0, i = 0;
				ec.sub = json::array();

				do
				{
					pos = str.find(delim, prev);
					if (pos == string::npos) pos = str.length();
					string token = str.substr(prev, pos - prev);
					if (!token.empty())	ec.sub.push_back(token);
					prev = pos + delim.length();
				} while (pos < str.length() && prev < str.length());

				delete[] buf;
				return;
			}

			ec.throw_json(__FUNCTION__, "Can't load string array from the "s + ec.obj["PathFolder"].get<string>() + ec.obj["FileName"].get<string>() + " file!" );
		}
	}

	ec.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileName property!" );
}

void  jsonStringArrayToFile(jsonRVM& rmvm, EntContext& ec)
{
	if (ec.obj.is_object() && ec.sub.is_array())
	{
		if (ec.obj.count("PathFolder") && ec.obj.count("FileName"))
		{
			string	PathName = utf8_to_cp1251(ec.obj["PathFolder"].get<string>() + ec.obj["FileName"].get<string>());
			std::ofstream out(PathName.c_str());
			if (out.good())
			{
				for (auto& it : ec.sub)
				{
					if (it.is_string())
						out << it.get_ref<string&>() << endl;
					else
						out << it.dump() << endl;
				}
				return;
			}
			else
				ec.throw_json(__FUNCTION__, "Can't create the "s + ec.obj["PathFolder"].get<string>() + ec.obj["FileName"].get<string>() + " file!" );
		}
		else
			ec.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileName property!" );
	}
	else
		ec.throw_json(__FUNCTION__, "$obj must be object and $sub must be array!" );
}

void  fs_file_write_json(jsonRVM& rmvm, EntContext& ec)
{
	if (ec.obj.is_object())
	{
		if (ec.obj.count("PathFolder") && ec.obj.count("FileName"))
		{
			string	PathName = utf8_to_cp1251(ec.obj["PathFolder"].get<string>() + ec.obj["FileName"].get<string>());
			std::ofstream out(PathName.c_str());

			if (out.good())
			{
				out << ec.sub;
				ec.its = true;
				return;
			}

			ec.its = false;
			ec.throw_json(__FUNCTION__, "Can't open "s + PathName + " file."s );
		}
	}

	ec.its = false;
	ec.throw_json(__FUNCTION__, "$obj must be json object with PathFolder and FileName property!" );
}


void  jsonToFiles(jsonRVM& rmvm, EntContext& ec)
{
	ec.its = json::array();

	/*if (ec.sub.is_array())	//	� ec.sub ������ �������� FileInfo, ������ �� ������� ��������� ���� � ������� ���� ���������
	{
	size_t i = 0;
	for (auto& it : ec.sub.as_array())
	jsonToFiles(json(jref(EV["ctx"]), jref(EV["ent"]), it, json(), ec.obj), Model, (View)[i++]);
	}
	else*/
	if (ec.sub.is_object())
	{
		if (ec.obj.is_array())
		{
			size_t i = 0;
			for (auto& it : ec.obj)
			{
				json&	FileNameFormat = ec.sub["FileNameFormat"];
				json&	PathFolder = ec.sub["PathFolder"];
				string	FileInfoPathFolder = ".\\";
				string	FileInfoFileName = to_string(i) + ".json";

				if (FileNameFormat.is_null()) FileNameFormat = json("%s"s);
				if (PathFolder.is_null()) PathFolder = json("%s"s);

				if (it.is_object())
				{
					json &FileInfoVal = it["FileInfo"];

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
			json&	FileNameFormat = ec.sub["FileNameFormat"];
			json&	PathFolder = ec.sub["PathFolder"];
			string	FileInfoPathFolder = ".\\";
			string	FileInfoFileName = ".json";

			if (FileNameFormat.is_null()) FileNameFormat = "%s"s;
			if (PathFolder.is_null()) PathFolder = "%s"s;

			if (ec.obj.is_object())
			{
				json &FileInfoVal = ec.obj["FileInfo"];

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
			//jsonToFile(json(jref(EV["ctx"]), jref(EV["ent"]), FileInfo, json(), ec.obj), Model, View);
		}
	}
}


void	rmvm_dump(jsonRVM& rmvm, EntContext& ec)
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
	
	FSRM_API const string&	ImportRelationsModel(jsonRVM& rmvm)
	{
		rmvm.AddBaseEntity(rmvm["dir"], "scan"s, fs_dir_scan, "Scanning filesystem directory, $obj must be json object with PathFolder and FileNameFormat properties"s);
		rmvm.AddBaseEntity(rmvm["dir"], "create"s, fs_dir_create, "Create new directory, $obj must be string"s);
		rmvm.AddBaseEntity(rmvm["dir"], "delete"s, fs_dir_delete, "Delete directory, $obj must be string"s);

		rmvm.AddBaseEntity(rmvm["file"]["load"], "rm"s, fs_file_load_rm, "Loads rm from json"s);

		rmvm.AddBaseEntity(rmvm["file"]["read"], "json"s, fs_file_read_json, "Reads json file and parse it"s);
		rmvm.AddBaseEntity(rmvm["file"]["read"], "string"s, jsonFileToString, ""s);
		rmvm.AddBaseEntity(rmvm["file"]["read"]["array"], "string"s, jsonFileToStringArray, ""s);

		rmvm.AddBaseEntity(rmvm["file"]["write"]["array"], "string"s, jsonStringArrayToFile, ""s);
		rmvm.AddBaseEntity(rmvm["file"]["write"], "json"s, fs_file_write_json, "Write json to file"s);
		rmvm.AddBaseEntity(rmvm["files"]["write"], "json"s, jsonToFiles, ""s);

		rmvm.AddBaseEntity(rmvm["rmvm"], "dump"s, rmvm_dump, "Dump rmvm current state"s);

		return rmvm_version;
	}
	
}
