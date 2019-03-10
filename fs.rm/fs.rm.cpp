// fs.rm.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "fs.rm.h"
#include <iostream>
#include <fstream>
#include "windows.h"

#define MAX_NAME	1024


template<UINT CodePage>
wstring	_to_wstring(const string &data)
{
	size_t			nLengthW = data.length() + 1;
	wchar_t*	str = new wchar_t[nLengthW];
	memset(str, 0, nLengthW * sizeof(wchar_t));
	MultiByteToWideChar(CodePage, 0, data.c_str(), -1, str, (int)nLengthW);
	wstring	res(str);
	delete[] str;
	return res;
}

template<UINT CodePage>
string	wstring_to_(const wstring &data)
{
	size_t	size = data.length();
	if (size)
	{
		char*		str = new char[2 * size + 1];	//	for russian utf8 string case
		memset(str, 0, 2 * size + 1);
		WideCharToMultiByte(CodePage, 0, data.c_str(), -1, str, 2 * (int)size + 1, NULL, NULL);
		string		res(str);
		delete[] str;
		return res;
	}
	else
		return string();
}

inline wstring cp1251_to_wstring(const string &data) { return _to_wstring<CP_ACP>(data); }
inline wstring oem_to_wstring(const string &data) { return _to_wstring<CP_OEMCP>(data); }
inline wstring utf8_to_wstring(const string &data) { return _to_wstring<CP_UTF8>(data); }
inline string  wstring_to_cp1251(const wstring &data) { return wstring_to_<CP_ACP>(data); }
inline string  wstring_to_oem(const wstring &data) { return wstring_to_<CP_OEMCP>(data); }
inline string  wstring_to_utf8(const wstring &data) { return wstring_to_<CP_UTF8>(data); }

inline string cp1251_to_oem(const string &data) { return wstring_to_oem(_to_wstring<CP_ACP>(data)); }
inline string cp1251_to_utf8(const string &data) { return wstring_to_utf8(_to_wstring<CP_ACP>(data)); }
inline string oem_to_cp1251(const string &data) { return wstring_to_cp1251(_to_wstring<CP_OEMCP>(data)); }
inline string oem_to_utf8(const string &data) { return wstring_to_utf8(_to_wstring<CP_OEMCP>(data)); }
inline string utf8_to_cp1251(const string &data) { return wstring_to_cp1251(_to_wstring<CP_UTF8>(data)); }
inline string utf8_to_oem(const string &data) { return wstring_to_oem(_to_wstring<CP_UTF8>(data)); }


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

void  fs_dir_scan(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (!obj.is_object())
		throw(__FUNCTION__ + ": $obj must be json object with PathFolder and FileNameFormat properties!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());

	if (!obj.count("PathFolder") || !obj.count("FileNameFormat"))
		throw(__FUNCTION__ + ": $obj must be json object with PathFolder and FileNameFormat properties!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());

	string	FileNameFormat = obj["FileNameFormat"];
	string	PathFolder = obj["PathFolder"];
	json	FileInfo;
	HANDLE	handle;
	WIN32_FIND_DATA search_data;
	unsigned int i = 0;

	//	подготавливаем проекцию субъекта
	sub = json::array();	//	если адрес субъекта совпадает с проекцией сущности, то проекция пападёт в результат
	memset(&search_data, 0, sizeof(WIN32_FIND_DATA));
	handle = FindFirstFile((utf8_to_wstring(PathFolder) + utf8_to_wstring(FileNameFormat)).c_str(), &search_data);

	while (handle != INVALID_HANDLE_VALUE)
	{
		FileInfo = search_data;
		FileInfo["PathFolder"] = PathFolder;
		sub[i++] = FileInfo;

		if (FindNextFile(handle, &search_data) == FALSE)
		{
			FindClose(handle);
			break;
		}
	}

}

void  fs_dir_create(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (!obj.is_string())
		throw(__FUNCTION__ + ": $obj must be json string with PathName!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());

	string	PathName = utf8_to_cp1251(obj);
	sub = bool(CreateDirectoryA(PathName.c_str(), nullptr));
}

void  fs_dir_delete(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (!obj.is_string())
		throw(__FUNCTION__ + ": $obj must be json string with PathName!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());

	string	PathName = utf8_to_cp1251(obj);
	sub = bool(RemoveDirectoryA(PathName.c_str()));
}

void  fs_file_load_rm(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (!obj.is_object())
		throw(__FUNCTION__ + ": $obj must be json object with PathFolder and FileName properties!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());

	if (!obj.count("PathFolder") || !obj.count("FileName"))
		throw(__FUNCTION__ + ": $obj must be json object with PathFolder and FileName properties!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());

	string	PathName = utf8_to_cp1251(obj["PathFolder"].get<string>() + obj["FileName"].get<string>());
	std::ifstream in(PathName.c_str());

	if (!in.good())
		throw(__FUNCTION__ + ": Can't load json from the "s + obj["PathFolder"].get<string>() + obj["FileName"].get<string>() + " file!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());

	json	rm;
	in >> rm;

	if (rm.is_object())
	{
		Value = true;
		for (auto& p : rm.items())
			sub[p.key()] = p.value();
	}
	else
		Value = false;
}

void  fs_file_read_json(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (!obj.is_object())
		throw(__FUNCTION__ + ": $obj must be json object with PathFolder and FileName properties!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());

	if (!obj.count("PathFolder") || !obj.count("FileName"))
		throw(__FUNCTION__ + ": $obj must be json object with PathFolder and FileName properties!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());

	string	PathName = utf8_to_cp1251(obj["PathFolder"].get<string>() + obj["FileName"].get<string>());
	std::ifstream in(PathName.c_str());

	if (!in.good())
		throw(__FUNCTION__ + ": Can't load json from the "s + obj["PathFolder"].get<string>() + obj["FileName"].get<string>() + " file!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());

	Value = true;
	in >> sub;
	if (sub.is_object())
		sub["FileInfo"] = obj;

}

void  jsonFileToString(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);
	sub = json();

	if (obj.is_object())
	{
		if (obj.count("PathFolder") && obj.count("FileName"))
		{
			string	PathName = utf8_to_cp1251(obj["PathFolder"].get<string>() + obj["FileName"].get<string>());
			std::ifstream in(PathName.c_str(), ios_base::binary);

			if (in.good())
			{
				size_t size = size_t(in.seekg(0, ios_base::end).tellg().seekpos());
				in.seekg(0, ios_base::beg);
				char*	buf = new char[size + 1];
				in.read(buf, size);
				buf[size] = 0;
				sub = json(utf8_to_wstring(string(buf)));
				delete[] buf;
				Value = true;
				return;
			}
		}
	}

	Value = false;
}

void  jsonFileToStringArray(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (obj.is_object())
	{
		if (obj.count("PathFolder") && obj.count("FileName"))
		{
			string	PathName = utf8_to_cp1251(obj["PathFolder"].get<string>() + obj["FileName"].get<string>());
			std::ifstream in(PathName.c_str(), ios::binary | ios::ate);
			
			if (in.good())
			{
				Value = true;
				size_t size = in.tellg();
				in.seekg(0, ios::beg);
				char*	buf = new char[size + 1];
				in.read(buf, size);
				buf[size] = 0;
				string str(buf);
				string delim("\r\n");
				size_t prev = 0, pos = 0, i = 0;
				sub = json::array();

				do
				{
					pos = str.find(delim, prev);
					if (pos == string::npos) pos = str.length();
					string token = str.substr(prev, pos - prev);
					if (!token.empty())	sub.push_back(token);
					prev = pos + delim.length();
				} while (pos < str.length() && prev < str.length());

				delete[] buf;
				return;
			}

			throw(__FUNCTION__ + ": Can't load string array from the "s + obj["PathFolder"].get<string>() + obj["FileName"].get<string>() + " file!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
		}
	}

	throw(__FUNCTION__ + ": $obj must be json object with PathFolder and FileName property!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
}

void  jsonStringArrayToFile(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (obj.is_object() && sub.is_array())
	{
		if (obj.count("PathFolder") && obj.count("FileName"))
		{
			string	PathName = utf8_to_cp1251(obj["PathFolder"].get<string>() + obj["FileName"].get<string>());
			std::ofstream out(PathName.c_str());
			if (out.good())
			{
				for (auto& it : sub)
				{
					if (it.is_string())
						out << it.get_ref<string&>() << endl;
					else
						out << it.dump() << endl;
				}
				return;
			}
			else
				throw(__FUNCTION__ + ": Can't create the "s + obj["PathFolder"].get<string>() + obj["FileName"].get<string>() + " file!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
		}
		else
			throw(__FUNCTION__ + ": $obj must be json object with PathFolder and FileName property!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
	}
	else
		throw(__FUNCTION__ + ": $obj must be object and $sub must be array!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
}

void  fs_file_write_json(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (obj.is_object())
	{
		if (obj.count("PathFolder") && obj.count("FileName"))
		{
			string	PathName = utf8_to_cp1251(obj["PathFolder"].get<string>() + obj["FileName"].get<string>());
			std::ofstream out(PathName.c_str());

			if (out.good())
			{
				out << sub;
				Value = true;
				return;
			}

			Value = false;
			throw(__FUNCTION__ + ": Can't open "s + PathName + " file.\n"s);
		}
	}

	Value = false;
	throw(__FUNCTION__ + ": $obj must be json object with PathFolder and FileName property!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
}


void  jsonToFiles(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);
	Value = json::array();

	/*if (sub.is_array())	//	в sub массив объектов FileInfo, каждый из которых описывает файл в который надо сохранить
	{
	size_t i = 0;
	for (auto& it : sub.as_array())
	jsonToFiles(json(jref(EV["ctx"]), jref(EV["ent"]), it, json(), obj), Model, (View)[i++]);
	}
	else*/
	if (sub.is_object())
	{
		if (obj.is_array())
		{
			size_t i = 0;
			for (auto& it : obj)
			{
				json&	FileNameFormat = sub["FileNameFormat"];
				json&	PathFolder = sub["PathFolder"];
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
			json&	FileNameFormat = sub["FileNameFormat"];
			json&	PathFolder = sub["PathFolder"];
			string	FileInfoPathFolder = ".\\";
			string	FileInfoFileName = ".json";

			if (FileNameFormat.is_null()) FileNameFormat = "%s"s;
			if (PathFolder.is_null()) PathFolder = "%s"s;

			if (obj.is_object())
			{
				json &FileInfoVal = obj["FileInfo"];

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
			//jsonToFile(json(jref(EV["ctx"]), jref(EV["ent"]), FileInfo, json(), obj), Model, View);
		}
	}
}


FSRM_API void  ImportRelationsModel(json &Ent)
{
	Ent["fs"]["RVM_version"] = RVM_version;
	Addx86Entity(Ent["fs"]["dir"], "scan"s, fs_dir_scan, "Scanning filesystem directory"s);
	Addx86Entity(Ent["fs"]["dir"], "create"s, fs_dir_create, "Create new directory, $obj must be string"s);
	Addx86Entity(Ent["fs"]["dir"], "delete"s, fs_dir_delete, "Delete directory, $obj must be string"s);

	Addx86Entity(Ent["fs"]["file"]["load"], "rm"s, fs_file_load_rm, ""s);

	Addx86Entity(Ent["fs"]["file"]["read"], "json"s, fs_file_read_json, ""s);
	Addx86Entity(Ent["fs"]["file"]["read"], "string"s, jsonFileToString, ""s);
	Addx86Entity(Ent["fs"]["file"]["read"]["array"], "string"s, jsonFileToStringArray, ""s);

	Addx86Entity(Ent["fs"]["file"]["write"]["array"], "string"s, jsonStringArrayToFile, ""s);
	Addx86Entity(Ent["fs"]["file"]["write"], "json"s, fs_file_write_json, "Write json to file"s);
	Addx86Entity(Ent["fs"]["files"]["write"], "json"s, jsonToFiles, ""s);
}
