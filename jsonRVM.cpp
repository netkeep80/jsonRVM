/*
MIT License

Copyright © 2016 Vertushkin Roman Pavlovich

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

jsonRVM - json Relations (Model) Virtual Machine
https://github.com/netkeep80/jsonRVM
*/
#include "JsonRVM.h"
#include "windows.h"

#ifdef CSPush
#undef CSPush
#endif

#define	CSPush(name)	PushCS LevelName(name, EV.CallStack, EV.ctx_level);

void	jsonClone(Entity &EV, json &Result)
{	//	детальное клонирование json значения
	*EV["->"] = *EV["<-"];
}

void	jsonView(Entity &EV, json &Result)
{	//	получаем ссылку на субъект, что бы знать куда записывать проекцию объекта
	//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
	EV.parent.ViewEntity(*EV["<-"], *EV["->"]);	//	проецируем во внешнем контексте
}

void	jsonExec(Entity &EV, json &Result)
{	//	получаем ссылку на субъект, что бы знать где исполнять проекцию объекта
	//	контекст EV относится к сущности внутри которой идёт исплнение
	EV.parent.ExecEntity(*EV["<-"], *EV["->"]);	//	исполняем во внешнем контексте
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

#ifdef CSPush
#undef CSPush
#endif

#define	CSPush(name)	PushCS LevelName(name, EV.CallStack, EV.ctx_level);

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

__forceinline json xor_number_float(const json& a, const json& b) { return diff_operation<double, _sub<double>>(a, b); }
__forceinline json xor_number_integer(const json& a, const json& b) { return diff_operation<int64_t, _sub<int64_t>>(a, b); }
__forceinline json xor_number_unsigned(const json& a, const json& b) { return diff_operation<uint64_t, _sub<uint64_t>>(a, b); }

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

void	jsonXOR(Entity &EV, json &Result)
{
	json& subview = *EV["->"];
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);
	Result = subview ^ objview;
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
	{ Result.get_ref<stype&>() operation stype##( objview.get<otype>() ); return; }

#define OPP_ANYTO(operation,type,type_id)						\
	OPP_STO(operation, jf, number_float,    type, type_id)		\
	OPP_STO(operation, ji, number_integer,  type, type_id)		\
	OPP_STO(operation, ju, number_unsigned, type, type_id)

#define VM_OPP(operation)										\
	OPP_ANYTO(operation, jf, number_float)						\
	OPP_ANYTO(operation, ji, number_integer)					\
	OPP_ANYTO(operation, ju, number_unsigned)

#define	OP_BODY( name, operation )								\
void	json##name (Entity &EV, json &Result)					\
{																\
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);		\
	EV.parent.ViewEntity(*EV["->"], Result);					\
	switch( (uint8_t(Result.type()) << sub_field) | uint8_t(objview.type()) )	\
	{ VM_OPP( operation ) default: Result = json(); }			\
}

OP_BODY(Add, +=);
OP_BODY(Substract, -=);
OP_BODY(Mul, *=);
OP_BODY(Div, /=);

//////////////////////////////////////////////////////////////////////////////////////////////
void	jsonPower(Entity &EV, json &Result)
{
	json& subview = *EV["->"];
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);

	if (subview.is_number() && objview.is_number())
	{
		double db = objview.is_number_float() ? objview.get<double>() : double(objview.get<int64_t>());

		if (!subview.is_number_float())
		{	//	result must be integer
			double da = double(subview.get<int64_t>());
			Result = json(__int64(pow(da, db) + .5));
		}
		else
		{	//	result must be double
			double da = subview.get<double>();
			Result = json(pow(da, db));
		}
	}
	else
		Result = "power: error, both arguments must be numbers!"s;
}

void	jsonSqrt(Entity &EV, json &Result)
{
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);
	json& subview = *EV["->"];

	if (objview.is_number()) subview = json(sqrt(objview.get<double>()));
	else Result = json("sqrt: error, objview must be number!");
}

////////////////////////////////////////////////////////////////////////////////////
/*
  sub  |  obj  |                action
------------------------------------------------------------------------------------
 null  | null  | foreach val in Result: val = Result(val)
 null  | objv  | foreach val in objv:   val = Result(val)
 subv  | null  | foreach val in Result: val = subv(val)
 subv  | objv  | foreach val in objv:   val = subv(val)
*/
void	jsonForEach(Entity &EV, json &Result)
{
	json subview = *EV["->"];
	EV.parent.ViewEntity(*EV["<-"], Result);

	if (Result.is_array())
	{
		for (size_t i = 0; i < Result.size(); i++)
		{
			CSPush("["s + to_string(i) + "]"s);
			EV["<-"] = EV["->"] = &Result[i];
			EV.ExecEntity(subview, Result[i]);
		}
	}
	else if (Result.is_object())
	{
		for (auto& it : Result.items())
		{
			CSPush(it.key());
			EV["<-"] = EV["->"] = &it.value();
			EV.ExecEntity(subview, it.value());
		}
	}
}

void	jsonSize(Entity &EV, json &Result)
{
	json& subview = *EV["->"];
	EV.parent.ViewEntity(*EV["<-"], Result);
	subview = Result.size();
}

#define define_json_is_type(json_type)						\
void	json_is_##json_type(Entity &EV, json &Result)		\
{															\
	json& subview = *EV["->"];								\
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);	\
	subview = objview.is_##json_type();						\
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

void	jsonIntegerSequence(Entity &EV, json &Result)
{
	json& subview = *EV["->"];
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);

	if (objview.is_object())
	{
		json::number_integer_t	from = objview["from"];
		json::number_integer_t	to = objview["to"];
		json::number_integer_t	step = objview.count("step") ? objview["step"] : 1;
		subview = json::array();

		for (json::number_integer_t i = from; from <= to; from += step)
			subview.push_back(i);
	}
	else
	{
		subview = json::array();
		throw("<-/ must has 'from', 'to' and 'step' properties, but <-/ = '"s + objview.dump() + "'"s);
	}
}

void	jsonUnion(Entity &EV, json &Result)
{
	json& subview = *EV["->"];
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);
	Result = subview;
	for (auto& it : objview)
		(Result).push_back(it);
}

void	jsonNull(Entity &EV, json &Result) {}

void	jsonInt32(Entity &EV, json &Result)
{
	json& subview = *EV["->"];
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);

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

	Result = subview;
}

void	jsonDouble(Entity &EV, json &Result)
{
	json& subview = *EV["->"];
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);

	switch (objview.type())
	{
	case json::value_t::array:
	case json::value_t::object:
		subview = double(objview.size());
		break;

	case json::value_t::string:
		subview = double(atof(objview.get<string>().c_str()));
		break;

	case json::value_t::boolean:
		if (objview.get<bool>())
			subview = double(1);
		else
			subview = double(0);
		break;

	case json::value_t::number_float:
	case json::value_t::number_integer:
	case json::value_t::number_unsigned:
		subview = double(objview.get<double>());
		break;

	default:
		subview = double(0);
	}

	Result = subview;
}

void	jsonSplitstring(Entity &EV, json &Result)
{
	json& subview = *EV["->"];
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);

	if (objview.is_string() && subview.is_string())
	{
		const string& str = objview.get<string>();
		const string& delim = subview.get<string>();
		size_t prev = 0, pos = 0, i = 0;
		Result = json::array();

		do
		{
			pos = str.find(delim, prev);
			if (pos == string::npos) pos = str.length();
			string token = str.substr(prev, pos - prev);
			if (token.empty())
				(Result)[i++] = json();
			else
				(Result)[i++] = json(token);
			prev = pos + delim.length();
		} while (pos < str.length() && prev < str.length());
	}
	else
		Result = json();

}

void	jsonJoinstring(Entity &EV, json &Result)
{
	json& subview = *EV["->"];
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);
	string	result = "";
	bool	first = true;

	if (objview.is_string() && subview.is_array())
	{
		for (auto& it : subview)
		{
			if (first) first = false;
			else       result += objview.get<string>();

			switch (it.type())
			{
			case json::value_t::object:
			case json::value_t::array:
				break;	//	сложные типы не печатаем

			case json::value_t::number_float:
			case json::value_t::number_integer:
			case json::value_t::number_unsigned:
				if (it.is_number_float())		result += to_string(it.get<double>());
				else if (it.get<int32_t>())		result += to_string(it.get<int32_t>());
				else if (it.get<uint32_t>())	result += to_string(it.get<uint32_t>());
				else if (it.get<int64_t>())		result += to_string(it.get<int64_t>());
				else if (it.get<uint64_t>())	result += to_string(it.get<uint64_t>());
				break;

			case json::value_t::string:		//	иерархическая символьная ссылка в проекции сущности
				result += it.get<string>();
				break;

			case json::value_t::boolean:	//	фильтрация атрибутов модели по маске, управление видимостью проекций
				result += it.dump();
				break;

			default:	//	null не печатаем
				break;
			}
		}
	}

	Result = json(result);
}

void	jsonAt(Entity &EV, json &Result)
{
	json& subview = *EV["->"];
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);

	if (subview.is_array())
	{
		if (objview.is_number())
			Result = &subview[objview.get<size_t>()];
		else if (objview.is_string())
			Result = &subview[atoi(objview.get<string>().c_str())];
	}
	else if (subview.is_object())
	{
		if (objview.is_number())
			Result = &subview[to_string(objview.get<uint64_t>())];
		else if (objview.is_string())
			Result = &subview[objview.get<string>()];
	}
	else if (subview.is_null())
	{
		if (objview.is_number())
		{
			subview = json::array();
			Result = &subview[objview.get<size_t>()];
		}
		else if (objview.is_string())
		{
			subview = json::object();
			Result = &subview[objview.get<string>()];
		}
	}
	else
		Result = json();
}

void	jsonIsEq(Entity &EV, json &Result)
{
	json& subview = *EV["->"];
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);
	Result = json(subview == objview);
}

void	jsonIsNotEq(Entity &EV, json &Result)
{
	json& subview = *EV["->"];
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);
	Result = json(subview != objview);
}

void	jsonSum(Entity &EV, json &Result)
{
	__int64	isum = 0;
	double	dsum = 0.0;
	json& subview = *EV["->"];
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);

	if (objview.is_array())
	{
		for (auto& it : objview)
		{
			if (it.is_number())
			{
				if (!it.is_number_float())
					isum += it.get<int64_t>();
				else
					dsum += it.get<double>();
			}
		}
	}

	if (0.0 == dsum)
		subview = isum;
	else if (0 == isum)
		subview = dsum;
	else
		subview = dsum + double(isum);

	Result = subview;
}

void	jsonWhere(Entity &EV, json &Result)
{
	json subview = *EV["->"];	//	фильтр для where clause, при исполнении должен возвращать bool
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);
	Result = json::array();

	if (objview.is_array())
	{
		CSPush(".->/"s);
		for (size_t i = 0; i < objview.size(); i++)
		{	//	исполняем объект
			CSPush("["s + to_string(i) + "]"s);
			json	boolres;
			EV[""] = &boolres;
			EV["<-"] = EV["->"] = &objview[i];
			EV.ExecEntity(subview, boolres);

			if (boolres.is_boolean())
				if (boolres.get<bool>())
					Result.push_back(objview[i]);	//	фильтруем
		}
	}
}

void	jsonBelow(Entity &EV, json &Result)	//	<
{
	json& subview = *EV["->"];
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);

	if (subview.type() == objview.type()) switch (objview.type())
	{
	case json::value_t::array:
	case json::value_t::object:
		Result = json(subview.size() < objview.size());
		return;

	case json::value_t::string:
		Result = json(subview.get<string>().size() < objview.get<string>().size());

	case json::value_t::boolean:
		Result = json(int(subview.get<bool>()) < int(objview.get<bool>()));

	case json::value_t::number_float:
		Result = json(subview.get<double>() < objview.get<double>());

	case json::value_t::number_integer:
		Result = json(subview.get<int64_t>() < objview.get<int64_t>());

	case json::value_t::number_unsigned:
		Result = json(subview.get<uint64_t>() < objview.get<uint64_t>());

	default:
		Result = json();
	}
}

void	jsonAnd(Entity &EV, json &Result)
{
	json& subview = *EV["->"];
	json objview; EV.parent.ViewEntity(*EV["<-"], objview);

	if (subview.is_boolean() && objview.is_boolean())
	{
		Result = subview.get<bool>() && objview.get<bool>();
	}

	Result = false;
}

void	IfObjTrueThenExecSub(Entity &EV, json &Result)
{
	json& subview = *EV["->"];
	EV.parent.ViewEntity(*EV["<-"], Result);

	if (!Result.is_boolean())
	{
		Result = false;
		throw("<-/ must be boolean!"s);
	}

	if (Result.get<bool>())
		EV.parent.ExecEntity(subview, Result);
}

void	IfObjFalseThenExecSub(Entity &EV, json &Result)
{
	json& subview = *EV["->"];
	EV.parent.ViewEntity(*EV["<-"], Result);

	if (!Result.is_boolean())
	{
		Result = false;
		throw("<-/ must be boolean!"s);
	}

	if (!Result.get<bool>())
		EV.parent.ExecEntity(subview, Result);
}

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


void	jsonLoadDLL(Entity &EV, json &Result)
{
	json& subview = *EV["->"];//	определяем сущность для размещения словаря
	EV.parent.ViewEntity(*EV["<-"], Result);//	вычисляем имя бибилиотеки

	if (Result.is_object())
	{
		if (Result.count("PathFolder") && Result.count("FileName"))
		{
			string	FullFileName = Result["PathFolder"].get<string>() + Result["FileName"].get<string>();
			if (!subview.is_object()) subview = json::object();
			Result = LoadedDLLs.LoadDict(FullFileName);
			if (Result) LoadedDLLs[FullFileName].Init(subview);
			return;
		}
	}

	Result = false;
	throw("<-/ must be json object with PathFolder, FileName properties"s);
}

////////////////////////////////////////////////////////////////////////////////

void	ImportRelationsModel(json &Ent)
{
	Addx86Entity(Ent["RVM"], "view"s, jsonView, "ViewEntity: View object model in parent ctx and then set subject value"s);
	Addx86Entity(Ent["RVM"], "exec"s, jsonExec, "EcexEntity: Executes object model in parent ctx and then set subject value"s);
	Addx86Entity(Ent["RVM"]["load"], "dll"s, jsonLoadDLL, "загружает словарь сущностей из dll");

#define map_json_is_type(json_type)	Addx86Entity(Ent, "is_"s + #json_type, json_is_##json_type, ""s );
	map_json_is_type(array);
	map_json_is_type(boolean);
	map_json_is_type(number_float);
	map_json_is_type(number_integer);
	map_json_is_type(number_unsigned);
	map_json_is_type(null);
	map_json_is_type(number);
	map_json_is_type(object);
	map_json_is_type(string);
	map_json_is_type(structured);
	map_json_is_type(discarded);

	Addx86Entity(Ent, "union"s, jsonUnion, ""s);
	Addx86Entity(Ent, "Union"s, jsonUnion, ""s);
	Addx86Entity(Ent, "null"s, jsonNull, ""s);
	Addx86Entity(Ent, "integer"s, jsonInt32, ""s);
	Addx86Entity(Ent, "int"s, jsonInt32, ""s);
	Addx86Entity(Ent, "float"s, jsonDouble, ""s);
	Addx86Entity(Ent, "double"s, jsonDouble, ""s);
	Addx86Entity(Ent, "*"s, jsonMul, ""s);
	Addx86Entity(Ent, ":"s, jsonDiv, "субъект делитель, объект делимое"s);
	Addx86Entity(Ent, "+", jsonAdd, ""s);
	Addx86Entity(Ent, "-", jsonSubstract, ""s);
	Addx86Entity(Ent, "pow"s, jsonPower, ""s);
	Addx86Entity(Ent, "^"s, jsonXOR, ""s);
	Addx86Entity(Ent, "Splitstring"s, jsonSplitstring, ""s);
	Addx86Entity(Ent, "sqrt"s, jsonSqrt, ""s);
	Addx86Entity(Ent, "foreach"s, jsonForEach, ""s);
	Addx86Entity(Ent["sequence"], "integer"s, jsonIntegerSequence, ""s);
	Addx86Entity(Ent, "size"s, jsonSize, ""s);
	Addx86Entity(Ent, "Joinstring"s, jsonJoinstring, ""s);
	Addx86Entity(Ent, "at"s, jsonAt, ""s);
	Addx86Entity(Ent, "[]"s, jsonAt, ""s);
	Addx86Entity(Ent, "=="s, jsonIsEq, ""s);
	Addx86Entity(Ent, "!="s, jsonIsNotEq, ""s);
	Addx86Entity(Ent, "sum"s, jsonSum, ""s);
	Addx86Entity(Ent, "Where"s, jsonWhere, ""s);
	Addx86Entity(Ent, "<"s, jsonBelow, ""s);
	Addx86Entity(Ent, "And"s, jsonAnd, ""s);
	Addx86Entity(Ent, "&&"s, jsonAnd, ""s);
	Addx86Entity(Ent, "then"s, IfObjTrueThenExecSub, ""s);
	Addx86Entity(Ent, "else"s, IfObjFalseThenExecSub, "");
	//Addx86Entity(Ent, "="s, jsonCopy, "jsonCopy: Copy object view to subject value"s);
	Addx86Entity(Ent, "clone"s, jsonClone, "clone: clone object model to subject"s);
}
