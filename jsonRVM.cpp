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

//	функция отображает объект в значение проекции субъекта осуществляя подстановку в шаблонную проекцию объекта
//	Необходимо проверить нет ли закэшированного значения в View?
//	и если его нет то вычислить и вернуть.
json&	_jsonView(EntView &EV)
{	//	получаем ссылку на субъекта, что бы знать куда записывать проекцию объекта
	//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
	return *EV["->"] = EV.GetObjView();
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
__forceinline json type_operation(const json& a, const json& b)
{
	return json(_T(_OP(a.get<_T>(), b.get<_T>())));
}

template<typename _T, typename _OP>
__forceinline json diff_operation(const json& a, const json& b)
{
	_OP	operation(a.get<_T>(), b.get<_T>());
	_T&	res = operation;
	if (res == _T()) return json();
	return json(res);
}

template<typename _OP>
__forceinline json object_operation(const json& a, const json& b)
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
__forceinline json array_operation(const json& a, const json& b)
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

json&	jsonXOR(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();
	json&	objview = EV.GetObjView();
	return View = subview ^ objview;
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

json&	jsonMul(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();
	json&	objview = EV.GetObjView();
	return View = subview * objview;
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

json&	jsonDiv(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();//EV["->/"], View);
	json&	objview = EV.GetObjView();//EV["<-/"], View);
	return View = subview / objview;
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

json&	jsonAdd(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();
	json&	objview = EV.GetObjView();
	return View = subview + objview;
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

json&	jsonSubtract(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();
	json&	objview = EV.GetObjView();
	return View = subview - objview;
}

//////////////////////////////////////////////////////////////////////////////////////////////
json&	jsonPower(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();//EV["->/"], View);
	json&	objview = EV.GetObjView();//EV["<-/"], View);

	if (subview.is_number() && objview.is_number())
	{
		double db = objview.is_number_float() ? objview.get<double>() : double(objview.get<int64_t>());

		if (!subview.is_number_float())
		{	//	result must be integer
			double da = double(subview.get<int64_t>());
			return View = json(__int64(pow(da, db) + .5));
		}
		else
		{	//	result must be double
			double da = subview.get<double>();
			return View = json(pow(da, db));
		}
	}
	else
	{
		return View = json("power: error, both arguments must be numbers!");
	}
}


json&	jsonSqrt(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();
	json&	objview = EV.GetObjView();

	if (objview.is_number())
		return subview = json(sqrt(objview.get<double>()));
	else
		return subview = json("sqrt: error, objview must be number!");
}


////////////////////////////////////////////////////////////////////////////////////

json&		jsonForEach(EntView &EV, json& Model, json& View)
{
	json&	subview = *(EV["->/"] = &View);
	json&	objview = EV.GetObjView();//EV["<-/"], View);

	if (objview.is_array())
	{
		CSPush(".<-/"s);
		subview = json::array();
		for (size_t i = 0; i < objview.size(); i++)
		{
			EntView	ctx(EV.parent, *EV[""]);// , *EV[""], *EV["->"], EV.obj);
											//ctx.subview = &subview[i];
											//ctx.objview = &objview[i];
			CSPush("["s + to_string(i) + "]"s)
				json& res = ctx.jsonExec(*EV["->"]);
			subview[i] = res;
		}
	}
	else if (objview.is_object())
	{
		CSPush("<-"s);
		subview = json::array();
		size_t i = 0;
		for (auto& it : objview.items())
		{
			EntView	ctx(EV.parent, *EV[""]);// , *EV[""], *EV["->"], EV.obj);
											//ctx.subview = &subview[i];
											//ctx.objview = &it.second;
			CSPush(it.key());
			subview[i] = ctx.jsonExec(*EV["->"]);
			i++;
		}
	}

	return subview;
}


json&	jsonSize(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();//EV["->/"], View);
	json&	objview = EV.GetObjView();//EV["<-/"], View);
	return subview = json(objview.size());
}

#define define_json_is_type(json_type)								\
json&	json_is_##json_type(EntView &EV, json& Model, json& View)	\
{																	\
	json&	subview = EV.GetSubView();								\
	json&	objview = EV.GetObjView();								\
	return subview = json(objview.is_##json_type());				\
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


json&	jsonIntSeq(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();//EV["->/"], View);
	json&	objview = EV.GetObjView();//EV["<-/"], View);

	if (objview.is_number_integer() && subview.is_number_integer())
	{
		View = json::array();
		int i = 0;
		int from = objview.get<int>();
		int to = subview.get<int>();

		if (from < to)
		{
			for (; from <= to; from++)
				(View)[i++] = json(from);
		}
		else
		{
			for (; from >= to; from--)
				(View)[i++] = json(from);
		}
	}
	else
	{
		View = json("int_seq: error, both arguments must be integer!");
	}

	return View;
}


json&	jsonUnion(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();
	json&	objview = EV.GetObjView();
	json&	result = View;
	result = subview;
	size_t i = result.size();
	for (auto& it : objview)
		result[i++] = it;
	return result;
}


json&		jsonNull(EntView &EV, json& Model, json& View)
{
	//	just do nothing
	return View;
}


json&	jsonInt32(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();//EV["->/"], View);
	json&	objview = EV.GetObjView();//EV["<-/"], View);

	switch (objview.type())
	{
	case json::value_t::array:
	case json::value_t::object:
		return subview = objview.size();

	case json::value_t::string:
		return subview = atoi(objview.get<string>().c_str());

	case json::value_t::boolean:
		if (objview.get<bool>())
			return subview = int(1);
		else
			return subview = int(0);

	case json::value_t::number_float:
	case json::value_t::number_integer:
	case json::value_t::number_unsigned:
		return subview = objview.get<int>();

	default:
		return subview = int(0);
	}
}


json&	jsonDouble(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();//EV["->/"], View);
	json&	objview = EV.GetObjView();//EV["<-/"], View);

	switch (objview.type())
	{
	case json::value_t::array:
	case json::value_t::object:
		return subview = double(objview.size());

	case json::value_t::string:
		return subview = double(atof(objview.get<string>().c_str()));

	case json::value_t::boolean:
		if (objview.get<bool>())
			return subview = double(1);
		else
			return subview = double(0);

	case json::value_t::number_float:
	case json::value_t::number_integer:
	case json::value_t::number_unsigned:
		return subview = double(objview.get<double>());

	default:
		return subview = double(0);
	}
}


json&	jsonSplitstring(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();
	json&	objview = EV.GetObjView();
	json&	result = View;

	if (objview.is_string() && subview.is_string())
	{
		const string& str = objview.get<string>();
		const string& delim = subview.get<string>();
		size_t prev = 0, pos = 0, i = 0;
		result = json::array();

		do
		{
			pos = str.find(delim, prev);
			if (pos == string::npos) pos = str.length();
			string token = str.substr(prev, pos - prev);
			if (token.empty())
				result[i++] = json();
			else
				result[i++] = json(token);
			prev = pos + delim.length();
		} while (pos < str.length() && prev < str.length());
	}
	else
		result = json();

	return result;
}


json&	jsonJoinstring(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();	//	array
	json&	objview = EV.GetObjView();	//	separator string
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

	return View = json(result);
}


json&	jsonAt(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();
	json&	objview = EV.GetObjView();

	if (subview.is_array())
	{
		if (objview.is_number())
			return subview[objview.get<uint32_t>()];
		else if (objview.is_string())
			return subview[atoi(objview.get<string>().c_str())];
	}
	else if (subview.is_object())
	{
		if (objview.is_number())
			return subview[to_string(objview.get<uint32_t>())];
		else if (objview.is_string())
			return subview[objview.get<string>()];
	}
	else if (subview.is_null())
	{
		if (objview.is_number())
		{
			subview = json::array();
			return subview[objview.get<uint32_t>()];
		}
		else if (objview.is_string())
		{
			subview = json::object();
			return subview[objview.get<string>()];
		}
	}

	return View = json();
}


json&	jsonEq(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();//EV["->/"], View);
	json&	objview = EV.GetObjView();//EV["<-/"], View);
	return View = json(subview == objview);
}


json&	jsonNotEq(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();//EV["->/"], View);
	json&	objview = EV.GetObjView();//EV["<-/"], View);
	return View = json(subview != objview);
}


json&	jsonSum(EntView &EV, json& Model, json& View)
{
	__int64	isum = 0;
	double	dsum = 0.0;
	json& subview = EV.GetSubView();//EV["->/"], View);
	json& objview = EV.GetObjView();//EV["<-/"], View);

	if (objview.is_array())
	{
		for (auto& it : objview)
		{
			if (it.is_number())
			{
				if (!it.is_number_float()) isum += it.get<int64_t>();
				else dsum += it.get<double>();
			}
		}
	}

	if (0.0 == dsum)
		subview = json(isum);
	else if (0 == isum)
		subview = json(dsum);
	else
		subview = json(dsum + double(isum));

	return subview;
}


json&	jsonWhere(EntView &EV, json& Model, json& View)	//	todo: поменять местами sub и obj
{
	json& subview = EV.GetSubView();//EV["->/"], View);	//	множество элементов
	json& objview = *EV["<-/"];

	if (subview.is_array())
	{
		CSPush(".->/"s);
		View = json::array();
		for (size_t i = 0; i < subview.size(); i++)
		{	//	исполняем объект
			EntView	ctx(EV.parent, *EV[""]);// , *EV["->"], EV.obj, EV.obj);
											//ctx.subview = &objview;	//	результат проверки критерия записываем в проекцию объекта
											//ctx.objview = &subview[i];	//	проекция объекта это элемент из множества проекции субъекта
			CSPush("["s + to_string(i) + "]"s);
			objview = ctx.jsonExec(*EV["<-"]);
			if (objview.is_boolean())
			{
				if (objview.get<bool>())
				{
					View[View.size()] = subview[i];	//	фильтруем
				}
			}
		}
	}
	else
	{
		View = json();
	}

	return View;
}


json&	jsonBelow(EntView &EV, json& Model, json& View)	//	<
{
	json&	subview = EV.GetSubView();//EV["->/"], View);
	json&	objview = EV.GetObjView();//EV["<-/"], View);

	if (subview.type() == objview.type()) switch (objview.type())
	{
	case json::value_t::array:
	case json::value_t::object:
		return View = json(subview.size() < objview.size());

	case json::value_t::string:
		return View = json(subview.get<string>().size() < objview.get<string>().size());

	case json::value_t::boolean:
		return View = json(int(subview.get<bool>()) < int(objview.get<bool>()));

	case json::value_t::number_float:
		return View = json(subview.get<double>() < objview.get<double>());

	case json::value_t::number_integer:
		return View = json(subview.get<int64_t>() < objview.get<int64_t>());

	case json::value_t::number_unsigned:
		return View = json(subview.get<uint64_t>() < objview.get<uint64_t>());

	default:
		return View = json();
	}

	return View = json();
}


json&	jsonAnd(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();//EV["->/"], View);
	json&	objview = EV.GetObjView();//EV["<-/"], View);

	if (subview.is_boolean() && objview.is_boolean())
	{
		return View = json(subview.get<bool>() && objview.get<bool>());
	}

	return View = json(false);
}


json&	jsonThen(EntView &EV, json& Model, json& View)
{
	json&	subview = EV.GetSubView();//EV["->/"], View);

	if (subview.is_boolean())
	{
		bool	subval = subview.get<bool>();
		View = json(subval);
		if (subval) EV.GetObjView();//EV["<-/"], View);
		return View;
	}

	return View = json(false);
}

json&	jsonElse(EntView &EV)
{
	json&	subview = EV.GetSubView();

	if (subview.is_boolean())
	{
		bool	subval = subview.get<bool>();
		if (!subval) return EV.GetObjView();
		return subview;
	}
	else
		return EV.ErrorMessage("Else"s, "Error, <-/ must be boolean!"s);
}

#include "windows.h"
//	Поддержка загрузки DLL

template<UINT CodePage>
wstring	_to_wstring(const string &data)
{
	int			nLengthW = data.length() + 1;
	wchar_t*	str = new wchar_t[nLengthW];
	memset(str, 0, nLengthW * sizeof(wchar_t));
	MultiByteToWideChar(CodePage, 0, data.c_str(), -1, str, nLengthW);
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
	int	size = data.length();
	if (size)
	{
		char*		str = new char[2 * size + 1];	//	for russian utf8 string case
		memset(str, 0, 2 * size + 1);
		WideCharToMultiByte(CodePage, 0, data.c_str(), -1, str, 2 * size + 1, NULL, NULL);
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
				(FARPROC &)it[LibName].Init = GetProcAddress(it[LibName].handle, FN_INIT_DICT);
			else
				it[LibName].Init = nullptr;
		}

		return (it[LibName].Init != nullptr);
	}
} LoadedDLLs;


json&	jsonLoadDLL(EntView &EV)
{
	json&	objview = EV.GetObjView();	//	вычисляем имя бибилиотеки
	json&	subview = *EV["->"];		//	определяем сущность для размещения словаря

	if (!subview.is_object()) subview = json::object();

	if (objview.is_object())
	{
		if (objview.count("FileInfo"))
		{
			json &FileInfoVal = objview["FileInfo"];

			if (FileInfoVal.count("PathFolder") && FileInfoVal.count("FileName"))
			{
				string	FullFileName = FileInfoVal["PathFolder"].get<string>() + "\\" + FileInfoVal["FileName"].get<string>();

				if (LoadedDLLs.LoadDict(FullFileName))
				{
					LoadedDLLs[FullFileName].Init(subview);
					return subview;
				}
				else
					return EV.ErrorMessage("LoadDLL"s, "error while loading '" + FullFileName + "' dictionary"s);
			}
		}
	}

	return EV.ErrorMessage("LoadDLL"s, "ent/<- must be json object with FileInfo { PathFolder, FileNameFormat } property"s);
}

////////////////////////////////////////////////////////////////////////////////

void	InitRVMDict(json &Ent)
{
	Addx86Entity(Ent, "="s, _jsonView, "Executes object in parent ctx and then set subject value"s);
/*
 #define map_json_is_type(json_type)	Addx86Entity(Ent, "is_"s + L#json_type, json_is_##json_type );
 map_json_is_type(array);
 map_json_is_type(boolean);
 map_json_is_type(double);
 map_json_is_type(integer);
 map_json_is_type(null);
 map_json_is_type(number);
 map_json_is_type(object);
 map_json_is_type(string);

 //Addx86Entity( "funcname", func );
 Addx86Entity(Ent, "union"s, jsonUnion);
 Addx86Entity(Ent, "Union"s, jsonUnion);
 Addx86Entity(Ent, "null"s, jsonNull );
 Addx86Entity(Ent, "Int32"s, jsonInt32 );
 Addx86Entity(Ent, "int"s, jsonInt32 );
 Addx86Entity(Ent, "double"s, jsonDouble );
 Addx86Entity(Ent, "*"s, jsonMul );
 Addx86Entity(Ent, ":"s, jsonDiv );
 Addx86Entity(Ent, "+", jsonAdd);
 Addx86Entity(Ent, "-", jsonSubtract );
 Addx86Entity(Ent, "pow"s, jsonPower );
 Addx86Entity(Ent, "^"s, jsonXOR );
 Addx86Entity(Ent, "Splitstring"s, jsonSplitstring );
 Addx86Entity(Ent, "sqrt"s, jsonSqrt );
 Addx86Entity(Ent, "foreach"s, jsonForEach );
 Addx86Entity(Ent, "int_seq"s, jsonIntSeq );
 Addx86Entity(Ent, "size"s, jsonSize );
 Addx86Entity(Ent, "Joinstring"s, jsonJoinstring );
 Addx86Entity(Ent, "at"s, jsonAt );
 Addx86Entity(Ent, "."s, jsonAt );
 Addx86Entity(Ent, "=="s, jsonEq );
 Addx86Entity(Ent, "!="s, jsonNotEq );
 Addx86Entity(Ent, "sum"s, jsonSum );
 Addx86Entity(Ent, "Where"s, jsonWhere );
 Addx86Entity(Ent, "<"s, jsonBelow );
 Addx86Entity(Ent, "And"s, jsonAnd );
 Addx86Entity(Ent, "&&"s, jsonAnd );
 Addx86Entity(Ent, "then"s, jsonThen);*/
	Addx86Entity(Ent, "else"s, jsonElse, "");
	Addx86Entity(Ent, "LoadDLL"s, jsonLoadDLL, "загружает словарь сущностей из dll");
}
