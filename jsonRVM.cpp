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
	bool	val = LoadedDLLs.LoadDict(LibName);
	if (val) LoadedDLLs[LibName].Init(Ent);
	return val;
}

//////////////////////////	base dictionary  ///////////////////////////////////

void  jsonLoadDLL(EntContext& ec)
{
	if (ec.obj.is_object())
	{
		if (ec.obj.count("PathFolder") && ec.obj.count("FileName"))
		{
			string	FullFileName = ec.obj["PathFolder"].get<string>() + ec.obj["FileName"].get<string>();
			if (!ec.sub.is_object()) ec.sub = json::object();
			ec.val = LoadAndInitDict(FullFileName, ec.sub);
			return;
		}
	}

	ec.val = false;
	throw json({ { __FUNCTION__, "$obj must be json object with PathFolder, FileName properties!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
}


void  sleep_ms(EntContext& ec)
{
	Sleep(ec.obj.get<json::number_unsigned_t>());
}

void  jsonCopy(EntContext& ec)
{	//	полное копированиее json значения объекта в субъект
	ec.sub = ec.obj;
}

void  jsonView(EntContext& ec)
{	//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
	//	проецируем во внешнем контексте
	JSONExec(EntContext(ec.sub, ec.ctx.obj, ec.ctx.sub, ec.ctx.ent, ec.ctx, ec.root), ec.obj);
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
	json ec.obj;

	for (auto it = a.begin(); it != a.end(); it++)
	{
		const string& key = it.key();
		if (b.count(key)) ec.obj[key] = _OP(it.value(), b[key]);
	}
	return ec.obj;
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
	if (a.get<bool>() != b.get<bool>())	return json::array({ a, b});
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
	else return json::array({ a, b});
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
	else return json::array({ a, b});
}

void  jsonXOR(EntContext& ec)
{
	ec.val = ec.sub ^ ec.obj;
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
	{ ec.val = (ec.sub.get_ref<stype&>()) operation (ec.obj.get_ref<otype&>()); return; }

#define OPP_ANYTO(operation,type,type_id)														\
	OPP_STO(operation, jf, number_float,    type, type_id)										\
	OPP_STO(operation, ji, number_integer,  type, type_id)										\
	OPP_STO(operation, ju, number_unsigned, type, type_id)

#define VM_OPP(operation)																		\
	OPP_ANYTO(operation, jf, number_float)														\
	OPP_ANYTO(operation, ji, number_integer)													\
	OPP_ANYTO(operation, ju, number_unsigned)

#define	OP_BODY( name, operation )																\
void  json##name (EntContext& ec)			\
{																								\
	switch( (uint8_t(ec.sub.type()) << sub_field) | uint8_t(ec.obj.type()) )							\
	{ VM_OPP( operation ) default: ec.val = json(); }												\
}

OP_BODY(Add, +);
OP_BODY(Substract, -);
OP_BODY(Mul, *);

void  jsonDiv(EntContext& ec)
{
	if (ec.obj.is_number())
		if (ec.obj.get<double>() == 0.0)
		{
			ec.val = json();
			return;
		}
	switch( (uint8_t(ec.sub.type()) << sub_field) | uint8_t(ec.obj.type()) )
	{ VM_OPP(/) default: ec.val = json(); }
}


//////////////////////////////////////////////////////////////////////////////////////////////
void  jsonPower(EntContext& ec)
{
	if (ec.sub.is_number() && ec.obj.is_number())
	{
		json::number_float_t db = ec.obj.is_number_float() ? ec.obj.get<json::number_float_t>() : json::number_float_t(ec.obj.get<json::number_integer_t>());

		if (!ec.sub.is_number_float())
		{	//	result must be integer
			json::number_float_t da = json::number_float_t(ec.sub.get<json::number_integer_t>());
			ec.val = json::number_integer_t(pow(da, db) + .5);
		}
		else
		{	//	result must be double
			json::number_float_t da = ec.sub.get<json::number_float_t>();
			ec.val = json::number_float_t(pow(da, db));
		}
	}
	else
		ec.val = json();
}

void  jsonSqrt(EntContext& ec)
{
	if (ec.obj.is_number())
		ec.sub = json::number_float_t(sqrt(ec.obj.get<json::number_float_t>()));
	else
		ec.sub = json();
}

////////////////////////////////////////////////////////////////////////////////////


void  jsonForEachObject(EntContext& ec)
{	/*
		Множественный JSONExec для проекции объекта типа array
	*/
	if (ec.obj.is_array())
	{
		if (!ec.val.is_array()) ec.val = json::array();
		size_t i = 0;
		for (auto& it: ec.obj)
		{
			try {
				json& value = ec.val[i];
				JSONExec(EntContext(value, it, value, ec.ent, ec.ctx, ec.root), ec.sub); i++;
			}
			catch (json& j) { throw json({ { __FUNCTION__, {"/["s + to_string(i) + "]/"s, j }} }); }
		}
	}
	else
		throw json({ { __FUNCTION__, "$obj must be array!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
}

void  jsonForEachSubject(EntContext& ec)
{	/*
		Множественный JSONExec для субъекта типа array
	*/
	if (ec.sub.is_array())
	{
		if (!ec.val.is_array()) ec.val = json::array();
		size_t i = 0;
		for (auto& it : ec.sub)
		{
			try {
				json& value = ec.val[i];
				JSONExec(EntContext(value, it, value, ec.ent, ec.ctx, ec.root), ec.obj); i++;
			}
			catch (json& j) { throw json({ { __FUNCTION__, {"/["s + to_string(i) + "]/"s, j }} }); }
		}
	}
	else
		throw json({ { __FUNCTION__, "$sub must be array!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
}

void  jsonSize(EntContext& ec)
{
	ec.sub = ec.obj.size();
}

#define define_json_is_type(json_type)															\
void  json_is_##json_type(EntContext& ec)	\
{																								\
	ec.sub = ec.obj.is_##json_type();																	\
}

void  json_is_not_null(EntContext& ec)
{
	ec.sub = !ec.obj.is_null();
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

void  jsonIntegerSequence(EntContext& ec)
{
	if (ec.obj.is_object())
	{
		json::number_integer_t	from = ec.obj["from"];
		json::number_integer_t	to = ec.obj["to"];
		json::number_integer_t	step = ec.obj.count("step") ? ec.obj["step"] : 1;
		ec.sub = json::array();

		for (json::number_integer_t i = from; i <= to; i += step)
			ec.sub.push_back(i);
	}
	else
		throw json({ { __FUNCTION__, "$obj must has 'from', 'to' and 'step' properties!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
}

void  jsonUnion(EntContext& ec)
{
	if (!ec.sub.is_array())
	{
		json	tmp = ec.sub;
		ec.sub = json::array();
		if (!tmp.is_null())
			ec.sub[0] = tmp;
	}

	if (!ec.obj.is_array())
	{
		if (!ec.obj.is_null())
			ec.sub.push_back(ec.obj);
	}
	else
		for (auto& it : ec.obj)
			ec.sub.push_back(it);

	ec.val = ec.sub;
}

void  jsonNull(EntContext& ec) {}

void  jsonInt32(EntContext& ec)
{
	switch (ec.obj.type())
	{
	case json::value_t::array:
	case json::value_t::object:
		ec.sub = ec.obj.size();
		break;

	case json::value_t::string:
		ec.sub = atoi(ec.obj.get<string>().c_str());
		break;

	case json::value_t::boolean:
		if (ec.obj.get<bool>())
			ec.sub = int(1);
		else
			ec.sub = int(0);
		break;

	case json::value_t::number_float:
	case json::value_t::number_integer:
	case json::value_t::number_unsigned:
		ec.sub = ec.obj.get<int>();
		break;

	default:
		ec.sub = int(0);
	}

	ec.val = ec.sub;
}

void  jsonDouble(EntContext& ec)
{
	switch (ec.obj.type())
	{
	case json::value_t::array:
	case json::value_t::object:
		ec.sub = json::number_float_t(ec.obj.size());
		break;

	case json::value_t::string:
		ec.sub = json::number_float_t(std::stod(ec.obj.get_ref<string&>()));
		break;

	case json::value_t::boolean:
		if (ec.obj.get<bool>())
			ec.sub = json::number_float_t(1.0);
		else
			ec.sub = json::number_float_t(0.0);
		break;

	case json::value_t::number_float:
		ec.sub = ec.obj;
		break;

	case json::value_t::number_integer:
		ec.sub = json::number_float_t(ec.obj.get<json::number_integer_t>());
		break;

	case json::value_t::number_unsigned:
		ec.sub = json::number_float_t(ec.obj.get<json::number_unsigned_t>());
		break;

	default:
		ec.sub = json::number_float_t(0);
	}
}


void  string_string(EntContext& ec)
{
	ec.sub = ""s;
	string&	result = ec.sub.get_ref<string&>();

	switch (ec.obj.type())
	{
	case json::value_t::number_float:
		result = to_string(ec.obj.get<json::number_float_t>());
		break;

	case json::value_t::number_integer:
		result = to_string(ec.obj.get<json::number_integer_t>());
		break;

	case json::value_t::number_unsigned:
		result = to_string(ec.obj.get<json::number_unsigned_t>());
		break;

	case json::value_t::string:
		result = ec.obj.get_ref<string&>();
		break;

	default:
		result = ec.obj.dump();
		break;
	}
}


void  string_add(EntContext& ec)
{
	if(!ec.sub.is_string())	ec.sub = ""s;
	string&	result = ec.sub.get_ref<string&>();

	switch (ec.obj.type())
	{
	case json::value_t::number_float:
		result += to_string(ec.obj.get<json::number_float_t>());
		break;

	case json::value_t::number_integer:
		result += to_string(ec.obj.get<json::number_integer_t>());
		break;

	case json::value_t::number_unsigned:
		result += to_string(ec.obj.get<json::number_unsigned_t>());
		break;

	case json::value_t::string:
		result += ec.obj.get_ref<string&>();
		break;

	default:
		result += ec.obj.dump();
		break;
	}
}


void  string_find(EntContext& ec)
{
	if (!(ec.obj.is_string() && ec.sub.is_string())) throw json({ { __FUNCTION__, "$obj and $sub must be strings!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
	ec.val = static_cast<json::number_integer_t>(ec.obj.get_ref<string&>().find(ec.sub.get_ref<string&>().c_str()));
}


void  string_split(EntContext& ec)
{
	if (ec.obj.is_string() && ec.sub.is_string())
	{
		const string& delim = ec.obj.get_ref<string&>();
		string str = ec.sub.get_ref<string&>();
		size_t prev = 0, pos = 0;
		ec.val = json::array();

		do
		{
			pos = str.find(delim, prev);
			if (pos == string::npos) pos = str.length();
			ec.val.push_back(str.substr(prev, pos - prev));
			prev = pos + delim.length();
		} while (pos < str.length() && prev < str.length());
	}
	else
		throw json({ { __FUNCTION__, "$obj and $sub must be string!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
}

void  string_join(EntContext& ec)
{
	string	result = "";
	bool	first = true;

	if (ec.obj.is_string() && ec.sub.is_array())
	{
		string&	splitter = ec.obj.get_ref<string&>();

		for (auto& it : ec.sub)
		{
			if (first) first = false;
			else       result += splitter;

			switch (it.type())
			{
			case json::value_t::object:
			case json::value_t::array:
				throw json({ { __FUNCTION__, "$sub must be array of simple type values!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });

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

		ec.val = result;
	}
	else
		throw json({ { __FUNCTION__, "$obj must be string and $sub must be array!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
}

void  jsonGet(EntContext& ec)
{
	if (ec.sub.is_array())
	{
		if (ec.obj.is_number())
			ec.val = ec.sub[ec.obj.get<size_t>()];
		else
			throw json({ { __FUNCTION__, "$obj must be unsigned number!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
	}
	else if (ec.sub.is_object())
	{
		if (ec.obj.is_string())
			ec.val = ec.sub[ec.obj.get_ref<string&>()];
		else
			throw json({ { __FUNCTION__, "$obj must be string!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
	}
	else
		throw json({ { __FUNCTION__, "$sub must be array or object!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
}

void  jsonSet(EntContext& ec)
{
	if (ec.sub.is_array())
	{
		if (ec.obj.is_number_unsigned())
			ec.sub[ec.obj.get<size_t>()] = ec.val;
		else
			throw json({ { __FUNCTION__, "$obj must be unsigned number!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
	}
	else if (ec.sub.is_object())
	{
		if (ec.obj.is_string())
			ec.sub[ec.obj.get_ref<string&>()] = ec.val;
		else
			throw json({ { __FUNCTION__, "$obj must be string!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
	}
	else if (ec.sub.is_null())
	{
		if (ec.obj.is_number_unsigned())
			ec.sub[ec.obj.get<size_t>()] = ec.val;
		else if (ec.obj.is_string())
			ec.sub[ec.obj.get_ref<string&>()] = ec.val;
		else
			throw json({ { __FUNCTION__, "$obj must be unsigned number or string!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
	}
	else
		throw json({ { __FUNCTION__, "$sub must be array, object or null!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
}

void  jsonErase(EntContext& ec)
{
	if (ec.sub.is_object())
	{
		if (ec.obj.is_string())
			ec.sub.erase(ec.obj.get_ref<string&>());
		else
			throw json({ { __FUNCTION__, "$obj must be string!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
	}
	else
		throw json({ { __FUNCTION__, "$sub must be object!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
}

void  jsonIsEq(EntContext& ec)
{
	ec.val = ec.sub == ec.obj;
}

void  jsonIsNotEq(EntContext& ec)
{
	ec.val = ec.sub != ec.obj;
}

void  jsonSum(EntContext& ec)
{
	__int64	isum = 0;
	double	dsum = 0.0;

	if (ec.obj.is_array())
	{
		for (auto& it : ec.obj) switch (it.type())
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

		if (0.0 == dsum)	ec.sub = json::number_integer_t(isum);
		else if (0 == isum)	ec.sub = json::number_float_t(dsum);
		else				ec.sub = json::number_float_t(dsum + json::number_float_t(isum));
		return;
	}
	else
		throw json({ { __FUNCTION__, "$obj must be json array!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
}

void  jsonWhere(EntContext& ec)
{
	ec.val = json::array();		//	подготовка выходного массива
	
	if (ec.obj.is_null()) return;

	if (ec.obj.is_array())
	{
		size_t i = 0;
		for (json&	it : ec.obj)
		{
			try {
				json	boolres;
				JSONExec(EntContext(boolres, it, boolres, ec.ent, ec.ctx, ec.root), ec.sub);
				if (boolres.is_boolean())
					if (boolres.get<bool>())
						ec.val.push_back(it);	//	фильтруем
				i++;
			}
			catch (json& j) { throw json({ { __FUNCTION__, {"/["s + to_string(i) + "]/"s, j }} }); }
		}
	}
	else
		throw json({ { __FUNCTION__, "$obj must be json array!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
}

void  jsonBelow(EntContext& ec)	//	<
{
	if (ec.sub.type() == ec.obj.type()) switch (ec.obj.type())
	{
	case json::value_t::array:
	case json::value_t::object:
		ec.val = json(ec.sub.size() < ec.obj.size());
		return;

	case json::value_t::string:
		ec.val = json(ec.sub.get<string>().size() < ec.obj.get<string>().size());
		return;

	case json::value_t::boolean:
		ec.val = json(int(ec.sub.get<bool>()) < int(ec.obj.get<bool>()));
		return;

	case json::value_t::number_float:
		ec.val = json(ec.sub.get<json::number_float_t>() < ec.obj.get<json::number_float_t>());
		return;

	case json::value_t::number_integer:
		ec.val = json(ec.sub.get<json::number_integer_t>() < ec.obj.get<json::number_integer_t>());
		return;

	case json::value_t::number_unsigned:
		ec.val = json(ec.sub.get<json::number_unsigned_t>() < ec.obj.get<json::number_unsigned_t>());
		return;

	default:
		ec.val = json();
		return;
	}
}

void  jsonAnd(EntContext& ec)
{
	if (ec.sub.is_boolean() && ec.obj.is_boolean())
		ec.val = ec.sub.get<bool>() && ec.obj.get<bool>();
	else
		throw json({ { __FUNCTION__, "$obj and $sub must be boolean!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
}

void  IfObjTrueThenExecSub(EntContext& ec)
{
	if (!ec.obj.is_boolean())
		throw json({ { __FUNCTION__, "$obj must be boolean!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });

	try
	{
		if (ec.obj.get<bool>())
			JSONExec(ec.ctx, ec.sub);
	}
	catch (json& j) { throw json({ { __FUNCTION__, j} }); }
}

void  IfObjFalseThenExecSub(EntContext& ec)
{
	if (!ec.obj.is_boolean())
		throw json({ { __FUNCTION__, "$obj must be boolean!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });

	try
	{
		if (!ec.obj.get<bool>())
			JSONExec(ec.ctx, ec.sub);
	}
	catch (json& j) { throw json({ { __FUNCTION__, j} }); }
}

void  ExecSubWhileObjTrue(EntContext& ec)
{
	while (true)
	{
		if (!ec.obj.is_boolean()) throw json({ { __FUNCTION__, "$obj must be boolean!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });
		if (!ec.obj.get<bool>()) return;
		try
		{
			JSONExec(ec.ctx, ec.sub);
		}
		catch (json& j) { throw json({ { __FUNCTION__, j} }); }
	}
}

void  json_switch_bool(EntContext& ec)
{
	if (ec.obj.is_null())
		return;

	if (!ec.obj.is_boolean())
		throw json({ { __FUNCTION__, "$obj must be boolean!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });

	if (!ec.sub.is_object())
		throw json({ { __FUNCTION__, "$sub must be json object!\n $sub = "s + ec.sub.dump()} });

	try
	{
		if (ec.obj.get<bool>())	JSONExec(ec.ctx, ec.sub["true"]);
		else	JSONExec(ec.ctx, ec.sub["false"]);
	}
	catch (json& j) { throw json({ { __FUNCTION__, j} }); }
	catch (json::exception& e) { throw json({ { __FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)} }); }
	catch (std::exception& e) { throw json({ { __FUNCTION__, "std::exception: "s + e.what()} }); }
	catch (...) { throw json({ { __FUNCTION__, "unknown exception"s} }); }
}

void  json_switch_number(EntContext& ec)
{
	if (ec.obj.is_null())
		return;

	if (!ec.obj.is_number_unsigned())
		throw json({ { __FUNCTION__, "$obj must be unsigned number!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });

	if (!ec.sub.is_array())
		throw json({ { __FUNCTION__, "$sub must be json array!\n $sub = "s + ec.sub.dump()} });

	try
	{
		size_t	id = ec.obj.get<size_t>();
		if (id < ec.sub.size()) JSONExec(ec.ctx, ec.sub[id]);
	}
	catch (json& j) { throw json({ { __FUNCTION__, j} }); }
	catch (json::exception& e) { throw json({ { __FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)} }); }
	catch (std::exception& e) { throw json({ { __FUNCTION__, "std::exception: "s + e.what()} }); }
	catch (...) { throw json({ { __FUNCTION__, "unknown exception"s} }); }
}


void  json_switch_string(EntContext& ec)
{
	if (ec.obj.is_null())
		return;

	if (!ec.obj.is_string())
		throw json({ { __FUNCTION__, "$obj must be string!\n $obj = "s + ec.obj.dump() + "\n $sub = " + ec.sub.dump()} });

	if (!ec.sub.is_object())
		throw json({ { __FUNCTION__, "$sub must be json object!\n $sub = "s + ec.sub.dump()} });

	try
	{
		if(ec.sub.count(ec.obj.get_ref<string&>())) JSONExec(ec.ctx, ec.sub[ec.obj.get_ref<string&>()]);
		else JSONExec(ec.ctx, ec.sub["default"]);
	}
	catch (json& j) { throw json({ { __FUNCTION__, j} }); }
	catch (json::exception& e) { throw json({ { __FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)} }); }
	catch (std::exception& e) { throw json({ { __FUNCTION__, "std::exception: "s + e.what()} }); }
	catch (...) { throw json({ { __FUNCTION__, "unknown exception"s} }); }
}


void  json_throw(EntContext& ec) { throw ec.obj; }


void  json_catch(EntContext& ec)
{
	//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
	//	проецируем во внешнем контексте
	try
	{
		JSONExec(EntContext(ec.val, ec.ctx.obj, ec.ctx.sub, ec.ctx.ent, ec.ctx, ec.root), ec.obj);
	}
	catch (json& j) { ec.sub = j; }
}


#define define_object_method(object_method)		\
void  json_call_##object_method(EntContext& ec)	\
{												\
	ec.sub = ec.obj.##object_method();			\
}

#define define_static_method(static_method)		\
void  json_call_##static_method(EntContext& ec)	\
{												\
	ec.val = json::##static_method();			\
}

define_static_method(array)
define_static_method(meta)
define_static_method(object)

void  json_call_null(EntContext& ec)
{
	ec.val = json();
}

#include <iostream>

void  jsonPrint(EntContext& ec)
{
	cout << ec.obj.dump(1) << endl;
}


void  jsonTAG(EntContext& ec)
{
	if (!ec.sub.is_object())
		throw json({ { __FUNCTION__, "$sub must be json object!\n $sub = "s + ec.sub.dump()} });

	string	tag = ec.sub["<>"];

	if (!ec.val.is_string())
		ec.val = ""s;

	string& body = ec.val.get_ref<string&>();
	body += "<" + tag;

	for (auto& it : ec.sub.items())
		if ("<>" != it.key())
			body += " " + it.key() + "=" + it.value().dump();

	body += ">";
	//	что бы выходной поток xml попадал в тоже значение ec.val нужно исполнять в текущем контексте EV
	json	objview;
	JSONExec(EntContext(objview, ec.obj, ec.sub, ec.ent, ec.ctx, ec.root), ec.obj);
	if (objview.is_string()) body += objview.get_ref<string&>();
	else                     body += objview.dump();
	body += "</" + tag + ">";
}


void  jsonXML(EntContext& ec)
{
	std::string res = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"s;

	if (!ec.sub.is_object())
		throw json({ { __FUNCTION__, "$sub must be json object!\n $sub = "s + ec.sub.dump()} });

	string	tag = ec.sub["<>"];

	if (!ec.val.is_string())
		ec.val = res;
	else if(!ec.val.get_ref<string&>().size())
		ec.val = res;

	string& body = ec.val.get_ref<string&>();
	body += "<" + tag;

	for (auto& it : ec.sub.items())
		if ("<>" != it.key())
			body += " " + it.key() + "=" + it.value().dump();

	body += ">";
	//	что бы выходной поток xml попадал в тоже значение ec.val нужно исполнять в текущем контексте EV
	json	objview;
	JSONExec(EntContext(objview, ec.obj, ec.sub, ec.ent, ec.ctx, ec.root), ec.obj);
	if (objview.is_string()) body += objview.get_ref<string&>();
	else                     body += objview.dump();
	body += "</" + tag + ">";
}


void  jsonHTML(EntContext& ec)
{
	std::string res = "<!DOCTYPE html>"s;

	if (!ec.sub.is_object())
		throw json({ { __FUNCTION__, "$sub must be json object!\n $sub = "s + ec.sub.dump()} });

	string	tag = ec.sub["<>"];

	if (!ec.val.is_string())
		ec.val = res;
	else if (!ec.val.get_ref<string&>().size())
		ec.val = res;

	string& body = ec.val.get_ref<string&>();
	body += "<" + tag;

	for (auto& it : ec.sub.items())
		if ("<>" != it.key())
			body += " " + it.key() + "=" + it.value().dump();

	body += ">";
	//	что бы выходной поток xml попадал в тоже значение ec.val нужно исполнять в текущем контексте EV
	json	objview;
	JSONExec(EntContext(objview, ec.obj, ec.sub, ec.ent, ec.ctx, ec.root), ec.obj);
	if (objview.is_string()) body += objview.get_ref<string&>();
	else                     body += objview.dump();
	body += "</" + tag + ">";
}


void  jsonDump(EntContext& ec)
{
	ec.sub = ec.obj.dump(3);
}


__int64 GetClock(void)
{
	__int64 t;
	__asm rdtsc;
	__asm lea ecx, t;
	__asm mov[ecx], eax;
	__asm mov[ecx + 4], edx;
	return t;
}


__int64 StartTime = 0;

void  jsonStartClock(EntContext& ec)
{
	StartTime = GetClock();
}

void  jsonStopClock(EntContext& ec)
{
	__int64 Time = GetClock() - StartTime;
	cout << "Time left = "s << Time/__int64(1000) << " Ktics" << endl;
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

	//	json
#define map_json_static_method(static_method)	Addx86Entity(Ent["json"], #static_method, json_call_##static_method, ""s );
	map_json_static_method(array);
	map_json_static_method(null);
	map_json_static_method(meta);
	map_json_static_method(object);
	Addx86Entity(Ent["json"], "dump"s, jsonDump, ""s);

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
	Addx86Entity(Ent["string"], "="s, string_string, ""s);
	Addx86Entity(Ent["string"], "+="s, string_add, ""s);
	Addx86Entity(Ent["string"], "find"s, string_find, ""s);
	Addx86Entity(Ent["string"], "split"s, string_split, ""s);
	Addx86Entity(Ent["string"], "join"s, string_join, ""s);

	//	control
	Addx86Entity(Ent, "foreachobj"s, jsonForEachObject, ""s);
	Addx86Entity(Ent, "foreachsub"s, jsonForEachSubject, ""s);
	Addx86Entity(Ent, "then"s, IfObjTrueThenExecSub, ""s);
	Addx86Entity(Ent, "else"s, IfObjFalseThenExecSub, "");
	Addx86Entity(Ent, "while"s, ExecSubWhileObjTrue, ""s);
	Addx86Entity(Ent["switch"], "bool"s, json_switch_bool, ""s);
	Addx86Entity(Ent["switch"], "number"s, json_switch_number, ""s);
	Addx86Entity(Ent["switch"], "string"s, json_switch_string, ""s);
	Addx86Entity(Ent, "throw"s, json_throw, "");
	Addx86Entity(Ent, "catch"s, json_catch, "");

	//	display
	Addx86Entity(Ent, "print"s, jsonPrint, ""s);

	//	browser
	Addx86Entity(Ent, "tag"s, jsonTAG, ""s);
	Addx86Entity(Ent, "xml"s, jsonXML, ""s);
	Addx86Entity(Ent, "html"s, jsonHTML, ""s);

	//	debug
	Addx86Entity(Ent["debug"]["clock"], "start"s, jsonStartClock, ""s);
	Addx86Entity(Ent["debug"]["clock"], "stop"s, jsonStopClock, ""s);
}
