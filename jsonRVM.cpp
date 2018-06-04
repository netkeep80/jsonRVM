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


void __fastcall jsonLoadDLL(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);//	определяем сущность для размещения словаря
	ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), Value);//	вычисляем имя бибилиотеки

	if (Value.is_object())
	{
		if (Value.count("PathFolder") && Value.count("FileName"))
		{
			string	FullFileName = Value["PathFolder"].get<string>() + Value["FileName"].get<string>();
			if (!subview.is_object()) subview = json::object();
			Value = LoadedDLLs.LoadDict(FullFileName);
			if (Value) LoadedDLLs[FullFileName].Init(subview);
			return;
		}
	}

	Value = false;
	throw(__FUNCTION__ + ": <-/ must be json object with PathFolder, FileName properties"s);
}


void __fastcall sleep_ms(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);
	Sleep(objview.get<json::number_unsigned_t>());
}

void __fastcall jsonClone(json &EV, json &Value)
{	//	детальное клонирование json значения
	jref(EV["->"]) = jref(EV["<-"]);
}

void __fastcall jsonView(json &EV, json &Value)
{	//	получаем ссылку на субъект, что бы знать куда записывать проекцию объекта
	//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
	ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), jref(EV["->"]));	//	проецируем во внешнем контексте
}

void __fastcall jsonExternViewObj(json &EV, json &Value)
{
	ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), Value);	//	проецируем во внешнем контексте
}

void __fastcall jsonExternViewSub(json &EV, json &Value)
{
	ViewEntity(jref(EV["ctx"]), jref(EV["->"]), Value);	//	проецируем во внешнем контексте
}

void __fastcall jsonExec(json &EV, json &Value)
{	//	получаем ссылку на субъект, что бы знать где исполнять проекцию объекта
	//	контекст EV относится к сущности внутри которой идёт исплнение
	ExecEntity(jref(EV["ctx"]), jref(EV["<-"]), jref(EV["->"]));	//	исполняем во внешнем контексте
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

void __fastcall jsonXOR(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);
	Value = subview ^ objview;
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
	{ Value = (subview.get_ref<stype&>()) operation (objview.get_ref<otype&>()); return; }

#define OPP_ANYTO(operation,type,type_id)														\
	OPP_STO(operation, jf, number_float,    type, type_id)										\
	OPP_STO(operation, ji, number_integer,  type, type_id)										\
	OPP_STO(operation, ju, number_unsigned, type, type_id)

#define VM_OPP(operation)																		\
	OPP_ANYTO(operation, jf, number_float)														\
	OPP_ANYTO(operation, ji, number_integer)													\
	OPP_ANYTO(operation, ju, number_unsigned)

#define	OP_BODY( name, operation )																\
void __fastcall json##name (json &EV, json &Value)												\
{																								\
	json subview; ViewEntity(jref(EV["ctx"]), jref(EV["->"]), subview);							\
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);							\
	switch( (uint8_t(subview.type()) << sub_field) | uint8_t(objview.type()) )					\
	{ VM_OPP( operation ) default: throw(__FUNCTION__ + ": <-/ and ->/ must be numbers"s); }		\
}

OP_BODY(Add, +);
OP_BODY(Substract, -);
OP_BODY(Mul, *);

void __fastcall jsonDiv(json &EV, json &Value)
{
	json subview; ViewEntity(jref(EV["ctx"]), jref(EV["->"]), subview);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);
	if (objview.is_number())
		if (objview.get<double>() == 0.0)
			throw(__FUNCTION__ + ": <-/ must be not zere"s);
	switch( (uint8_t(subview.type()) << sub_field) | uint8_t(objview.type()) )
	{ VM_OPP(/) default: throw(__FUNCTION__ + ": <-/ and ->/ must be numbers"s); }
}


//////////////////////////////////////////////////////////////////////////////////////////////
void __fastcall jsonPower(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (subview.is_number() && objview.is_number())
	{
		json::number_float_t db = objview.is_number_float() ? objview.get<json::number_float_t>() : json::number_float_t(objview.get<json::number_integer_t>());

		if (!subview.is_number_float())
		{	//	result must be integer
			json::number_float_t da = json::number_float_t(subview.get<json::number_integer_t>());
			Value = json::number_integer_t(pow(da, db) + .5);
		}
		else
		{	//	result must be double
			json::number_float_t da = subview.get<json::number_float_t>();
			Value = json::number_float_t(pow(da, db));
		}
	}
	else
		Value = "power: error, both arguments must be numbers!"s;
}

void __fastcall jsonSqrt(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (objview.is_number())
		subview = json::number_float_t(sqrt(objview.get<json::number_float_t>()));
	else
		throw(__FUNCTION__ + ": /<- must be number!"s);
}

////////////////////////////////////////////////////////////////////////////////////
/*
  sub  |  obj  |                action
------------------------------------------------------------------------------------
 null  | null  | foreach val in Value: val = Value(val)
 null  | objv  | foreach val in objv:   val = Value(val)
 subv  | null  | foreach val in Value: val = subv(val)
 subv  | objv  | foreach val in objv:   val = subv(val)
*/
void __fastcall jsonForEach(json &EV, json &Value)
{
	json subview = jref(EV["->"]);
	ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), Value);

	if (Value.is_array())
	{
		for (auto& it : Value)
		{
			json	objview = it;	//	кэшируем значение
			EV["<-"] = jrefval(objview);
			EV[""] = EV["->"] = jrefval(it);
			//EV[""] = EV["<-"] = EV["->"] = &it;
			ExecEntity(EV, subview, it);
		}
		EV[""] = jrefval(Value);
	}
	else
		throw(__FUNCTION__ + ": <-/ must be array"s);
}

void __fastcall jsonSize(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);
	size_t	size = objview.size();
	if (!size)
	{
		objview = json();
	}
	subview = size;
}

#define define_json_is_type(json_type)						\
void __fastcall json_is_##json_type(json &EV, json &Value)\
{															\
	json& subview = jref(EV["->"]);								\
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);	\
	subview = objview.is_##json_type();						\
}

void __fastcall json_is_not_null(json &EV, json &Value)	\
{															\
	json& subview = jref(EV["->"]);								\
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);	\
	subview = !objview.is_null();							\
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

void __fastcall jsonIntegerSequence(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (objview.is_object())
	{
		json::number_integer_t	from = objview["from"];
		json::number_integer_t	to = objview["to"];
		json::number_integer_t	step = objview.count("step") ? objview["step"] : 1;
		subview = json::array();

		for (json::number_integer_t i = from; i <= to; i += step)
			subview.push_back(i);
	}
	else
		throw(__FUNCTION__ + ": <-/ must has 'from', 'to' and 'step' properties"s);
}

void __fastcall jsonUnion(json &EV, json &Value)
{
	json subview; ViewEntity(jref(EV["ctx"]), jref(EV["->"]), subview);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (!subview.is_array())
	{
		json	tmp = subview;
		subview = json::array();
		if (!tmp.is_null())
			subview[0] = tmp;
	}

	if (!objview.is_array())
	{
		if (!objview.is_null())
			subview.push_back(objview);
	}
	else
		for (auto& it : objview)
			subview.push_back(it);

	Value = subview;
}

void __fastcall jsonNull(json &EV, json &Value) {}

void __fastcall jsonInt32(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	switch (objview.type())
	{
	case json::value_t::array:
	case json::value_t::object:
		subview = objview.size();
		break;

	case json::value_t::string:
		subview = atoi(objview.get<string>().c_str());
		break;

	case json::value_t::boolean:
		if (objview.get<bool>())
			subview = int(1);
		else
			subview = int(0);
		break;

	case json::value_t::number_float:
	case json::value_t::number_integer:
	case json::value_t::number_unsigned:
		subview = objview.get<int>();
		break;

	default:
		subview = int(0);
	}

	Value = subview;
}

void __fastcall jsonDouble(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	switch (objview.type())
	{
	case json::value_t::array:
	case json::value_t::object:
		subview = json::number_float_t(objview.size());
		break;

	case json::value_t::string:
		subview = json::number_float_t(std::stod(objview.get_ref<string&>()));
		break;

	case json::value_t::boolean:
		if (objview.get<bool>())
			subview = json::number_float_t(1.0);
		else
			subview = json::number_float_t(0.0);
		break;

	case json::value_t::number_float:
		subview = objview;
		break;

	case json::value_t::number_integer:
		subview = json::number_float_t(objview.get<json::number_integer_t>());
		break;

	case json::value_t::number_unsigned:
		subview = json::number_float_t(objview.get<json::number_unsigned_t>());
		break;

	default:
		subview = json::number_float_t(0);
	}
}

void __fastcall string_split(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (objview.is_string() && subview.is_string())
	{
		const string& delim = objview.get_ref<string&>();
		string str = subview.get_ref<string&>();
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
		throw(__FUNCTION__ + ": <-/ and -> must be string"s);
}

void __fastcall string_join(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);
	string	result = "";
	bool	first = true;

	if (objview.is_string() && subview.is_array())
	{
		string&	splitter = objview.get_ref<string&>();

		for (auto& it : subview)
		{
			if (first) first = false;
			else       result += splitter;

			switch (it.type())
			{
			case json::value_t::object:
			case json::value_t::array:
				throw(__FUNCTION__ + ": -> must be array of simple type values"s);

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
		throw(__FUNCTION__ + ": <-/ must be string and -> must be array"s);
}

void __fastcall jsonGet(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (subview.is_array())
	{
		if (objview.is_number())
			Value = subview[objview.get<size_t>()];
		else
			throw(__FUNCTION__ + ": <-/ must be unsigned number"s);
	}
	else if (subview.is_object())
	{
		if (objview.is_string())
			Value = subview[objview.get_ref<string&>()];
		else
			throw(__FUNCTION__ + ": <-/ must be string"s);
	}
	else
		throw(__FUNCTION__ + ": -> must be array or object"s);
}

void __fastcall jsonSet(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (subview.is_array())
	{
		if (objview.is_number_unsigned())
			subview[objview.get<size_t>()] = Value;
		else
			throw(__FUNCTION__ + ": <-/ must be unsigned number"s);
	}
	else if (subview.is_object())
	{
		if (objview.is_string())
			subview[objview.get_ref<string&>()] = Value;
		else
			throw(__FUNCTION__ + ": <-/ must be string"s);
	}
	else if (subview.is_null())
	{
		if (objview.is_number_unsigned())
			subview[objview.get<size_t>()] = Value;
		else if (objview.is_string())
			subview[objview.get_ref<string&>()] = Value;
		else
			throw(__FUNCTION__ + ": <-/ must be unsigned number or string"s);
	}
	else
		throw(__FUNCTION__ + ": -> must be array, object or null"s);
}

void __fastcall jsonErase(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);
	
	if (subview.is_object())
	{
		if (objview.is_string())
			subview.erase(objview.get_ref<string&>());
		else
			throw(__FUNCTION__ + ": <-/ must be string"s);
	}
	else
		throw(__FUNCTION__ + ": -> must be object"s);
}

void __fastcall jsonIsEq(json &EV, json &Value)
{
	json subview; ViewEntity(jref(EV["ctx"]), jref(EV["->"]), subview);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);
	Value = subview == objview;
}

void __fastcall jsonIsNotEq(json &EV, json &Value)
{
	json subview; ViewEntity(jref(EV["ctx"]), jref(EV["->"]), subview);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);
	Value = subview != objview;
}

void __fastcall jsonSum(json &EV, json &Value)
{
	__int64	isum = 0;
	double	dsum = 0.0;
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (objview.is_array())
	{
		for (auto& it : objview) switch (it.type())
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

		if (0.0 == dsum)	subview = json::number_integer_t(isum);
		else if (0 == isum)	subview = json::number_float_t(dsum);
		else				subview = json::number_float_t(dsum + json::number_float_t(isum));
		return;
	}
	else
		throw(__FUNCTION__ + ": <-/ must be json array"s);
}

void __fastcall jsonWhere(json &EV, json &Value)
{
	json subview = jref(EV["->"]);	//	фильтр для where clause, при исполнении должен возвращать bool
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);	//	фильтруемые данные
	Value = json::array();		//	подготовка выходного массива
	
	if (objview.is_array())
	{
		for (auto& it : objview)
		{
			json	boolres = it;
			EV["<-"] = jrefval(it);
			EV[""] = EV["->"] = jrefval(boolres);
			ExecEntity(EV, subview, boolres);
			if (boolres.is_boolean())
				if (boolres.get_ref<bool&>())
					Value.push_back(it);	//	фильтруем
		}
		EV[""] = jrefval(Value);
	}
	else if (objview.is_null())
	{
		Value = json();
	}
	else
		throw(__FUNCTION__ + ": <-/ must be json array"s);
}

void __fastcall jsonBelow(json &EV, json &Value)	//	<
{
	json subview; ViewEntity(jref(EV["ctx"]), jref(EV["->"]), subview);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (subview.type() == objview.type()) switch (objview.type())
	{
	case json::value_t::array:
	case json::value_t::object:
		Value = json(subview.size() < objview.size());
		return;

	case json::value_t::string:
		Value = json(subview.get<string>().size() < objview.get<string>().size());
		return;

	case json::value_t::boolean:
		Value = json(int(subview.get<bool>()) < int(objview.get<bool>()));
		return;

	case json::value_t::number_float:
		Value = json(subview.get<json::number_float_t>() < objview.get<json::number_float_t>());
		return;

	case json::value_t::number_integer:
		Value = json(subview.get<json::number_integer_t>() < objview.get<json::number_integer_t>());
		return;

	case json::value_t::number_unsigned:
		Value = json(subview.get<json::number_unsigned_t>() < objview.get<json::number_unsigned_t>());
		return;

	default:
		Value = json();
		return;
	}
}

void __fastcall jsonAnd(json &EV, json &Value)
{
	json subview; ViewEntity(jref(EV["ctx"]), jref(EV["->"]), subview);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (subview.is_boolean() && objview.is_boolean())
	{
		Value = subview.get<bool>() && objview.get<bool>();
	}
	else throw(__FUNCTION__ + ": <-/ and ->/ must be boolean"s);
}

void __fastcall IfObjTrueThenExecSub(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (!objview.is_boolean())
		throw(__FUNCTION__ + ": <-/ must be boolean!"s);

	if (objview.get<bool>())
		ExecEntity(jref(EV["ctx"]), subview, Value);
}

void __fastcall IfObjFalseThenExecSub(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (!objview.is_boolean())
		throw(__FUNCTION__ + ": <-/ must be boolean!"s);

	if (!objview.get<bool>())
		ExecEntity(jref(EV["ctx"]), subview, Value);
}

void __fastcall ExecSubWhileObjTrue(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);

	while (true)
	{
		ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), Value);

		if (!Value.is_boolean())
		{
			Value = false;
			throw(__FUNCTION__ + ": <-/ must be boolean!"s);
		}

		if (Value.get<bool>())
			ExecEntity(jref(EV["ctx"]), subview, Value);
		else
			return;
	}
}

////////////////////////////////////////////////////////////////////////////////

void __fastcall ImportRelationsModel(json &Ent)
{
	Addx86Entity(Ent["RVM"]["load"], "dll"s, jsonLoadDLL, "загружает словарь сущностей из dll");
	Addx86Entity(Ent["RVM"]["sleep"], "ms"s, sleep_ms, "sleep in milliconds"s);
	Addx86Entity(Ent, "view"s, jsonView, "ViewEntity: View object model in parent ctx and then set subject value"s);
	Addx86Entity(Ent["extern"], "<-"s, jsonExternViewObj, "View object model in parent ctx and then set Value"s);
	Addx86Entity(Ent["extern"], "->"s, jsonExternViewSub, "View subject model in parent ctx and then set Value"s);
	Addx86Entity(Ent["RVM"], "exec"s, jsonExec, "EcexEntity: Executes object model in parent ctx and then set subject value"s);
	Addx86Entity(Ent, "="s, jsonView, "ViewEntity: View object model in parent ctx and then set subject value"s);
	Addx86Entity(Ent, "clone"s, jsonClone, "clone: clone object model to subject"s);
	
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
	Addx86Entity(Ent, "foreach"s, jsonForEach, ""s);
	Addx86Entity(Ent, "then"s, IfObjTrueThenExecSub, ""s);
	Addx86Entity(Ent, "else"s, IfObjFalseThenExecSub, "");
	Addx86Entity(Ent, "while"s, ExecSubWhileObjTrue, ""s);
}
