/*        c
       v__|__m
     m   _|_   v
  c__|__/_|_\__|__c  jsonRVM
     |  \_|_/  |     json Relations (Model) Virtual Machine
     v    |    m     https://github.com/netkeep80/jsonRVM
        __|__
       /  |  \
      /___|___\
Fractal Triune Entity

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
Copyright © 2016 Vertushkin Roman Pavlovich <https://vk.com/earthbirthbook>.

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
#include "JsonRVM.h"
#include "windows.h"

//	Поддержка загрузки DLL
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

inline wstring cp1251_to_wstring(const string &data) { return _to_wstring<CP_ACP>(data); }
inline wstring oem_to_wstring(const string &data) { return  _to_wstring<CP_OEMCP>(data); }
inline wstring utf8_to_wstring(const string &data) { return  _to_wstring<CP_UTF8>(data); }

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

inline string to_cp1251_string(const wstring &data) { return wstring_to_<CP_ACP>(data); }
inline string to_oem_string(const wstring &data) { return  wstring_to_<CP_OEMCP>(data); }
inline string to_utf8_string(const wstring &data) { return  wstring_to_<CP_UTF8>(data); }

class DLL
{
public:
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

	bool	LoadDict(const string& LibName)
	{
		DLLs&	it = *this;
		if (find(LibName) == end())	it[LibName] = DLL();

		if (!it[LibName].handle)
		{
			//	перезагружаем либу
			it[LibName].handle = LoadLibrary(utf8_to_wstring(LibName).c_str());

			if (it[LibName].handle)
			{
				(FARPROC &)it[LibName].Init = GetProcAddress(it[LibName].handle, IMPORT_RELATIONS_MODEL);
				if (!it[LibName].Init)
					throw(LibName + " does't has function "s + IMPORT_RELATIONS_MODEL);
			}
			else
			{
				it[LibName].Init = nullptr;
				throw("can't load '" + LibName + "' dictionary"s);
			}
		}

		return (it[LibName].Init != nullptr);
	}
} LoadedDLLs;


bool	LoadAndInitDict(const string& LibName, json &Ent)
{
	bool	Value = LoadedDLLs.LoadDict(LibName);
	if (Value) LoadedDLLs[LibName].Init(Ent);
	return Value;
}

//////////////////////////	base dictionary  ///////////////////////////////////

void  jsonLoadDLL(json &EV)
{
	json& Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);//	определяем сущность для размещения словаря
	json& obj = val2ref(EV["$obj"]);//	вычисляем имя бибилиотеки

	if (obj.is_object())
	{
		if (obj.count("PathFolder") && obj.count("FileName"))
		{
			string	FullFileName = obj["PathFolder"].get<string>() + obj["FileName"].get<string>();
			if (!sub.is_object()) sub = json::object();
			Value = LoadAndInitDict(FullFileName, sub);
			return;
		}
	}

	Value = false;
	throw(__FUNCTION__ + ": $obj must be json object with PathFolder, FileName properties!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
}


void  sleep_ms(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);
	Sleep(obj.get<json::number_unsigned_t>());
}

void  jsonCopy(json &EV)
{	//	полное копированиее json значения объекта в субъект
	val2ref(EV["$sub"]) = val2ref(EV["$obj"]);
}

void  jsonView(json &EV)
{	//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
	//	проецируем во внешнем контексте
	json	clone = val2ref(EV[".."]);
	clone[""] = EV["$sub"];
	JSONExec(clone, val2ref(EV["$obj"]));
}

template<typename _T> struct _add
{
	typedef _T& _Tref;	_T c;
	operator _Tref() { return c; };
	_add(const _T a, const _T b) { c = a + b; };
};

template<typename _T> struct _sub
{
	typedef _T& _Tref;	_T c;
	operator _Tref() { return c; };
	_sub(const _T a, const _T b) { c = a - b; };
};

template<typename _T> struct _mul
{
	typedef _T& _Tref;	_T c;
	operator _Tref() { return c; };
	_mul(const _T a, const _T b) : c(a * b) {};
};

template<typename _T> struct _div
{
	typedef _T& _Tref;	_T c;
	operator _Tref() { return c; };
	_div(const _T a, const _T b) : c(a / b) {};
};

template<typename _T, typename _OP>
json type_operation(const json& a, const json& b)
{
	return json(_T(_OP(a.get<_T>(), b.get<_T>())));
}

template<typename _T, typename _OP>
json diff_operation(const json& a, const json& b)
{
	_OP	operation(a.get<_T>(), b.get<_T>());
	_T&	res = operation;
	if (res == _T()) return json();
	return json(res);
}

template<typename _OP>
json object_operation(const json& a, const json& b)
{
	json obj;

	for (auto it = a.begin(); it != a.end(); it++)
	{
		const string& key = it.key();
		if (b.count(key)) obj[key] = _OP(it.value(), b[key]);
	}
	return obj;
}

template<typename _OP>
json array_operation(const json& a, const json& b)
{
	json	ar = json::array();
	size_t	max_size = min<size_t>(a.size(), b.size());
	for (size_t i = 0; i < max_size; i++) ar[i] = _OP(a[i], b[i]);
	return ar;
}

#define SUB_CASE(op,type,op1,op2) case json::value_t::##type: return op##_##type(op1,op2);

json operator ^ (const json& a, const json& b);

__forceinline json xor_array(const json& a, const json& b)
{
	json ar = json::array();
	bool	not_is_null = false;

	for (size_t i = 0; (i < a.size()) && (i < b.size()); i++)
	{
		ar[i] = a.at(i) ^ b.at(i);
		not_is_null = not_is_null || !ar[i].is_null();
	}

	if (a.size() > b.size())
	{
		for (size_t i = b.size(); i < a.size(); i++)
		{
			ar[i] = a.at(i);
			not_is_null = not_is_null || !ar[i].is_null();
		}
	}
	else if (a.size() < b.size())
	{
		for (size_t i = a.size(); i < b.size(); i++)
		{
			ar[i] = b.at(i);
			not_is_null = not_is_null || !ar[i].is_null();
		}
	}

	if (not_is_null) return ar;
	else return json();
}

__forceinline json xor_boolean(const json& a, const json& b)
{
	if (a.get<bool>() != b.get<bool>())	return json::array({ a, b });
	else return json();
}

__forceinline json xor_number_float(const json& a, const json& b) { return diff_operation<json::number_float_t, _sub<json::number_float_t>>(a, b); }
__forceinline json xor_number_integer(const json& a, const json& b) { return diff_operation<json::number_integer_t, _sub<json::number_integer_t>>(a, b); }
__forceinline json xor_number_unsigned(const json& a, const json& b) { return diff_operation<json::number_unsigned_t, _sub<json::number_unsigned_t>>(a, b); }

__forceinline json xor_object(const json& a, const json& b)
{
	json obj = json(), field;

	for (auto it = a.begin(); it != a.end(); it++)
	{
		const string &key = it.key();
		const json &val = it.value();
		if (b.count(key))
		{
			field = val ^ b.at(key);
			if (!field.is_null()) obj[key] = field;
		}
		else obj[key] = val;
	}

	for (auto it = b.begin(); it != b.end(); it++)
	{
		const string &key = it.key();
		const json &val = it.value();
		if (a.count(key))
		{
			field = a.at(key) ^ val;
			if (!field.is_null()) obj[key] = field;
		}
		else obj[key] = val;
	}

	return obj;
}

__forceinline json xor_string(const json& a, const json& b)
{
	if (a.get<string>() == b.get<string>()) return json();
	else return json::array({ a, b });
}

__forceinline json value_xor_array(const json& a, const json& b)
{
	json ar = json::array();
	bool	not_is_null = false;

	for (size_t i = 0; i < b.size(); i++)
	{
		ar[i] = a ^ b.at(i);
		not_is_null = not_is_null || !ar[i].is_null();
	}

	if (not_is_null) return ar;
	else return json();
}

__forceinline json array_xor_value(const json& a, const json& b)
{
	json ar = json::array();
	bool	not_is_null = false;

	for (size_t i = 0; i < a.size(); i++)
	{
		ar[i] = a.at(i) ^ b;
		not_is_null = not_is_null || !ar[i].is_null();
	}

	if (not_is_null) return ar;
	else return json();
}

json operator ^ (const json& a, const json& b)
{
	if (a.type() == b.type()) switch (a.type())
	{
		SUB_CASE(xor, array, a, b);
		SUB_CASE(xor, boolean, a, b);
		SUB_CASE(xor, number_float, a, b);
		SUB_CASE(xor, number_integer, a, b);
		SUB_CASE(xor, number_unsigned, a, b);
		SUB_CASE(xor, object, a, b);
		SUB_CASE(xor, string, a, b);
	default: return json();
	}
	else if (a.is_array()) return array_xor_value(a, b);
	else if (b.is_array()) return value_xor_array(a, b);
	else return json::array({ a, b });
}

void  jsonXOR(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);
	Value = sub ^ obj;
}

////////////////////////////////////////////////////////
/*
enum class value_t : std::uint8_t
{
	null,             ///< null value
	object,           ///< object (unordered set of name/value pairs)
	array,            ///< array (ordered collection of values)
	string,           ///< string value
	boolean,          ///< boolean value
	number_integer,   ///< number value (signed integer)
	number_unsigned,  ///< number value (unsigned integer)
	number_float,     ///< number value (floating-point)
	discarded         ///< discarded by the the parser callback function
};
*/

typedef json::number_float_t	jf;
typedef json::number_integer_t	ji;
typedef json::number_unsigned_t	ju;

#define     sub_field       4
#define     obj_field       4

#define OPP_STO(operation,stype,stype_id,otype,otype_id)										\
	case (uint8_t(json::value_t::stype_id) << sub_field) | uint8_t(json::value_t::otype_id):	\
	{ Value = (sub.get_ref<stype&>()) operation (obj.get_ref<otype&>()); return; }

#define OPP_ANYTO(operation,type,type_id)														\
	OPP_STO(operation, jf, number_float,    type, type_id)										\
	OPP_STO(operation, ji, number_integer,  type, type_id)										\
	OPP_STO(operation, ju, number_unsigned, type, type_id)

#define VM_OPP(operation)																		\
	OPP_ANYTO(operation, jf, number_float)														\
	OPP_ANYTO(operation, ji, number_integer)													\
	OPP_ANYTO(operation, ju, number_unsigned)

#define	OP_BODY( name, operation )																\
void  json##name (json &EV)																		\
{																								\
	json &Value = val2ref(EV[""]);																\
	json& sub = val2ref(EV["$sub"]);					\
	json& obj = val2ref(EV["$obj"]);					\
	switch( (uint8_t(sub.type()) << sub_field) | uint8_t(obj.type()) )					\
	{ VM_OPP( operation ) default: Value = json(); }		\
}

OP_BODY(Add, +);
OP_BODY(Substract, -);
OP_BODY(Mul, *);

void  jsonDiv(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);
	if (obj.is_number())
		if (obj.get<double>() == 0.0)
		{
			Value = json();
			return;
		}
	switch( (uint8_t(sub.type()) << sub_field) | uint8_t(obj.type()) )
	{ VM_OPP(/) default: Value = json(); }
}


//////////////////////////////////////////////////////////////////////////////////////////////
void  jsonPower(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (sub.is_number() && obj.is_number())
	{
		json::number_float_t db = obj.is_number_float() ? obj.get<json::number_float_t>() : json::number_float_t(obj.get<json::number_integer_t>());

		if (!sub.is_number_float())
		{	//	result must be integer
			json::number_float_t da = json::number_float_t(sub.get<json::number_integer_t>());
			Value = json::number_integer_t(pow(da, db) + .5);
		}
		else
		{	//	result must be double
			json::number_float_t da = sub.get<json::number_float_t>();
			Value = json::number_float_t(pow(da, db));
		}
	}
	else
		Value = json();
}

void  jsonSqrt(json &EV)
{
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (obj.is_number())
		sub = json::number_float_t(sqrt(obj.get<json::number_float_t>()));
	else
		sub = json();
}

////////////////////////////////////////////////////////////////////////////////////


void  jsonForEachObject(json &EV)
{	/*
		Множественный JSONExec для проекции объекта типа array
	*/
	json& Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& obj = val2ref(EV["$obj"]);
	json& sub = val2ref(EV["$sub"]);

	if (obj.is_array())
	{
		if (!Value.is_array()) Value = json::array();

		for (size_t i = 0; i < obj.size(); i++)
		{
			try {
				json& it = obj[i];
				json& val = Value[i];
				json  clone = EV;
				clone["$obj"] = ref2id(it);
				clone[""] = clone["$sub"] = ref2id(val);
				JSONExec(clone, sub);
			}
			catch (string& error) { throw(__FUNCTION__ + "/["s + to_string(i) + "]/"s + error); }
			catch (json::exception& e) { throw(__FUNCTION__ + "/["s + to_string(i) + "]/"s + "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
			catch (std::exception& e) { throw(__FUNCTION__ + "/["s + to_string(i) + "]/"s + "std::exception: "s + e.what()); }
			catch (...) { throw(__FUNCTION__ + "/["s + to_string(i) + "]/"s + "unknown exception"s); }
		}
	}
	else
		throw(__FUNCTION__ + ": $obj must be array!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
}

void  jsonForEachSubject(json &EV)
{
	/*
		Множественный JSONExec для субъекта типа array
	*/
	json& Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& obj = val2ref(EV["$obj"]);
	json& sub = val2ref(EV["$sub"]);

	if (sub.is_array())
	{
		if (!Value.is_array()) Value = json::array();

		for (size_t i = 0; i < sub.size(); i++)
		{
			try {
				json& it = sub[i];
				json& val = Value[i];
				json  clone = EV;
				clone["$sub"] = ref2id(it);
				clone[""] = clone["$obj"] = ref2id(val);
				JSONExec(clone, obj);
			}
			catch (string& error) { throw(__FUNCTION__ + "/["s + to_string(i) + "]/"s + error); }
			catch (json::exception& e) { throw(__FUNCTION__ + "/["s + to_string(i) + "]/"s + "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
			catch (std::exception& e) { throw(__FUNCTION__ + "/["s + to_string(i) + "]/"s + "std::exception: "s + e.what()); }
			catch (...) { throw(__FUNCTION__ + "/["s + to_string(i) + "]/"s + "unknown exception"s); }
		}
	}
	else
		throw(__FUNCTION__ + ": $sub must be array!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
}

void  jsonSize(json &EV)
{
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);
	sub = obj.size();
}

#define define_json_is_type(json_type)		\
void  json_is_##json_type(json &EV)			\
{											\
	json& sub = val2ref(EV["$sub"]);		\
	json& obj = val2ref(EV["$obj"]);		\
	sub = obj.is_##json_type();		\
}

void  json_is_not_null(json &EV)
{
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);
	sub = !obj.is_null();
}

define_json_is_type(array)
define_json_is_type(boolean)
define_json_is_type(number_float)
define_json_is_type(number_integer)
define_json_is_type(number_unsigned)
define_json_is_type(null)
define_json_is_type(number)
define_json_is_type(object)
define_json_is_type(string)
define_json_is_type(structured)
define_json_is_type(discarded)

void  jsonIntegerSequence(json &EV)
{
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (obj.is_object())
	{
		json::number_integer_t	from = obj["from"];
		json::number_integer_t	to = obj["to"];
		json::number_integer_t	step = obj.count("step") ? obj["step"] : 1;
		sub = json::array();

		for (json::number_integer_t i = from; i <= to; i += step)
			sub.push_back(i);
	}
	else
		throw(__FUNCTION__ + ": $obj must has 'from', 'to' and 'step' properties!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
}

void  jsonUnion(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (!sub.is_array())
	{
		json	tmp = sub;
		sub = json::array();
		if (!tmp.is_null())
			sub[0] = tmp;
	}

	if (!obj.is_array())
	{
		if (!obj.is_null())
			sub.push_back(obj);
	}
	else
		for (auto& it : obj)
			sub.push_back(it);

	Value = sub;
}

void  jsonNull(json &EV) {}

void  jsonInt32(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	switch (obj.type())
	{
	case json::value_t::array:
	case json::value_t::object:
		sub = obj.size();
		break;

	case json::value_t::string:
		sub = atoi(obj.get<string>().c_str());
		break;

	case json::value_t::boolean:
		if (obj.get<bool>())
			sub = int(1);
		else
			sub = int(0);
		break;

	case json::value_t::number_float:
	case json::value_t::number_integer:
	case json::value_t::number_unsigned:
		sub = obj.get<int>();
		break;

	default:
		sub = int(0);
	}

	Value = sub;
}

void  jsonDouble(json &EV)
{
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	switch (obj.type())
	{
	case json::value_t::array:
	case json::value_t::object:
		sub = json::number_float_t(obj.size());
		break;

	case json::value_t::string:
		sub = json::number_float_t(std::stod(obj.get_ref<string&>()));
		break;

	case json::value_t::boolean:
		if (obj.get<bool>())
			sub = json::number_float_t(1.0);
		else
			sub = json::number_float_t(0.0);
		break;

	case json::value_t::number_float:
		sub = obj;
		break;

	case json::value_t::number_integer:
		sub = json::number_float_t(obj.get<json::number_integer_t>());
		break;

	case json::value_t::number_unsigned:
		sub = json::number_float_t(obj.get<json::number_unsigned_t>());
		break;

	default:
		sub = json::number_float_t(0);
	}
}

void  string_split(json &EV)
{
	json& Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (obj.is_string() && sub.is_string())
	{
		const string& delim = obj.get_ref<string&>();
		string str = sub.get_ref<string&>();
		size_t prev = 0, pos = 0;
		Value = json::array();

		do
		{
			pos = str.find(delim, prev);
			if (pos == string::npos) pos = str.length();
			Value.push_back(str.substr(prev, pos - prev));
			prev = pos + delim.length();
		} while (pos < str.length() && prev < str.length());
	}
	else
		throw(__FUNCTION__ + ": $obj and $sub must be string!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
}

void  string_join(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);
	string	result = "";
	bool	first = true;

	if (obj.is_string() && sub.is_array())
	{
		string&	splitter = obj.get_ref<string&>();

		for (auto& it : sub)
		{
			if (first) first = false;
			else       result += splitter;

			switch (it.type())
			{
			case json::value_t::object:
			case json::value_t::array:
				throw(__FUNCTION__ + ": $sub must be array of simple type values!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());

			case json::value_t::number_float:
				result += to_string(it.get<json::number_float_t>());
				break;

			case json::value_t::number_integer:
				result += to_string(it.get<json::number_integer_t>());
				break;

			case json::value_t::number_unsigned:
				result += to_string(it.get<json::number_unsigned_t>());
				break;

			case json::value_t::string:
				result += it.get_ref<string&>();
				break;

			case json::value_t::boolean:
				result += it.dump();
				break;

			default:	//	null не печатаем
				break;
			}
		}

		Value = result;
	}
	else
		throw(__FUNCTION__ + ": $obj must be string and $sub must be array!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
}

void  jsonGet(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (sub.is_array())
	{
		if (obj.is_number())
			Value = sub[obj.get<size_t>()];
		else
			throw(__FUNCTION__ + ": $obj must be unsigned number!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
	}
	else if (sub.is_object())
	{
		if (obj.is_string())
			Value = sub[obj.get_ref<string&>()];
		else
			throw(__FUNCTION__ + ": $obj must be string!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
	}
	else
		throw(__FUNCTION__ + ": $sub must be array or object!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
}

void  jsonSet(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (sub.is_array())
	{
		if (obj.is_number_unsigned())
			sub[obj.get<size_t>()] = Value;
		else
			throw(__FUNCTION__ + ": $obj must be unsigned number!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
	}
	else if (sub.is_object())
	{
		if (obj.is_string())
			sub[obj.get_ref<string&>()] = Value;
		else
			throw(__FUNCTION__ + ": $obj must be string!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
	}
	else if (sub.is_null())
	{
		if (obj.is_number_unsigned())
			sub[obj.get<size_t>()] = Value;
		else if (obj.is_string())
			sub[obj.get_ref<string&>()] = Value;
		else
			throw(__FUNCTION__ + ": $obj must be unsigned number or string!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
	}
	else
		throw(__FUNCTION__ + ": $sub must be array, object or null!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
}

void  jsonErase(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);
	
	if (sub.is_object())
	{
		if (obj.is_string())
			sub.erase(obj.get_ref<string&>());
		else
			throw(__FUNCTION__ + ": $obj must be string!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
	}
	else
		throw(__FUNCTION__ + ": $sub must be object!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
}

void  jsonIsEq(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);
	Value = sub == obj;
}

void  jsonIsNotEq(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);
	Value = sub != obj;
}

void  jsonSum(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	__int64	isum = 0;
	double	dsum = 0.0;
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (obj.is_array())
	{
		for (auto& it : obj) switch (it.type())
		{
		case json::value_t::number_float:
			dsum += it.get<json::number_float_t>();
			break;

		case json::value_t::number_integer:
			isum += it.get<json::number_integer_t>();
			break;

		case json::value_t::number_unsigned:
			isum += it.get<json::number_unsigned_t>();
			break;

		default:
			break;
		}

		if (0.0 == dsum)	sub = json::number_integer_t(isum);
		else if (0 == isum)	sub = json::number_float_t(dsum);
		else				sub = json::number_float_t(dsum + json::number_float_t(isum));
		return;
	}
	else
		throw(__FUNCTION__ + ": $obj must be json array!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
}

void  jsonWhere(json &EV)
{
	json& Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& obj = val2ref(EV["$obj"]);	//	фильтруемые данные
	json& sub = val2ref(EV["$sub"]);	//	фильтр для where clause, при исполнении должен возвращать bool
	Value = json::array();		//	подготовка выходного массива
	
	if (obj.is_null()) return;

	if (obj.is_array())
	{
		for (size_t i = 0; i < obj.size(); i++)
		{
			try {
				json	boolres;
				json&	it = obj[i];
				json	clone = EV;
				clone["$obj"] = ref2id(it);
				clone[""] = clone["$sub"] = ref2id(boolres);
				JSONExec(clone, sub);
				if (boolres.is_boolean())
					if (boolres.get<bool>())
						Value.push_back(it);	//	фильтруем
			}
			catch (string& error)		{ throw(__FUNCTION__ + "/["s + to_string(i) + "]/"s + error); }
			catch (json::exception& e)	{ throw(__FUNCTION__ + "/["s + to_string(i) + "]/"s + "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
			catch (std::exception& e)	{ throw(__FUNCTION__ + "/["s + to_string(i) + "]/"s + "std::exception: "s + e.what()); }
			catch (...)					{ throw(__FUNCTION__ + "/["s + to_string(i) + "]/"s + "unknown exception"s); }
		}
	}
	else
		throw(__FUNCTION__ + ": $obj must be json array!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
}

void  jsonBelow(json &EV)	//	<
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (sub.type() == obj.type()) switch (obj.type())
	{
	case json::value_t::array:
	case json::value_t::object:
		Value = json(sub.size() < obj.size());
		return;

	case json::value_t::string:
		Value = json(sub.get<string>().size() < obj.get<string>().size());
		return;

	case json::value_t::boolean:
		Value = json(int(sub.get<bool>()) < int(obj.get<bool>()));
		return;

	case json::value_t::number_float:
		Value = json(sub.get<json::number_float_t>() < obj.get<json::number_float_t>());
		return;

	case json::value_t::number_integer:
		Value = json(sub.get<json::number_integer_t>() < obj.get<json::number_integer_t>());
		return;

	case json::value_t::number_unsigned:
		Value = json(sub.get<json::number_unsigned_t>() < obj.get<json::number_unsigned_t>());
		return;

	default:
		Value = json();
		return;
	}
}

void  jsonAnd(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (sub.is_boolean() && obj.is_boolean())
	{
		Value = sub.get<bool>() && obj.get<bool>();
	}
	else throw(__FUNCTION__ + ": $obj and $sub must be boolean!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());
}

void  IfObjTrueThenExecSub(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (!obj.is_boolean())
		throw(__FUNCTION__ + ": $obj must be boolean!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());

	try
	{
		if (obj.get<bool>())
			JSONExec(val2ref(EV[".."]), sub);
	}
	catch (string& error)		{ throw("\n "s + __FUNCTION__ + "/"s + error); }
	catch (json::exception& e)	{ throw("\n "s + __FUNCTION__ + "/"s + "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
	catch (std::exception& e)	{ throw("\n "s + __FUNCTION__ + "/"s + "std::exception: "s + e.what()); }
	catch (...)					{ throw("\n "s + __FUNCTION__ + "/"s + "unknown exception"s); }
}

void  IfObjFalseThenExecSub(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	if (!obj.is_boolean())
		throw(__FUNCTION__ + ": $obj must be boolean!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());

	try
	{
		if (!obj.get<bool>())
			JSONExec(val2ref(EV[".."]), sub);
	}
	catch (string& error)		{ throw("\n "s + __FUNCTION__ + "/"s + error); }
	catch (json::exception& e)	{ throw("\n "s + __FUNCTION__ + "/"s + "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
	catch (std::exception& e)	{ throw("\n "s + __FUNCTION__ + "/"s + "std::exception: "s + e.what()); }
	catch (...)					{ throw("\n "s + __FUNCTION__ + "/"s + "unknown exception"s); }
}

void  ExecSubWhileObjTrue(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);

	while (true)
	{
		if (!obj.is_boolean())
			throw(__FUNCTION__ + ": $obj must be boolean!\n $obj = "s + obj.dump() + "\n $sub = " + sub.dump());

		if (obj.get<bool>())
			JSONExec(val2ref(EV[".."]), sub);
		else
			return;
	}
}


#define define_object_method(object_method)	\
void  json_call_##object_method(json &EV)	\
{											\
	json& sub = val2ref(EV["$sub"]);		\
	json& obj = val2ref(EV["$obj"]);		\
	sub = obj.##object_method();	\
}

#define define_static_method(static_method)	\
void  json_call_##static_method(json &EV)	\
{											\
	json& Value = val2ref(EV[""]);		\
	Value = json::##static_method();		\
}

define_static_method(array)
define_static_method(meta)
define_static_method(object)

#include <iostream>

void  jsonPrint(json &EV)
{
	json& obj = val2ref(EV["$obj"]);
	cout << obj.dump(1) << endl;
}


void  jsonTAG(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);
	
	if (!sub.is_object())
		throw(__FUNCTION__ + ": $sub must be json object!\n $sub = "s + sub.dump());

	string	tag = sub["<>"];

	if (!Value.is_string())
		Value = ""s;

	string& body = Value.get_ref<string&>();
	body += "<" + tag;

	for (auto& it : sub.items())
		if ("<>" != it.key())
			body += " " + it.key() + "=" + it.value().dump();

	body += ">";
	//	что бы выходной поток xml попадал в тоже значение Value нужно исполнять в текущем контексте EV
	json	clone = EV, objview;
	clone[""] = ref2id(objview);
	JSONExec(clone, obj);
	if (objview.is_string()) body += objview.get_ref<string&>();
	else                     body += objview.dump();
	body += "</" + tag + ">";
}


void  jsonXML(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);
	std::string res = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"s;

	if (!sub.is_object())
		throw(__FUNCTION__ + ": $sub must be json object!\n $sub = "s + sub.dump());

	string	tag = sub["<>"];

	if (!Value.is_string())
		Value = res;
	else if(!Value.get_ref<string&>().size())
		Value = res;

	string& body = Value.get_ref<string&>();
	body += "<" + tag;

	for (auto& it : sub.items())
		if ("<>" != it.key())
			body += " " + it.key() + "=" + it.value().dump();

	body += ">";
	//	что бы выходной поток xml попадал в тоже значение Value нужно исполнять в текущем контексте EV
	json	clone = EV, objview;
	clone[""] = ref2id(objview);
	JSONExec(clone, obj);
	if (objview.is_string()) body += objview.get_ref<string&>();
	else                     body += objview.dump();
	body += "</" + tag + ">";
}


void  jsonHTML(json &EV)
{
	json &Value = val2ref(EV[""]);	//	текущее значение json проекции
	json& sub = val2ref(EV["$sub"]);
	json& obj = val2ref(EV["$obj"]);
	std::string res = "<!DOCTYPE html>"s;

	if (!sub.is_object())
		throw(__FUNCTION__ + ": $sub must be json object!\n $sub = "s + sub.dump());

	string	tag = sub["<>"];

	if (!Value.is_string())
		Value = res;
	else if (!Value.get_ref<string&>().size())
		Value = res;

	string& body = Value.get_ref<string&>();
	body += "<" + tag;

	for (auto& it : sub.items())
		if ("<>" != it.key())
			body += " " + it.key() + "=" + it.value().dump();

	body += ">";
	//	что бы выходной поток xml попадал в тоже значение Value нужно исполнять в текущем контексте EV
	json	clone = EV, objview;
	clone[""] = ref2id(objview);
	JSONExec(clone, obj);
	if (objview.is_string()) body += objview.get_ref<string&>();
	else                     body += objview.dump();
	body += "</" + tag + ">";
}

////////////////////////////////////////////////////////////////////////////////

void  ImportRelationsModel(json &Ent)
{
	Ent["RVM_version"] = RVM_version;
	Addx86Entity(Ent["RVM"]["load"], "dll"s, jsonLoadDLL, "загружает словарь сущностей из dll");
	Addx86Entity(Ent["RVM"]["sleep"], "ms"s, sleep_ms, "sleep in milliconds"s);
	Addx86Entity(Ent, "view"s, jsonView, "ViewEntity: View object model in parent ctx and then set subject value"s);
	Addx86Entity(Ent, "="s, jsonCopy, "Copy: copy object model to subject value"s);
	
#define map_json_is_type(json_type)	Addx86Entity(Ent, "is_"s + #json_type, json_is_##json_type, ""s );
	map_json_is_type(array);
	map_json_is_type(boolean);
	map_json_is_type(number_float);
	map_json_is_type(number_integer);
	map_json_is_type(number_unsigned);
	map_json_is_type(null);
	map_json_is_type(not_null);
	map_json_is_type(number);
	map_json_is_type(object);
	map_json_is_type(string);
	map_json_is_type(structured);
	map_json_is_type(discarded);

#define map_json_static_method(static_method)	Addx86Entity(Ent["json"], #static_method, json_call_##static_method, ""s );
	map_json_static_method(array);
	map_json_static_method(meta);
	map_json_static_method(object);

	//	convert
	Addx86Entity(Ent, "integer"s, jsonInt32, ""s);
	Addx86Entity(Ent, "int"s, jsonInt32, ""s);
	Addx86Entity(Ent, "float"s, jsonDouble, ""s);
	Addx86Entity(Ent, "double"s, jsonDouble, ""s);
	Addx86Entity(Ent, "null"s, jsonNull, ""s);

	//	data operations
	Addx86Entity(Ent, "where"s, jsonWhere, ""s);
	Addx86Entity(Ent, "union"s, jsonUnion, ""s);
	Addx86Entity(Ent, "size"s, jsonSize, ""s);
	Addx86Entity(Ent, "get"s, jsonGet, ""s);
	Addx86Entity(Ent, "set"s, jsonSet, ""s);
	Addx86Entity(Ent, "erase"s, jsonErase, "Удаляет элемент элементы, которые соответствуют заданному ключу."s);
	Addx86Entity(Ent["sequence"], "integer"s, jsonIntegerSequence, ""s);
	
	//	math
	Addx86Entity(Ent, "*"s, jsonMul, ""s);
	Addx86Entity(Ent, ":"s, jsonDiv, "субъект делимое, объект делитель"s);
	Addx86Entity(Ent, "+", jsonAdd, ""s);
	Addx86Entity(Ent, "-", jsonSubstract, ""s);
	Addx86Entity(Ent, "pow"s, jsonPower, ""s);
	Addx86Entity(Ent, "sqrt"s, jsonSqrt, ""s);
	Addx86Entity(Ent, "sum"s, jsonSum, ""s);

	//	logic
	Addx86Entity(Ent, "^"s, jsonXOR, ""s);
	Addx86Entity(Ent, "=="s, jsonIsEq, ""s);
	Addx86Entity(Ent, "!="s, jsonIsNotEq, ""s);
	Addx86Entity(Ent, "<"s, jsonBelow, ""s);
	Addx86Entity(Ent, "&&"s, jsonAnd, ""s);

	//	strings
	Addx86Entity(Ent["string"], "split"s, string_split, ""s);
	Addx86Entity(Ent["string"], "join"s, string_join, ""s);

	//	control
	Addx86Entity(Ent, "foreachobj"s, jsonForEachObject, ""s);
	Addx86Entity(Ent, "foreachsub"s, jsonForEachSubject, ""s);
	Addx86Entity(Ent, "then"s, IfObjTrueThenExecSub, ""s);
	Addx86Entity(Ent, "else"s, IfObjFalseThenExecSub, "");
	Addx86Entity(Ent, "while"s, ExecSubWhileObjTrue, ""s);

	//	display
	Addx86Entity(Ent, "print"s, jsonPrint, ""s);

	//	browser
	Addx86Entity(Ent, "tag"s, jsonTAG, ""s);
	Addx86Entity(Ent, "xml"s, jsonXML, ""s);
	Addx86Entity(Ent, "html"s, jsonHTML, ""s);
}
