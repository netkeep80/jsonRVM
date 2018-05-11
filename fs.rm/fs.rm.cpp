// fs.rm.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "fs.rm.h"
#include "iostream"
#include "fstream"
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

inline wstring cp1251_to_wstring(const string &data)	{ return _to_wstring<CP_ACP>(data); }
inline wstring oem_to_wstring(const string &data)		{ return _to_wstring<CP_OEMCP>(data); }
inline wstring utf8_to_wstring(const string &data)		{ return _to_wstring<CP_UTF8>(data); }
inline string  wstring_to_cp1251(const wstring &data)	{ return wstring_to_<CP_ACP>(data); }
inline string  wstring_to_oem(const wstring &data)		{ return wstring_to_<CP_OEMCP>(data); }
inline string  wstring_to_utf8(const wstring &data)		{ return wstring_to_<CP_UTF8>(data); }

inline string cp1251_to_oem(const string &data)		{ return wstring_to_oem(_to_wstring<CP_ACP>(data)); }
inline string cp1251_to_utf8(const string &data)	{ return wstring_to_utf8(_to_wstring<CP_ACP>(data)); }
inline string oem_to_cp1251(const string &data)		{ return wstring_to_cp1251(_to_wstring<CP_OEMCP>(data)); }
inline string oem_to_utf8(const string &data)		{ return wstring_to_utf8(_to_wstring<CP_OEMCP>(data)); }
inline string utf8_to_cp1251(const string &data)	{ return wstring_to_cp1251(_to_wstring<CP_UTF8>(data)); }
inline string utf8_to_oem(const string &data)		{ return wstring_to_oem(_to_wstring<CP_UTF8>(data)); }


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


void __fastcall fs_dir_scan(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (objview.is_object())
	{
		if (objview.count("PathFolder") && objview.count("FileNameFormat"))
		{
			string	FileNameFormat = objview["FileNameFormat"];
			string	PathFolder = objview["PathFolder"];
			json	FileInfo;
			HANDLE	handle;
			WIN32_FIND_DATA search_data;
			unsigned int i = 0;
			//	подготавливаем проекцию субъекта
			subview = json::array();	//	если адрес субъекта совпадает с проекцией сущности, то проекция пападёт в результат
			memset(&search_data, 0, sizeof(WIN32_FIND_DATA));
			handle = FindFirstFile((utf8_to_wstring(PathFolder) + utf8_to_wstring(FileNameFormat)).c_str(), &search_data);

			while (handle != INVALID_HANDLE_VALUE)
			{
				FileInfo = search_data;
				FileInfo["PathFolder"] = PathFolder;
				subview[i++] = FileInfo;

				if (FindNextFile(handle, &search_data) == FALSE)
				{
					FindClose(handle);
					break;
				}
			}
			return;
		}
	}

	throw(__FUNCTION__ + ": <-/ must be json object with PathFolder and FileNameFormat properties"s);
}

void __fastcall fs_dir_create(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (objview.is_string())
	{
		string	PathName = utf8_to_cp1251(objview);
		subview = bool(CreateDirectoryA(PathName.c_str(), nullptr));
	}
	else
		throw(__FUNCTION__ + ": <-/ must be json string with PathName"s);
}

void __fastcall fs_dir_delete(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (objview.is_string())
	{
		string	PathName = utf8_to_cp1251(objview);
		subview = bool(RemoveDirectoryA(PathName.c_str()));
	}
	else
		throw(__FUNCTION__ + ": <-/ must be json string with PathName"s);
}

void __fastcall fs_file_read_json(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (objview.is_object())
	{
		if (objview.count("PathFolder") && objview.count("FileName"))
		{
			string	PathName = utf8_to_cp1251(objview["PathFolder"].get<string>() + objview["FileName"].get<string>() );
			std::ifstream in( PathName.c_str() );

			if (in.good())
			{
				Value = true;
				in >> subview;
				if (subview.is_object()) subview["FileInfo"] = objview;
			}
			else
				throw(__FUNCTION__ + ": Can't load json from the "s + objview["PathFolder"].get<string>() + objview["FileName"].get<string>() + " file!");
		}
	}

	throw(__FUNCTION__ + ": <-/ must be json object with PathFolder and FileName properties"s);
}

void __fastcall jsonFileToString(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);
	subview = json();

	if (objview.is_object())
	{
		if (objview.count("PathFolder") && objview.count("FileName"))
		{
			string	PathName = utf8_to_cp1251(objview["PathFolder"].get<string>() + objview["FileName"].get<string>());
			std::ifstream in(PathName.c_str(), ios_base::binary);

			if (in.good())
			{
				size_t size = size_t(in.seekg(0, ios_base::end).tellg().seekpos());
				in.seekg(0, ios_base::beg);
				char*	buf = new char[size + 1];
				in.read(buf, size);
				buf[size] = 0;
				subview = json(utf8_to_wstring(string(buf)));
				delete[] buf;
				Value = true;
				return;
			}
		}
	}

	Value = false;
}

void __fastcall jsonFileToStringArray(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (objview.is_object())
	{
		if (objview.count("PathFolder") && objview.count("FileName"))
		{
			string	PathName = utf8_to_cp1251(objview["PathFolder"].get<string>() + objview["FileName"].get<string>());
			std::ifstream in(PathName.c_str(), ios_base::binary);

			if (in.good())
			{
				Value = true;
				size_t size = size_t(in.seekg(0, ios_base::end).tellg().seekpos());
				in.seekg(0, ios_base::beg);
				char*	buf = new char[size + 1];
				in.read(buf, size);
				buf[size] = 0;
				string str(buf);
				string delim("\r\n");
				size_t prev = 0, pos = 0, i = 0;
				subview = json::array();

				do
				{
					pos = str.find(delim, prev);
					if (pos == string::npos) pos = str.length();
					string token = str.substr(prev, pos - prev);
					if (!token.empty())	subview.push_back(token);
					prev = pos + delim.length();
				} while (pos < str.length() && prev < str.length());

				delete[] buf;
				return;
			}
			else
				throw(__FUNCTION__ + ": Can't load string array from the "s + objview["PathFolder"].get<string>() + objview["FileName"].get<string>() + " file!");
		}
	}
	
	throw(__FUNCTION__ + ": <-/ must be json object with PathFolder and FileName property"s);
}

void __fastcall jsonStringArrayToFile(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (objview.is_object() && subview.is_array())
	{
		if (objview.count("PathFolder") && objview.count("FileName"))
		{
			string	PathName = utf8_to_cp1251(objview["PathFolder"].get<string>() + objview["FileName"].get<string>());
			std::ofstream out(PathName.c_str());
			if (out.good())
			{
				for (auto& it : subview)
				{
					if (it.is_string())
						out << it.get_ref<string&>() << endl;
					else
						out << it.dump() << endl;
				}
				return;
			}
			else
				throw(__FUNCTION__ + ": Can't create the "s + objview["PathFolder"].get<string>() + objview["FileName"].get<string>() + " file.");
		}
		else
			throw(__FUNCTION__ + ": <-/ must be json object with PathFolder and FileName property"s);
	}
	else
		throw(__FUNCTION__ + ": <-/ must be object and -> must be array"s);
}

void __fastcall fs_file_write_json(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (objview.is_object())
	{
		if (objview.count("PathFolder") && objview.count("FileName"))
		{
			string	PathName = utf8_to_cp1251(objview["PathFolder"].get<string>() + objview["FileName"].get<string>());
			std::ofstream out(PathName.c_str());

			if (out.good())
			{
				out << subview;
				Value = true;
				return;
			}

			Value = false;
			throw(__FUNCTION__ + ": Can't open "s + PathName + " file.\n"s);
		}
	}

	Value = false;
	throw(__FUNCTION__ + ": <-/ must be json object with PathFolder and FileName property"s);
}


void __fastcall jsonToFiles(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);
	Value = json::array();

	/*if (subview.is_array())	//	в subview массив объектов FileInfo, каждый из которых описывает файл в который надо сохранить
	{
	size_t i = 0;
	for (auto& it : subview.as_array())
	jsonToFiles(json(jref(EV["ctx"]), jref(EV["ent"]), it, json(), objview), Model, (View)[i++]);
	}
	else*/
	if (subview.is_object())
	{
		if (objview.is_array())
		{
			size_t i = 0;
			for (auto& it : objview)
			{
				json&	FileNameFormat = subview["FileNameFormat"];
				json&	PathFolder = subview["PathFolder"];
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
			json&	FileNameFormat = subview["FileNameFormat"];
			json&	PathFolder = subview["PathFolder"];
			string	FileInfoPathFolder = ".\\";
			string	FileInfoFileName = ".json";

			if (FileNameFormat.is_null()) FileNameFormat = "%s"s;
			if (PathFolder.is_null()) PathFolder = "%s"s;

			if (objview.is_object())
			{
				json &FileInfoVal = objview["FileInfo"];

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
			//jsonToFile(json(jref(EV["ctx"]), jref(EV["ent"]), FileInfo, json(), objview), Model, View);
		}
	}
}


FSRM_API void __fastcall ImportRelationsModel(json &Ent)
{
	Addx86Entity(Ent["fs"]["dir"], "scan"s, fs_dir_scan, "Scanning filesystem directory"s);
	Addx86Entity(Ent["fs"]["dir"], "create"s, fs_dir_create, "Create new directory, <- must be string"s);
	Addx86Entity(Ent["fs"]["dir"], "delete"s, fs_dir_delete, "Delete directory, <- must be string"s);
	
	Addx86Entity(Ent["fs"]["file"]["read"], "json"s, fs_file_read_json, ""s);
	Addx86Entity(Ent["fs"]["file"]["read"], "string"s, jsonFileToString, ""s);
	Addx86Entity(Ent["fs"]["file"]["read"]["array"], "string"s, jsonFileToStringArray, ""s);

	Addx86Entity(Ent["fs"]["file"]["write"]["array"], "string"s, jsonStringArrayToFile, ""s);
	Addx86Entity(Ent["fs"]["file"]["write"], "json"s, fs_file_write_json, "Write json to file"s);
	Addx86Entity(Ent["fs"]["files"]["write"], "json"s, jsonToFiles, ""s);
}
