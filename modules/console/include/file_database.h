#pragma once
#include <string>
#include <fstream>
#include "windows.h"
#include "database_api.h"


namespace rm
{
	using namespace std;
	using namespace nlohmann;

	template<UINT CodePage>
	wstring	_to_wstring(const string& data)
	{
		size_t			nLengthW = data.length() + 1;
		wchar_t* str = new wchar_t[nLengthW];
		memset(str, 0, nLengthW * sizeof(wchar_t));
		MultiByteToWideChar(CodePage, 0, data.c_str(), -1, str, (int)nLengthW);
		wstring	res(str);
		delete[] str;
		return res;
	}

	template<UINT CodePage>
	string	wstring_to_(const wstring& data)
	{
		size_t	size = data.length();
		if (size)
		{
			char* str = new char[2 * size + 1];	//	for russian utf8 string case
			memset(str, 0, 2 * size + 1);
			WideCharToMultiByte(CodePage, 0, data.c_str(), -1, str, 2 * (int)size + 1, NULL, NULL);
			string		res(str);
			delete[] str;
			return res;
		}
		else
			return string();
	}

	inline wstring cp1251_to_wstring(const string& data) { return _to_wstring<CP_ACP>(data); }
	inline wstring oem_to_wstring(const string& data) { return _to_wstring<CP_OEMCP>(data); }
	inline wstring utf8_to_wstring(const string& data) { return _to_wstring<CP_UTF8>(data); }
	inline string  wstring_to_cp1251(const wstring& data) { return wstring_to_<CP_ACP>(data); }
	inline string  wstring_to_oem(const wstring& data) { return wstring_to_<CP_OEMCP>(data); }
	inline string  wstring_to_utf8(const wstring& data) { return wstring_to_<CP_UTF8>(data); }

	inline string cp1251_to_oem(const string& data) { return wstring_to_oem(_to_wstring<CP_ACP>(data)); }
	inline string cp1251_to_utf8(const string& data) { return wstring_to_utf8(_to_wstring<CP_ACP>(data)); }
	inline string oem_to_cp1251(const string& data) { return wstring_to_cp1251(_to_wstring<CP_OEMCP>(data)); }
	inline string oem_to_utf8(const string& data) { return wstring_to_utf8(_to_wstring<CP_OEMCP>(data)); }
	inline string utf8_to_cp1251(const string& data) { return wstring_to_cp1251(_to_wstring<CP_UTF8>(data)); }
	inline string utf8_to_oem(const string& data) { return wstring_to_oem(_to_wstring<CP_UTF8>(data)); }


	class file_database_t : public database_api
	{
		string PathFolder;
	public:
		file_database_t(const string& path)
			: PathFolder(path)
		{}

		virtual void	get_entity(json& ent, const string& ent_id) override
		{
			string	PathName = PathFolder + utf8_to_cp1251(ent_id);
			std::ifstream in(PathName.c_str());

			if (!in.good())
				throw runtime_error(__FUNCTION__ + ": Can't load json from the "s + PathFolder + ent_id + " file!");

			in >> ent;
		}

		virtual void	add_entity(const json& ent, string& ent_id)
		{
			ent_id = to_string(reinterpret_cast<size_t>(&ent)) + ".json";

			string	PathName = PathFolder + utf8_to_cp1251(ent_id);
			std::ofstream out(PathName.c_str());

			if (!out.good())
				throw runtime_error(__FUNCTION__ + ": Can't open "s + PathName + " file."s);

			out << ent;
		}

		virtual void	query_entity(json& ent, const json& query)
		{
			cout << __FUNCTION__ << endl;
		}
	};
}
