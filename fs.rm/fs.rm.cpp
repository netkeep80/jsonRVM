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


void	jsonScanFolder(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview = EV["->"];
	jsonPtr objview; EV.GetObjView(objview);

	if (objview->is_object())
	{
		if (objview->count("PathFolder"))
		{
			if (objview->count("FileNameFormat"))
			{
				json	FileNameFormat = (*objview)["FileNameFormat"];
				json	PathFolder = (*objview)["PathFolder"];
				json	FileInfo, FileInfoVal;
				HANDLE	handle;
				WIN32_FIND_DATA search_data;
				//	записываем в результат указатель на значение субъекта
				*ResPtr = subview;
				//	подготавливаем проекцию субъекта
				*subview = json::array();	//	если адрес субъекта совпадает с проекцией сущности, то проекция пападёт в результат

				FileInfoVal["PathFolder"] = PathFolder;
				FileInfo["FileInfo"] = FileInfoVal;
				memset(&search_data, 0, sizeof(WIN32_FIND_DATA));
				handle = FindFirstFile((utf8_to_wstring(PathFolder) + utf8_to_wstring(FileNameFormat)).c_str(), &search_data);
				unsigned int i = subview->size();

				while (handle != INVALID_HANDLE_VALUE)
				{
					FileInfo["FileInfo"]["FileName"] = wstring_to_utf8(search_data.cFileName);
					(*subview)[i++] = FileInfo;

					if (FindNextFile(handle, &search_data) == FALSE)
					{
						FindClose(handle);
						break;
					}
				}
				return;
			}
		}
	}

	EV.ErrorMessage("ScanFolder"s, "ent/<- must be json object with PathFolder and FileNameFormat properties"s, ResPtr);
}


void	jsonFromFile(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);

	//	в objview массив объектов FileInfo, каждый из которых описывает файл который надо загрузить
	/*if (objview->is_array())
	{
		subview = json::array();
		for (size_t i = 0; i < objview->size(); i++)
		{
			EV["->/="] = &(*subview)[i];
			EV["<-/="] = &(*objview)[i];
			jsonFromFile(EV, Model, View);
		}
		//	возвращаем объединённый массив
		EV["->/="] = &subview;
	}
	else*/ if (objview->is_object())
	{
		json &FileInfoVal = (*objview)["FileInfo"];
		if (FileInfoVal.is_object())
		{
			string	PathName = utf8_to_cp1251(FileInfoVal["PathFolder"].get<string>() + "\\"s + FileInfoVal["FileName"].get<string>() );
			std::ifstream in( PathName.c_str() );

			if (in.good())
			{
				in >> *subview;
				if (subview->is_object()) (*subview)["FileInfo"] = FileInfoVal;
				*ResPtr = true;
			}
			else
				*ResPtr = json("Can't restore object from the " + FileInfoVal["PathFolder"].get<string>() + FileInfoVal["FileName"].get<string>() + " file!");
			return;
		}
	}

	*ResPtr = false;
}


void	jsonFileToString(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr objview; EV.GetObjView(objview);
	jsonPtr	subview = EV["->"];
	*subview = json();

	if (objview->is_object())
	{
		json &FileInfoVal = (*objview)["FileInfo"];
		if (FileInfoVal.is_object())
		{
			std::ifstream in((utf8_to_cp1251(FileInfoVal["PathFolder"]) + "\\" + utf8_to_cp1251(FileInfoVal["FileName"])).c_str(), ios_base::binary);

			if (in.good())
			{
				size_t size = size_t(in.seekg(0, ios_base::end).tellg().seekpos());
				in.seekg(0, ios_base::beg);
				char*	buf = new char[size + 1];
				in.read(buf, size);
				buf[size] = 0;
				(*subview) = json(utf8_to_wstring(string(buf)));
				delete[] buf;
				*ResPtr = true;
				return;
			}
		}
	}

	*ResPtr = false;
}


void	jsonFileToStringArray(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);

	if (objview->is_object())
	{
		json &FileInfoVal = (*objview)["FileInfo"];
		if (FileInfoVal.is_object())
		{
			std::ifstream in((utf8_to_cp1251(FileInfoVal["PathFolder"]) + "\\" + utf8_to_cp1251(FileInfoVal["FileName"])).c_str(), ios_base::binary);

			if (in.good())
			{
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
					if (!token.empty())	(*subview)[i++] = json(utf8_to_wstring(token));
					prev = pos + delim.length();
				} while (pos < str.length() && prev < str.length());

				delete[] buf;
			}
		}
	}

	*ResPtr = subview;
}



void	jsonStringArrayToFile(EntView &EV, const jsonPtr& ResPtr)
{
	json FileInfoVal;
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);

	if (subview->is_object())
	{
		FileInfoVal = (*subview)["FileInfo"];
		if (objview->is_array())
		{
			if (FileInfoVal.is_object())
			{
				std::ofstream out((utf8_to_cp1251(FileInfoVal["PathFolder"]) + "\\" + utf8_to_cp1251(FileInfoVal["FileName"])).c_str());
				if (out.good())
				{
					for (auto& it : *objview) out << utf8_to_cp1251(it) << endl;
				}
				else
					*ResPtr = json("Can't open the " + FileInfoVal["PathFolder"].get<string>() + FileInfoVal["FileName"].get<string>() + " file.\n");
			}
		}
		else if (objview->is_string())
		{
			*ResPtr = json("Can't store object in the " + FileInfoVal["PathFolder"].get<string>() + FileInfoVal["FileName"].get<string>() + " file, object is not array.\n");
		}
	}
	else
		*ResPtr = json("Can't store object, FileInfo is absent.\n");
}



void	jsonToFile(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);

									  /*if (subview->is_array())	//	в subview массив объектов FileInfo, каждый из которых описывает файл в который надо сохранить
									  {	//	если передано несколько имен файлов то записываем в несколько файлов одно и тоже
									  View = json::array();
									  size_t i = 0;
									  for (auto& it : subview->as_array())
									  jsonToFile(EntView(EV.parent, (*EV["ent"]), it, json(), objview), Model, (View)[i++]);
									  }
									  else*/
	if (subview->is_object())
	{
		json FileInfoVal = (*subview)["FileInfo"];

		if (FileInfoVal.is_object())
		{
			std::ofstream out((utf8_to_cp1251(FileInfoVal["PathFolder"]) + "\\"s + utf8_to_cp1251(FileInfoVal["FileName"])).c_str());

			if (out.good())
				out << objview;
			else
				*ResPtr = json("Can't store object in the "s + FileInfoVal["PathFolder"].get<string>() + FileInfoVal["FileName"].get<string>() + " file.\n"s);
		}

		*ResPtr = subview;
	}
}


void	jsonToFiles(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);
	*ResPtr = json::array();

	/*if (subview->is_array())	//	в subview массив объектов FileInfo, каждый из которых описывает файл в который надо сохранить
	{
	size_t i = 0;
	for (auto& it : subview->as_array())
	jsonToFiles(EntView(EV.parent, (*EV["ent"]), it, json(), objview), Model, (View)[i++]);
	}
	else*/
	if (subview->is_object())
	{
		if (objview->is_array())
		{
			size_t i = 0;
			for (auto& it : *objview)
			{
				json&	FileNameFormat = (*subview)["FileNameFormat"];
				json&	PathFolder = (*subview)["PathFolder"];
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
				//jsonToFile(EntView(EV.parent, (*EV["ent"]), FileInfo, json(), it), Model, View[i++]);
			}
		}
		else
		{
			json&	FileNameFormat = (*subview)["FileNameFormat"];
			json&	PathFolder = (*subview)["PathFolder"];
			string	FileInfoPathFolder = ".\\";
			string	FileInfoFileName = ".json";

			if (FileNameFormat.is_null()) FileNameFormat = "%s"s;
			if (PathFolder.is_null()) PathFolder = "%s"s;

			if (objview->is_object())
			{
				json &FileInfoVal = (*objview)["FileInfo"];

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
			//jsonToFile(EntView(EV.parent, (*EV["ent"]), FileInfo, json(), objview), Model, View);
		}
	}
}


FSRM_API void ImportRelationsModel(json &Ent)
{
	Addx86Entity(Ent["fs"]["dir"], "scan"s, jsonScanFolder, "Scanning filesystem directory"s);
	
	Addx86Entity(Ent["fs"]["file"]["read"], "json"s, jsonFromFile, ""s);
	Addx86Entity(Ent["fs"]["file"]["read"], "string"s, jsonFileToString, ""s);
	Addx86Entity(Ent["fs"]["file"]["read"], "array_of_string"s, jsonFileToStringArray, ""s);

	Addx86Entity(Ent["fs"]["file"]["write"], "array_of_string"s, jsonStringArrayToFile, ""s);
	Addx86Entity(Ent["fs"]["file"]["write"], "json"s, jsonToFile, ""s);
	Addx86Entity(Ent["fs"]["files"]["write"], "json"s, jsonToFiles, ""s);
}
