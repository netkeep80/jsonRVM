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

#ifdef CSPush
#undef CSPush
#endif

#define	CSPush(name)	PushCS LevelName(name, EV.CallStack, EV.ctx_level);

//	функция получает проекцию объекта и сохраняет её в субъекте
//	Необходимо проверить нет ли закэшированного значения в "="
//	и если его нет то вычислить и вернуть.
void	_jsonView(EntView &EV, const jsonPtr& ResPtr)
{	//	получаем ссылку на субъект, что бы знать куда записывать проекцию объекта
	//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
	jsonPtr	subview = EV["->"];
	*ResPtr = subview;
	EV.GetObjView(subview);
	//EV.jsonView("<-"s, EV["->"], ResPtr);
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

void	jsonXOR(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);
	*ResPtr = (*subview) ^ (*objview);
}

////////////////////////////////////////////////////////
json operator * (const json& a, const json& b);

__forceinline json mul_number_float(const json& a, const json& b) { return type_operation<double, _mul<double>>(a, b); }
__forceinline json mul_number_integer(const json& a, const json& b) { return type_operation<int64_t, _mul<int64_t>>(a, b); }
__forceinline json mul_number_unsigned(const json& a, const json& b) { return type_operation<uint64_t, _mul<uint64_t>>(a, b); }
__forceinline json mul_object(const json& a, const json& b) { return object_operation<_mul<json>>(a, b); }
__forceinline json mul_array(const json& a, const json& b) { return array_operation<_mul<json>>(a, b); }

__forceinline json array_mul_value(const json& a, const json& b)
{
	json ar = a;
	for (auto& it : ar) it = it * b;
	return ar;
}

json operator * (const json& sub, const json& obj)
{
	if (sub.type() == obj.type()) switch (sub.type())
	{
		SUB_CASE(mul, array, sub, obj);
		SUB_CASE(mul, number_float, sub, obj);
		SUB_CASE(mul, number_integer, sub, obj);
		SUB_CASE(mul, number_unsigned, sub, obj);
		SUB_CASE(mul, object, sub, obj);
	default: return json();
	}
	else if (sub.is_array()) return array_mul_value(sub, obj);
	else if (obj.is_array()) return array_mul_value(obj, sub);
	else return json();
}

void	jsonMul(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);
	*ResPtr = (*subview) * (*objview);
}

////////////////////////////////////////////////////////////////////////////////////////////
json operator / (const json& a, const json& b);

__forceinline json div_number_float(const json& a, const json& b) { return type_operation<double, _div<double>>(a, b); }
__forceinline json div_number_integer(const json& a, const json& b) { return type_operation<int64_t, _div<int64_t>>(a, b); }
__forceinline json div_number_unsigned(const json& a, const json& b) { return type_operation<uint64_t, _div<uint64_t>>(a, b); }
__forceinline json div_object(const json& a, const json& b) { return object_operation<_div<json>>(a, b); }
__forceinline json div_array(const json& a, const json& b) { return array_operation<_div<json>>(a, b); }

__forceinline json array_div_value(const json& a, const json& b)
{
	json ar = a;
	for (auto& it : ar) it = it / b;
	return ar;
}

__forceinline json value_div_array(const json& a, const json& b)
{
	json ar = b;
	for (auto& it : ar) it = a / it;
	return ar;
}

json operator / (const json& sub, const json& obj)
{
	if (sub.type() == obj.type()) switch (sub.type())
	{
		SUB_CASE(div, array, sub, obj);
		SUB_CASE(div, number_float, sub, obj);
		SUB_CASE(div, number_integer, sub, obj);
		SUB_CASE(div, number_unsigned, sub, obj);
		SUB_CASE(div, object, sub, obj);
	default: return json();
	}
	else if (sub.is_array()) return array_div_value(sub, obj);
	else if (obj.is_array()) return value_div_array(sub, obj);
	else return json();
}

void	jsonDiv(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);
	*ResPtr = (*subview) / (*objview);
}

////////////////////////////////////////////////////////////////////////////////////////////
json operator + (const json& a, const json& b);

__forceinline json add_number_float(const json& a, const json& b) { return type_operation<double, _add<double>>(a, b); }
__forceinline json add_number_integer(const json& a, const json& b) { return type_operation<int64_t, _add<int64_t>>(a, b); }
__forceinline json add_number_unsigned(const json& a, const json& b) { return type_operation<uint64_t, _add<uint64_t>>(a, b); }
__forceinline json add_object(const json& a, const json& b) { return object_operation<_add<json>>(a, b); }
__forceinline json add_array(const json& a, const json& b) { return array_operation<_add<json>>(a, b); }

__forceinline json array_add_value(const json& a, const json& b)
{
	json ar = a;
	for (auto& it : ar) it = it + b;
	return ar;
}

json operator + (const json& sub, const json& obj)
{
	if (sub.type() == obj.type()) switch (sub.type())
	{
		SUB_CASE(add, array, sub, obj);
		SUB_CASE(add, number_float, sub, obj);
		SUB_CASE(add, number_integer, sub, obj);
		SUB_CASE(add, number_unsigned, sub, obj);
		SUB_CASE(add, object, sub, obj);
	default: return json();
	}
	else if (sub.is_array()) return array_add_value(sub, obj);
	else if (obj.is_array()) return array_add_value(obj, sub);
	else return json();
}

void	jsonAdd(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);
	*ResPtr = (*subview) + (*objview);
}

////////////////////////////////////////////////////////////////////////////////////////////
json operator - (const json& a, const json& b);

__forceinline json subtract_number_float(const json& a, const json& b) { return type_operation<double, _sub<double>>(a, b); }
__forceinline json subtract_number_integer(const json& a, const json& b) { return type_operation<int64_t, _sub<int64_t>>(a, b); }
__forceinline json subtract_number_unsigned(const json& a, const json& b) { return type_operation<uint64_t, _sub<uint64_t>>(a, b); }
__forceinline json subtract_object(const json& a, const json& b) { return object_operation<_sub<json>>(a, b); }
__forceinline json subtract_array(const json& a, const json& b) { return array_operation<_sub<json>>(a, b); }

__forceinline json array_subtract_value(const json& a, const json& b)
{
	json ar = a;
	for (auto& it : ar) it = it - b;
	return ar;
}

json operator - (const json& sub, const json& obj)
{
	if (sub.type() == obj.type()) switch (sub.type())
	{
		SUB_CASE(subtract, array, sub, obj);
		SUB_CASE(subtract, number_float, sub, obj);
		SUB_CASE(subtract, number_integer, sub, obj);
		SUB_CASE(subtract, number_unsigned, sub, obj);
		SUB_CASE(subtract, object, sub, obj);
	default: return json();
	}
	else if (sub.is_array()) return array_subtract_value(sub, obj);
	else if (obj.is_array()) return array_subtract_value(obj, sub);
	else return json();
}

void	jsonSubtract(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);
	*ResPtr = (*subview) - (*objview);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void	jsonPower(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);

	if (subview->is_number() && objview->is_number())
	{
		double db = objview->is_number_float() ? objview->get<double>() : double(objview->get<int64_t>());

		if (!subview->is_number_float())
		{	//	result must be integer
			double da = double(subview->get<int64_t>());
			*ResPtr = json(__int64(pow(da, db) + .5));
		}
		else
		{	//	result must be double
			double da = subview->get<double>();
			*ResPtr = json(pow(da, db));
		}
	}
	else
		*ResPtr = "power: error, both arguments must be numbers!"s;
}


void	jsonSqrt(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr objview; EV.GetObjView(objview);
	*ResPtr = EV["->"];

	if (objview->is_number()) *ResPtr = json(sqrt(objview->get<double>()));
	else *ResPtr = json("sqrt: error, objview must be number!");
}


////////////////////////////////////////////////////////////////////////////////////

void	jsonForEachElement(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr objview; EV.GetObjView(objview);

	if (objview->is_array())
	{
		CSPush("<-/="s);
		*ResPtr = *objview;
		json*	subview = EV["->"];
		//ResPtr = EV["->/="] = &((*EV[""])["->/="] = json::array());
		for (size_t i = 0; i < ResPtr->size(); i++)
		{
			//EntView	ctx(EV.parent, *EV[""], ResPtr);// , *EV[""], *EV["->"], EV.obj);
											//ctx.subview = &(*subview)[i];
											//ctx.objview = &(*objview)[i];
			CSPush("["s + to_string(i) + "]"s);
			EV["="] = EV["<-"] = EV["->"] = &(*ResPtr)[i];
			EV.jsonExec(*subview, jsonPtr(&(*ResPtr)[i]));
		}

		EV["="] = ResPtr;
	}
	/*if (objview->is_object())
	{
		CSPush("<-"s);
		*ResPtr = json::array();
		size_t i = 0;
		for (auto& it : objview->items())
		{
			EntView	ctx(EV.parent, *EV[""], ResPtr);// , *EV[""], *EV["->"], EV.obj);
											//ctx.subview = &(*subview)[i];
											//ctx.objview = &it.second;
			CSPush(it.key());
			ctx.jsonExec(*EV["->"], jsonPtr(&(*ResPtr)[i]));
			i++;
		}
	}*/
}


void	jsonSize(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);
	*ResPtr = &(*subview = json(objview->size()));
}

#define define_json_is_type(json_type)								\
void	json_is_##json_type(EntView &EV, const jsonPtr& ResPtr)			\
{																	\
	jsonPtr subview; EV.GetSubView(subview);			\
	jsonPtr objview; EV.GetObjView(objview);			\
	*ResPtr = &(*subview = json(objview->is_##json_type()));			\
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


void	jsonIntSeq(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);

	if (objview->is_number_integer() && subview->is_number_integer())
	{
		*ResPtr = json::array();
		int i = 0;
		int from = objview->get<int>();
		int to = subview->get<int>();

		if (from < to)
		{
			for (; from <= to; from++)
				(*ResPtr)[i++] = json(from);
		}
		else
		{
			for (; from >= to; from--)
				(*ResPtr)[i++] = json(from);
		}
	}
	else
	{
		*ResPtr = json("int_seq: error, both arguments must be integer!");
	}

	*ResPtr;
}


void	jsonUnion(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);
	*ResPtr = subview;
	for (auto& it : *objview)
		(*ResPtr).push_back(it);
}


void	jsonNull(EntView &EV, const jsonPtr& ResPtr) {}


void	jsonInt32(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview = EV["->"];
	jsonPtr objview; EV.GetObjView(objview);

	switch (objview->type())
	{
	case json::value_t::array:
	case json::value_t::object:
		*subview = objview->size();
		break;

	case json::value_t::string:
		*subview = atoi(objview->get<string>().c_str());
		break;

	case json::value_t::boolean:
		if (objview->get<bool>())
			*subview = int(1);
		else
			*subview = int(0);
		break;

	case json::value_t::number_float:
	case json::value_t::number_integer:
	case json::value_t::number_unsigned:
		*subview = objview->get<int>();
		break;

	default:
		*subview = int(0);
	}

	*ResPtr = subview;
}


void	jsonDouble(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview = EV["->"];
	jsonPtr objview; EV.GetObjView(objview);

	switch (objview->type())
	{
	case json::value_t::array:
	case json::value_t::object:
		*subview = double(objview->size());
		break;

	case json::value_t::string:
		*subview = double(atof(objview->get<string>().c_str()));
		break;

	case json::value_t::boolean:
		if (objview->get<bool>())
			*subview = double(1);
		else
			*subview = double(0);
		break;

	case json::value_t::number_float:
	case json::value_t::number_integer:
	case json::value_t::number_unsigned:
		*subview = double(objview->get<double>());
		break;

	default:
		*subview = double(0);
	}

	*ResPtr = subview;
}


void	jsonSplitstring(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);

	if (objview->is_string() && subview->is_string())
	{
		const string& str = objview->get<string>();
		const string& delim = subview->get<string>();
		size_t prev = 0, pos = 0, i = 0;
		*ResPtr = json::array();

		do
		{
			pos = str.find(delim, prev);
			if (pos == string::npos) pos = str.length();
			string token = str.substr(prev, pos - prev);
			if (token.empty())
				(*ResPtr)[i++] = json();
			else
				(*ResPtr)[i++] = json(token);
			prev = pos + delim.length();
		} while (pos < str.length() && prev < str.length());
	}
	else
		*ResPtr = json();

}


void	jsonJoinstring(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);
	string	result = "";
	bool	first = true;

	if (objview->is_string() && subview->is_array())
	{
		for (auto& it : *subview)
		{
			if (first) first = false;
			else       result += objview->get<string>();

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

	*ResPtr = json(result);
}


void	jsonAt(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);

	if (subview->is_array())
	{
		if (objview->is_number())
			*ResPtr = &(*subview)[objview->get<size_t>()];
		else if (objview->is_string())
			*ResPtr = &(*subview)[atoi(objview->get<string>().c_str())];
	}
	else if (subview->is_object())
	{
		if (objview->is_number())
			*ResPtr = &(*subview)[to_string(objview->get<uint64_t>())];
		else if (objview->is_string())
			*ResPtr = &(*subview)[objview->get<string>()];
	}
	else if (subview->is_null())
	{
		if (objview->is_number())
		{
			*subview = json::array();
			*ResPtr = &(*subview)[objview->get<size_t>()];
		}
		else if (objview->is_string())
		{
			*subview = json::object();
			*ResPtr = &(*subview)[objview->get<string>()];
		}
	}
	else
		*ResPtr = json();
}


void	jsonEq(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);
	*ResPtr = json(*subview == *objview);
}


void	jsonNotEq(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);
	*ResPtr = json(*subview != *objview);
}


void	jsonSum(EntView &EV, const jsonPtr& ResPtr)
{
	__int64	isum = 0;
	double	dsum = 0.0;
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);

	if (objview->is_array())
	{
		for (auto& it : *objview)
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
		*subview = isum;
	else if (0 == isum)
		*subview = dsum;
	else
		*subview = dsum + double(isum);

	*ResPtr = subview;
}


void	jsonWhere(EntView &EV, const jsonPtr& ResPtr)	//	todo: поменять местами sub и obj
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview = EV["<-/="];

	if (subview->is_array())
	{
		CSPush(".->/"s);
		*ResPtr = json::array();
		for (size_t i = 0; i < subview->size(); i++)
		{	//	исполняем объект
			EntView	ctx(EV.parent, *EV[""], ResPtr);// , *EV["->"], EV.obj, EV.obj);
											//ctx.subview = &objview;	//	результат проверки критерия записываем в проекцию объекта
											//ctx.objview = &(*subview)[i];	//	проекция объекта это элемент из множества проекции субъекта
			CSPush("["s + to_string(i) + "]"s);
			ctx.jsonExec(*EV["<-"], objview);
			if (objview->is_boolean())
			{
				if (objview->get<bool>())
				{
					(*ResPtr)[ResPtr->size()] = (*subview)[i];	//	фильтруем
				}
			}
		}
	}
	else
	{
		*ResPtr = json();
	}

}


void	jsonBelow(EntView &EV, const jsonPtr& ResPtr)	//	<
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);

	if (subview->type() == objview->type()) switch (objview->type())
	{
	case json::value_t::array:
	case json::value_t::object:
		*ResPtr = json(subview->size() < objview->size());
		return;

	case json::value_t::string:
		*ResPtr = json(subview->get<string>().size() < objview->get<string>().size());

	case json::value_t::boolean:
		*ResPtr = json(int(subview->get<bool>()) < int(objview->get<bool>()));

	case json::value_t::number_float:
		*ResPtr = json(subview->get<double>() < objview->get<double>());

	case json::value_t::number_integer:
		*ResPtr = json(subview->get<int64_t>() < objview->get<int64_t>());

	case json::value_t::number_unsigned:
		*ResPtr = json(subview->get<uint64_t>() < objview->get<uint64_t>());

	default:
		*ResPtr = json();
	}
}


void	jsonAnd(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);
	jsonPtr objview; EV.GetObjView(objview);

	if (subview->is_boolean() && objview->is_boolean())
	{
		*ResPtr = subview->get<bool>() && objview->get<bool>();
	}

	*ResPtr = false;
}


void	jsonThen(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);

	if (subview->is_boolean())
	{
		bool subval = subview->get<bool>();
		*ResPtr = subval;
		if (subval)
		{
			jsonPtr	objview;
			EV.GetObjView(objview);
			*ResPtr = objview;
		}
	}
	else
		return EV.ErrorMessage("then"s, "Error, <-/= must be boolean!"s, ResPtr);
}


void	jsonElse(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr subview; EV.GetSubView(subview);

	if (subview->is_boolean())
	{
		bool	subval = subview->get<bool>();
		*ResPtr = subval;
		if (!subval)
		{
			jsonPtr	objview;
			EV.GetObjView(objview);
			*ResPtr = objview;
		}
	}
	else
		return EV.ErrorMessage("else"s, "Error, <-/= must be boolean!"s, ResPtr);
}


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
			printf("Unloading DLL: %s\n", dll.first.c_str());
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
			printf("Loading DLL: %s\n", LibName.c_str());
			it[LibName].handle = LoadLibrary(utf8_to_wstring(LibName).c_str());

			if (it[LibName].handle)
				(FARPROC &)it[LibName].Init = GetProcAddress(it[LibName].handle, IMPORT_RELATIONS_MODEL);
			else
				it[LibName].Init = nullptr;
		}

		return (it[LibName].Init != nullptr);
	}
} LoadedDLLs;


void	jsonLoadDLL(EntView &EV, const jsonPtr& ResPtr)
{
	jsonPtr objview; EV.GetObjView(objview);	//	вычисляем имя бибилиотеки
	jsonPtr	subview = EV["->"];		//	определяем сущность для размещения словаря

	if (!subview->is_object()) subview = json::object();

	if (objview->is_object())
	{
		if (objview->count("FileInfo"))
		{
			json &FileInfoVal = (*objview)["FileInfo"];

			if (FileInfoVal.count("PathFolder") && FileInfoVal.count("FileName"))
			{
				string	FullFileName = FileInfoVal["PathFolder"].get<string>() + FileInfoVal["FileName"].get<string>();

				if (LoadedDLLs.LoadDict(FullFileName))
				{
					LoadedDLLs[FullFileName].Init(*subview);
					*ResPtr = true;
				}
				else EV.ErrorMessage("LoadDLL"s, "error while loading '" + FullFileName + "' dictionary"s, ResPtr);
				return;
			}
		}
	}

	EV.ErrorMessage("/RVM/load/dll"s, "ent/<- must be json object with FileInfo { PathFolder, FileNameFormat } property"s, ResPtr);
}

////////////////////////////////////////////////////////////////////////////////

void	ImportRelationsModel(json &Ent)
{
	Addx86Entity(Ent, ""s, _jsonView, "jsonView: Executes object in parent ctx and then set subject value"s);
	Addx86Entity(Ent, "view"s, _jsonView, "jsonView: Executes object in parent ctx and then set subject value"s);

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
	Addx86Entity(Ent, "Int32"s, jsonInt32, ""s);
	Addx86Entity(Ent, "int"s, jsonInt32, ""s);
	Addx86Entity(Ent, "double"s, jsonDouble, ""s);
	Addx86Entity(Ent, "*"s, jsonMul, ""s);
	Addx86Entity(Ent, ":"s, jsonDiv, "субъект делитель, объект делимое"s);
	Addx86Entity(Ent, "+", jsonAdd, ""s);
	Addx86Entity(Ent, "-", jsonSubtract, ""s);
	Addx86Entity(Ent, "pow"s, jsonPower, ""s);
	Addx86Entity(Ent, "^"s, jsonXOR, ""s);
	Addx86Entity(Ent, "Splitstring"s, jsonSplitstring, ""s);
	Addx86Entity(Ent, "sqrt"s, jsonSqrt, ""s);
	Addx86Entity(Ent, "foreach"s, jsonForEachElement, ""s);
	Addx86Entity(Ent, "int_seq"s, jsonIntSeq, ""s);
	Addx86Entity(Ent, "size"s, jsonSize, ""s);
	Addx86Entity(Ent, "Joinstring"s, jsonJoinstring, ""s);
	Addx86Entity(Ent, "at"s, jsonAt, ""s);
	Addx86Entity(Ent, "."s, jsonAt, ""s);
	Addx86Entity(Ent, "=="s, jsonEq, ""s);
	Addx86Entity(Ent, "!="s, jsonNotEq, ""s);
	Addx86Entity(Ent, "sum"s, jsonSum, ""s);
	Addx86Entity(Ent, "Where"s, jsonWhere, ""s);
	Addx86Entity(Ent, "<"s, jsonBelow, ""s);
	Addx86Entity(Ent, "And"s, jsonAnd, ""s);
	Addx86Entity(Ent, "&&"s, jsonAnd, ""s);
	Addx86Entity(Ent, "then"s, jsonThen, ""s);
	Addx86Entity(Ent, "else"s, jsonElse, "");
	Addx86Entity(Ent["RVM"]["load"], "dll"s, jsonLoadDLL, "загружает словарь сущностей из dll");
}
