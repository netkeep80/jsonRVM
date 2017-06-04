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

jsonRVM - json Relation (Model) Virtual Machine
https://github.com/netkeep80/jsonRVM
*/
#include "BaseVoc.h"

#define SUB_CASE(op,type,op1,op2) case value::value_type::##type: return op##_##type(op1,op2);

value operator ^ (const value& a, const value& b);

__forceinline value xor_Array(const value& a, const value& b)
{
	value ar = value::array();
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
	else return value();
}


__forceinline value xor_Boolean(const value& a, const value& b)
{
	if (a.as_bool() != b.as_bool())	return value::array(vector<value>{ {a, b} });
	else return value();
}


__forceinline value xor_Number(const value& a, const value& b)
{
	value res;

	if (a.is_integer())
	{
		if (b.is_integer())
		{
			res = value(a.as_number().to_int64() - b.as_number().to_int64());
			if (res.as_number().to_int64() == 0) return value();
		}
		else
		{
			res = value(a.as_number().to_int64() - int64_t(b.as_double() + 0.5));
			if (res.as_number().to_int64() == 0) return value();
		}
	}
	else
	{
		if (b.is_integer())
		{
			res = value(int64_t(a.as_double() + 0.5) - b.as_number().to_int64());
			if (res.as_number().to_int64() == 0) return value();
		}
		else
		{
			res = value(float(a.as_double()) - float(b.as_double()));
			if (float(res.as_double()) == 0.0) return value();
		}
	}

	return res;
}


__forceinline value xor_Object(const value& a, const value& b)
{
	value obj = value(), field;

	for (auto it = a.as_object().cbegin(); it != a.as_object().cend(); it++)
	{
		const string_t &key = it->first;
		const value &val = it->second;
		if (b.as_object().find(key) != b.as_object().cend())
		{
			field = val ^ b.at(key);
			if (!field.is_null()) obj[key] = field;
		}
		else obj[key] = val;
	}

	for (auto it = b.as_object().cbegin(); it != b.as_object().cend(); it++)
	{
		const string_t &key = it->first;
		const value &val = it->second;
		if (a.as_object().find(key) != a.as_object().cend())
		{
			field = a.at(key) ^ val;
			if (!field.is_null()) obj[key] = field;
		}
		else obj[key] = val;
	}

	return obj;
}


__forceinline value xor_String(const value& a, const value& b)
{
	if (a.as_string() == b.as_string()) return value();
	else return value::array(vector<value>{ {a, b} });
}


__forceinline value value_xor_Array(const value& a, const value& b)
{
	value ar = value::array();
	bool	not_is_null = false;

	for (size_t i = 0; i < b.size(); i++)
	{
		ar[i] = a ^ b.at(i);
		not_is_null = not_is_null || !ar[i].is_null();
	}

	if (not_is_null) return ar;
	else return value();
}


__forceinline value Array_xor_value(const value& a, const value& b)
{
	value ar = value::array();
	bool	not_is_null = false;

	for (size_t i = 0; i < a.size(); i++)
	{
		ar[i] = a.at(i) ^ b;
		not_is_null = not_is_null || !ar[i].is_null();
	}

	if (not_is_null) return ar;
	else return value();
}


value operator ^ (const value& a, const value& b)
{
	if (a.type() == b.type()) switch (a.type())
	{
		SUB_CASE(xor, Array, a, b);
		SUB_CASE(xor, Boolean, a, b);
		SUB_CASE(xor, Number, a, b);
		SUB_CASE(xor, Object, a, b);
		SUB_CASE(xor, String, a, b);
	default: return value();
	}
	else if (a.is_array()) return Array_xor_value(a, b);
	else if (b.is_array()) return value_xor_Array(a, b);
	else return value::array(vector<value>{ {a, b} });
}


value&	jsonXOR(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);
	return *EV.jsonView = subview ^ objview;
}

////////////////////////////////////////////////////////
value operator * (const value& a, const value& b);

__forceinline value mul_Array(const value& a, const value& b)
{
	value ar = value::array();
	bool	not_is_null = false;

	for (size_t i = 0; (i < a.size()) && (i < b.size()); i++)
		ar[i] = a.at(i) * b.at(i);

	return ar;
}


__forceinline value mul_Number(const value& a, const value& b)
{
	if (a.is_integer())
	{
		if (b.is_integer()) return value(a.as_number().to_int64() * b.as_number().to_int64());
		else                return value(a.as_number().to_double() * b.as_double());
	}
	else return value(a.as_number().to_double() * b.as_number().to_double());
}


__forceinline value mul_Object(const value& a, const value& b)
{
	value obj = value();

	for (auto it = a.as_object().cbegin(); it != a.as_object().cend(); it++)
	{
		if (b.has_field(it->first))	obj[it->first] = it->second * b.at(it->first);
	}

	return obj;
}


__forceinline value Array_mul_value(const value& a, const value& b)
{
	value ar = a;
	for (auto& it :ar.as_array()) it = it * b;
	return ar;
}


value operator * (const value& sub, const value& obj)
{
	if (sub.type() == obj.type()) switch (sub.type())
	{
		SUB_CASE(mul, Array, sub, obj);
		SUB_CASE(mul, Number, sub, obj);
		SUB_CASE(mul, Object, sub, obj);
		default: return value();
	}
	else if (sub.is_array()) return Array_mul_value(sub, obj);
	else if (obj.is_array()) return Array_mul_value(obj, sub);
	else return value();
}


value&	jsonMul(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);
	return *EV.jsonView = subview * objview;
}

////////////////////////////////////////////////////////////////////////////////////////////
value operator / (const value& a, const value& b);

__forceinline value div_Array(const value& a, const value& b)
{
	value ar = value::array();
	bool	not_is_null = false;

	for (size_t i = 0; (i < a.size()) && (i < b.size()); i++)
		ar[i] = a.at(i) / b.at(i);

	return ar;
}


__forceinline value div_Number(const value& a, const value& b)
{
	if (a.is_integer())
	{
		if (b.is_integer())
		{
			if (b.as_number().to_int64() != 0) return value(a.as_number().to_int64() / b.as_number().to_int64());
		}
		else if (b.as_double() != 0.0) return value(a.as_number().to_double() / b.as_number().to_double());
	}
	else
	{
		if (b.is_integer())
		{
			if (b.as_number().to_int64() != 0) return value(a.as_number().to_double() / b.as_number().to_double());
		}
		else if (b.as_double() != 0.0) return value(a.as_number().to_double() / b.as_number().to_double());
	}
	return value();
}


__forceinline value div_Object(const value& a, const value& b)
{
	value obj = value();

	for (auto it = a.as_object().cbegin(); it != a.as_object().cend(); it++)
	{
		if (b.has_field(it->first))	obj[it->first] = it->second / b.at(it->first);
	}

	return obj;
}


__forceinline value Array_div_value(const value& a, const value& b)
{
	value ar = a;
	for (auto& it : ar.as_array()) it = it / b;
	return ar;
}


__forceinline value value_div_Array(const value& a, const value& b)
{
	value ar = b;
	for (auto& it : ar.as_array()) it = a / it;
	return ar;
}


value operator / (const value& sub, const value& obj)
{
	if (sub.type() == obj.type()) switch (sub.type())
	{
		SUB_CASE(div, Array, sub, obj);
		SUB_CASE(div, Number, sub, obj);
		SUB_CASE(div, Object, sub, obj);
	default: return value();
	}
	else if (sub.is_array()) return Array_div_value(sub, obj);
	else if (obj.is_array()) return value_div_Array(sub, obj);
	else return value();
}


value&	jsonDiv(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);
	return *EV.jsonView = subview / objview;
}

////////////////////////////////////////////////////////////////////////////////////////////
value operator - (const value& a, const value& b);

__forceinline value subtract_Array(const value& a, const value& b)
{
	value ar = value::array();
	bool	not_is_null = false;

	for (size_t i = 0; (i < a.size()) && (i < b.size()); i++)
		ar[i] = a.at(i) - b.at(i);

	return ar;
}


__forceinline value subtract_Number(const value& a, const value& b)
{
	if (a.is_integer() && b.is_integer())
		return value(a.as_number().to_int64() - b.as_number().to_int64());
	else
		return value(a.as_double() - b.as_double());
}


__forceinline value subtract_Object(const value& a, const value& b)
{
	value obj = value();

	for (auto it = a.as_object().cbegin(); it != a.as_object().cend(); it++)
	{
		if (b.has_field(it->first))	obj[it->first] = it->second - b.at(it->first);
	}

	return obj;
}


__forceinline value Array_subtract_value(const value& a, const value& b)
{
	value ar = a;
	for (auto& it : ar.as_array()) it = it - b;
	return ar;
}


value operator - (const value& sub, const value& obj)
{
	if (sub.type() == obj.type()) switch (sub.type())
	{
		SUB_CASE(subtract, Array, sub, obj);
		SUB_CASE(subtract, Number, sub, obj);
		SUB_CASE(subtract, Object, sub, obj);
	default: return value();
	}
	else if (sub.is_array()) return Array_subtract_value(sub, obj);
	else if (obj.is_array()) return Array_subtract_value(obj, sub);
	else return value();
}


value&	jsonSubtract(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);
	return *EV.jsonView = subview - objview;
}


//////////////////////////////////////////////////////////////////////////////////////////////
value&	jsonPower(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);

	if (subview.is_number() && objview.is_number())
	{
		double db = objview.is_integer() ? double(objview.as_number().to_int64()) : objview.as_double();

		if (subview.is_integer())
		{	//	result must be integer
			double da = double(subview.as_number().to_int64());
			return *EV.jsonView = value(__int64(pow(da, db) + .5));
		}
		else
		{	//	result must be double
			double da = subview.as_double();
			return *EV.jsonView = value(pow(da, db));
		}
	}
	else
	{
		return *EV.jsonView = value(L"power: error, both arguments must be numbers!");
	}
}


value&	jsonSqrt(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);

	if (objview.is_number())
		return subview = value(sqrt(objview.as_double()));
	else
		return subview = value(L"sqrt: error, both arguments must be numbers!");
}


////////////////////////////////////////////////////////////////////////////////////

value&		jsonForEach(EntView &EV)
{
	value&	subview = *(EV.subview = EV.jsonView);
	value&	objview = EV.GetObjView(EV._ov);

	if (objview.is_array())
	{
		subview = value::array();
		for (size_t i = 0; i < objview.size(); i++)
		{
			EntView	ctx(EV.parent, EV.ent, EV.ent, EV.sub, EV.obj, subview[i]);
			ctx.subview = &subview[i];
			ctx.objview = &objview[i];
			value& res = ctx.jsonExec(EV.sub/*ctx.rel*/);
			subview[i] = res;
		}
	}
	else if (objview.is_object())
	{
		subview = value::array();
		size_t i = 0;
		for ( auto& it : objview.as_object() )
		{
			EntView	ctx(EV.parent, EV.ent, EV.ent, EV.sub, EV.obj, subview[i]);
			ctx.subview = &subview[i];
			ctx.objview = &it.second;
			subview[i] = ctx.jsonExec(EV.sub/*ctx.rel*/);
			i++;
		}
	}
	else
	{
		EntView	ctx(EV.parent, EV.ent, EV.ent, EV.sub, EV.obj, subview);
		ctx.subview = &subview;
		ctx.objview = &objview;
		subview = ctx.jsonExec(EV.sub/*ctx.rel*/);
	}

	return subview;
}


value&	jsonSize(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);
	return subview = value(objview.size());
}

#define define_json_is_type(json_type)					\
value&	json_is_##json_type(EntView &EV)				\
{														\
	value&	subview = EV.GetSubView(EV._sv);					\
	value&	objview = EV.GetObjView(EV._ov);					\
	return subview = value(objview.is_##json_type());	\
}

define_json_is_type(array)
define_json_is_type(boolean)
define_json_is_type(double)
define_json_is_type(integer)
define_json_is_type(null)
define_json_is_type(number)
define_json_is_type(object)
define_json_is_type(string)


value&	jsonIntSeq(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);

	if (objview.is_integer() && subview.is_integer())
	{
		*EV.jsonView = value::array();
		int i = 0;
		int from = objview.as_integer();
		int to = subview.as_integer();

		if (from < to)
		{
			for (; from <= to; from++)
				(*EV.jsonView)[i++] = value(from);
		}
		else
		{
			for (; from >= to; from--)
				(*EV.jsonView)[i++] = value(from);
		}
	}
	else
	{
		*EV.jsonView = value(L"int_seq: error, both arguments must be integer!");
	}

	return *EV.jsonView;
}


value&	jsonUnion(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);
	value&	result = *EV.jsonView;
	result = subview;
	size_t i = result.as_array().size();
	for (auto& it : objview.as_array())
		result[i++] = it;
	return result;
}


value&		jsonNull(EntView &EV)
{
	//	just do nothing
	return *EV.jsonView;
}


value&	jsonInt32(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);

	switch (objview.type())
	{
	case value::Array:
	case value::Object:
		return subview = value(int(objview.size()));

	case value::String:
		return subview = value(int(_wtoi(objview.as_string().c_str())));

	case value::Boolean:
		if(objview.as_bool())
			return subview = value(int(1));
		else
			return subview = value(int(0));

	case value::Number:
		if (objview.is_integer())
			return subview = value(int(objview.as_integer()));
		else
			return subview = value(int(objview.as_double()));

	default:
		return subview = value(int(0));
	}
}


value&	jsonDouble(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);

	switch (objview.type())
	{
	case value::Array:
	case value::Object:
		return subview = value(double(objview.size()));

	case value::String:
		return subview = value(double(_wtof(objview.as_string().c_str())));

	case value::Boolean:
		if (objview.as_bool()) return subview = value(double(1));
		else return subview = value(double(0));

	case value::Number:
		if (objview.is_integer()) return subview = value(double(objview.as_integer()));
		else return subview = value(double(objview.as_double()));

	default:
		return subview = value(double(0));
	}
}


value&	jsonSplitString(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);

	if (objview.is_string() && subview.is_string())
	{
		const string_t& str = objview.as_string();
		const string_t& delim = subview.as_string();
		size_t prev = 0, pos = 0, i = 0;
		*EV.jsonView = value::array();
		do
		{
			pos = str.find(delim, prev);
			if (pos == string_t::npos) pos = str.length();
			string_t token = str.substr(prev, pos - prev);
			if (!token.empty()) (*EV.jsonView)[i++] = value(token);
			prev = pos + delim.length();
		} while (pos < str.length() && prev < str.length());
	}
	else
		*EV.jsonView = value();

	return *EV.jsonView;
}


value&	jsonJoinString(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);	//	array
	value&	objview = EV.GetObjView(EV._ov);	//	separator string
	wstring	result = L"";

	if (objview.is_string() && subview.is_array())
	{
		for (auto& it : subview.as_array())
		{
			if (result != L"") result += objview.as_string();
			switch (it.type())
			{
			case value::value_type::Object:
			case value::value_type::Array:
				break;	//	сложные типы не печатаем

			case value::value_type::Number:
				if (!it.as_number().is_integral())		result += to_wstring(it.as_number().to_double());
				else if (it.as_number().is_int32())		result += to_wstring(it.as_number().to_int32());
				else if (it.as_number().is_uint32())	result += to_wstring(it.as_number().to_uint32());
				else if (it.as_number().is_int64())		result += to_wstring(it.as_number().to_int64());
				else if (it.as_number().is_uint64())	result += to_wstring(it.as_number().to_uint64());
				break;

			case value::value_type::String:		//	иерархическая символьная ссылка в проекции сущности
				result += it.as_string();
				break;

			case value::value_type::Boolean:	//	фильтрация атрибутов модели по маске, управление видимостью проекций
				result += it.serialize();
				break;

			default:	//	null не печатаем
				break;
			}
		}
	}

	return *EV.jsonView = value(result);
}


value&	jsonAt(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);

	if (subview.is_array())
	{
		if (objview.is_number())
			return subview[objview.as_number().to_uint32()];
		else if (objview.is_string())
			return subview[_wtoi(objview.as_string().c_str())];
	}
	else if (subview.is_object())
	{
		if (objview.is_number())
			return subview[to_wstring(objview.as_number().to_uint32())];
		else if (objview.is_string())
			return subview[objview.as_string()];
	}
	else if (subview.is_null())
	{
		if (objview.is_number())
		{
			subview = value::array();
			return subview[objview.as_number().to_uint32()];
		}
		else if (objview.is_string())
		{
			subview = value::object();
			return subview[objview.as_string()];
		}
	}

	return *EV.jsonView = value();
}


value&	jsonEq(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);
	return *EV.jsonView = value(subview == objview);
}


value&	jsonNotEq(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);
	return *EV.jsonView = value(subview != objview);
}


value&	jsonSum(EntView &EV)
{
	__int64	isum = 0;
	double	dsum = 0.0;
	value& subview = EV.GetSubView(EV._sv);
	value& objview = EV.GetObjView(EV._ov);

	if (objview.is_array())
	{
		for (auto& it : objview.as_array())
		{
			if (it.is_number())
			{
				const number &num = it.as_number();
				if (it.is_integer()) isum += num.to_int64();
				else dsum += num.to_double();
			}
		}
	}
	
	if (0.0 == dsum)
		subview = value(isum);
	else if (0 == isum)
		subview = value(dsum);
	else
		subview = value(dsum + double(isum));

	return subview;
}


value&	jsonWhere(EntView &EV)
{
	value& jsonView = *EV.jsonView;
	value& subview = EV.GetSubView(EV._sv);	//	множество элементов
	value& objview = EV._ov;
	
	if (subview.is_array())
	{
		jsonView = value::array();
		for (size_t i = 0; i < subview.size(); i++)
		{	//	исполняем объект
			EntView	ctx(EV.parent, EV.ent, EV.sub, EV.obj, EV.obj, objview);
			ctx.subview = &objview;	//	результат проверки критерия записываем в проекцию объекта
			ctx.objview = &subview[i];	//	проекция объекта это элемент из множества проекции субъекта
			objview = ctx.jsonExec(EV.obj/*ctx.rel*/);
			if (objview.is_boolean())
			{
				if (objview.as_bool())
				{
					jsonView[jsonView.size()] = subview[i];	//	фильтруем
				}
			}
		}
	}
	else
	{
		jsonView = value();
	}

	return jsonView;
}


value&	jsonBelow(EntView &EV)	//	<
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);

	if(subview.type() == objview.type()) switch (objview.type())
	{
	case value::Array:
	case value::Object:
		return *EV.jsonView = value(subview.size() < objview.size());

	case value::String:
		return *EV.jsonView = value(subview.as_string().size() < objview.as_string().size());

	case value::Boolean:
		return *EV.jsonView = value(int(subview.as_bool()) < int(objview.as_bool()));

	case value::Number:
		if (objview.is_integer())
			return *EV.jsonView = value(subview.as_integer() < objview.as_integer());
		else
			return *EV.jsonView = value(subview.as_double() < objview.as_double());

	default:
		return *EV.jsonView = value();
	}

	return *EV.jsonView = value();
}


value&	jsonAnd(EntView &EV)
{
	value&	subview = EV.GetSubView(EV._sv);
	value&	objview = EV.GetObjView(EV._ov);

	if (subview.is_boolean() && objview.is_boolean())
	{
		return *EV.jsonView = value(subview.as_bool() && objview.as_bool());
	}

	return *EV.jsonView = value(false);
}

////////////////////////////////////////////////////////////////////////////////

bool	InitBaseVoc(EntView &EV)
{
#define map_json_is_type(json_type)	EV.Addx86Entity(L"is_"s + L#json_type, json_is_##json_type );
	map_json_is_type(array);
	map_json_is_type(boolean);
	map_json_is_type(double);
	map_json_is_type(integer);
	map_json_is_type(null);
	map_json_is_type(number);
	map_json_is_type(object);
	map_json_is_type(string);

	//Addx86Entity( "funcname", func );
	EV.Addx86Entity(L"union"s, jsonUnion);
	EV.Addx86Entity(L"Union"s, jsonUnion);
	EV.Addx86Entity(L"null"s, jsonNull );
	EV.Addx86Entity(L"Int32"s, jsonInt32 );
	EV.Addx86Entity(L"int"s, jsonInt32 );
	EV.Addx86Entity(L"double"s, jsonDouble );
	EV.Addx86Entity(L"*"s, jsonMul );
	EV.Addx86Entity(L"/"s, jsonDiv );
	EV.Addx86Entity(L"-", jsonSubtract );
	EV.Addx86Entity(L"pow"s, jsonPower );
	EV.Addx86Entity(L"^"s, jsonXOR );
	EV.Addx86Entity(L"SplitString"s, jsonSplitString );
	EV.Addx86Entity(L"sqrt"s, jsonSqrt );
	EV.Addx86Entity(L"foreach"s, jsonForEach );
	EV.Addx86Entity(L"int_seq"s, jsonIntSeq );
	EV.Addx86Entity(L"size"s, jsonSize );
	EV.Addx86Entity(L"JoinString", jsonJoinString );
	EV.Addx86Entity(L"at"s, jsonAt );
	EV.Addx86Entity(L"."s, jsonAt );
	EV.Addx86Entity(L"=="s, jsonEq );
	EV.Addx86Entity(L"!="s, jsonNotEq );
	EV.Addx86Entity(L"sum"s, jsonSum );
	EV.Addx86Entity(L"Where", jsonWhere );
	EV.Addx86Entity(L"<", jsonBelow );
	EV.Addx86Entity(L"And", jsonAnd );
	EV.Addx86Entity(L"&&", jsonAnd );
	return true;
}

