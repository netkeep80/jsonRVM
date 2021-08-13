/*        R
	   S__|__O
	 O   _|_   S
  R__|__/_|_\__|__R  jsonRVM
	 |  \_|_/  |     json Relations (Model) Virtual Machine
	 S    |    O     https://github.com/netkeep80/jsonRVM
		__|__
	   /  |  \
	  /___|___\
Fractal Triune Entity

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
Copyright � 2016 Vertushkin Roman Pavlovich <https://vk.com/earthbirthbook>.

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
#pragma once
#include <iostream>
#include <chrono>
#include <thread>
#include "JsonRVM.h"

namespace rm
{
	using namespace std;

	template<typename type_t>
	struct _add
	{
		typedef type_t& _Tref;	type_t c;
		operator _Tref() { return c; };
		_add(const type_t a, const type_t b) { c = a + b; };
	};

	template<typename type_t>
	struct _sub
	{
		typedef type_t& _Tref;	type_t c;
		operator _Tref() { return c; };
		_sub(const type_t a, const type_t b) { c = a - b; };
	};

	template<typename type_t>
	struct _mul
	{
		typedef type_t& _Tref;	type_t c;
		operator _Tref() { return c; };
		_mul(const type_t a, const type_t b) : c(a* b) {};
	};

	template<typename type_t>
	struct _div
	{
		typedef type_t& _Tref;	type_t c;
		operator _Tref() { return c; };
		_div(const type_t a, const type_t b) : c(a / b) {};
	};

	template<typename type_t, typename _OP>
	json type_operation(const json& a, const json& b)
	{
		return json(type_t(_OP(a.get<type_t>(), b.get<type_t>())));
	}

	template<typename type_t, typename _OP>
	json diff_operation(const json& a, const json& b)
	{
		_OP	operation(a.get<type_t>(), b.get<type_t>());
		type_t& res = operation;
		if (res == type_t()) return json();
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

	inline json operator ^ (const json& a, const json& b);

	inline json xor_array(const json& a, const json& b)
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

	inline json xor_boolean(const json& a, const json& b)
	{
		if (a.get<bool>() != b.get<bool>())	return json::array({ a, b });
		else return json();
	}

	inline json xor_number_float(const json& a, const json& b) { return diff_operation<json::number_float_t, _sub<json::number_float_t>>(a, b); }
	inline json xor_number_integer(const json& a, const json& b) { return diff_operation<json::number_integer_t, _sub<json::number_integer_t>>(a, b); }
	inline json xor_number_unsigned(const json& a, const json& b) { return diff_operation<json::number_unsigned_t, _sub<json::number_unsigned_t>>(a, b); }

	inline json xor_object(const json& a, const json& b)
	{
		json obj = json(), field;

		for (auto it = a.begin(); it != a.end(); it++)
		{
			const string& key = it.key();
			const json& val = it.value();
			if (b.count(key))
			{
				field = val ^ b.at(key);
				if (!field.is_null()) obj[key] = field;
			}
			else obj[key] = val;
		}

		for (auto it = b.begin(); it != b.end(); it++)
		{
			const string& key = it.key();
			const json& val = it.value();
			if (a.count(key))
			{
				field = a.at(key) ^ val;
				if (!field.is_null()) obj[key] = field;
			}
			else obj[key] = val;
		}

		return obj;
	}

	inline json xor_string(const json& a, const json& b)
	{
		if (a.get<string>() == b.get<string>()) return json();
		else return json::array({ a, b });
	}

	inline json value_xor_array(const json& a, const json& b)
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

	inline json array_xor_value(const json& a, const json& b)
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

	inline json operator ^ (const json& a, const json& b)
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

	void  sleep_ms(jsonRVM& rmvm, EntContext& ec)
	{
		this_thread::sleep_for(chrono::milliseconds(ec.obj.get<json::number_unsigned_t>()));
	}

	void  jsonCopy(jsonRVM& rmvm, EntContext& ec)
	{	//	полное копированиее json значения объекта в субъект
		ec.sub = ec.obj;
	}

	void  jsonView(jsonRVM& rmvm, EntContext& ec)
	{	//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
		//	проецируем во внешнем контексте
		rmvm.JSONExec(EntContext(ec.sub, ec.$.obj, ec.$.sub, ec.$.ent, ec.$), ec.obj);
	}

	void  jsonXOR(jsonRVM& rmvm, EntContext& ec)
	{
		ec.its = ec.sub ^ ec.obj;
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
	{ ec.its = (ec.sub.get_ref<stype&>()) operation (ec.obj.get_ref<otype&>()); return; }

#define OPP_ANYTO(operation,type,type_id)														\
	OPP_STO(operation, jf, number_float,    type, type_id)										\
	OPP_STO(operation, ji, number_integer,  type, type_id)										\
	OPP_STO(operation, ju, number_unsigned, type, type_id)

#define VM_OPP(operation)																		\
	OPP_ANYTO(operation, jf, number_float)														\
	OPP_ANYTO(operation, ji, number_integer)													\
	OPP_ANYTO(operation, ju, number_unsigned)

#define	OP_BODY( name, operation )																\
void  json##name (jsonRVM& rmvm, EntContext& ec)			\
{																								\
	switch( (uint8_t(ec.sub.type()) << sub_field) | uint8_t(ec.obj.type()) )							\
	{ VM_OPP( operation ) default: ec.its = json(); }												\
}

	OP_BODY(Add, +);
	OP_BODY(Substract, -);
	OP_BODY(Mul, *);

	void  jsonDiv(jsonRVM& rmvm, EntContext& ec)
	{
		if (ec.obj.is_number())
			if (ec.obj.get<double>() == 0.0)
			{
				ec.its = json();
				return;
			}
		switch ((uint8_t(ec.sub.type()) << sub_field) | uint8_t(ec.obj.type()))
		{
		VM_OPP(/ ) default: ec.its = json();
		}
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	void  jsonPower(jsonRVM& rmvm, EntContext& ec)
	{
		if (ec.sub.is_number() && ec.obj.is_number())
		{
			json::number_float_t db = ec.obj.is_number_float() ? ec.obj.get<json::number_float_t>() : json::number_float_t(ec.obj.get<json::number_integer_t>());

			if (!ec.sub.is_number_float())
			{	//	result must be integer
				json::number_float_t da = json::number_float_t(ec.sub.get<json::number_integer_t>());
				ec.its = json::number_integer_t(pow(da, db) + .5);
			}
			else
			{	//	result must be double
				json::number_float_t da = ec.sub.get<json::number_float_t>();
				ec.its = json::number_float_t(pow(da, db));
			}
		}
		else
			ec.its = json();
	}

	void  jsonSqrt(jsonRVM& rmvm, EntContext& ec)
	{
		if (ec.obj.is_number())
			ec.sub = json::number_float_t(sqrt(ec.obj.get<json::number_float_t>()));
		else
			ec.sub = json();
	}

	////////////////////////////////////////////////////////////////////////////////////


	void  jsonForEachObject(jsonRVM& rmvm, EntContext& ec)
	{	/*
			Множественный JSONExec для проекции объекта типа array
		*/
		if (ec.obj.is_array())
		{
			if (!ec.its.is_array()) ec.its = json::array();
			size_t i = 0;
			for (auto& it : ec.obj)
			{
				try {
					json& value = ec.its[i];
					rmvm.JSONExec(EntContext(value, it, value, ec.ent, ec.$), ec.sub); i++;
				}
				catch (json& j) { ec.throw_json(__FUNCTION__, json({ {"["s + to_string(i) + "]"s, j} })); }
			}
		}
		else
			ec.throw_json(__FUNCTION__, "$obj must be array!"s);
	}

	void  jsonForEachSubject(jsonRVM& rmvm, EntContext& ec)
	{	/*
			Множественный JSONExec для субъекта типа array
		*/
		if (ec.sub.is_array())
		{
			if (!ec.its.is_array()) ec.its = json::array();
			size_t i = 0;
			for (auto& it : ec.sub)
			{
				try {
					json& value = ec.its[i];
					rmvm.JSONExec(EntContext(value, it, value, ec.ent, ec.$), ec.obj); i++;
				}
				catch (json& j) { ec.throw_json(__FUNCTION__, json({ {"["s + to_string(i) + "]"s, j} })); }
			}
		}
		else
			ec.throw_json(__FUNCTION__, "$sub must be array!"s);
	}

	void  jsonSize(jsonRVM& rmvm, EntContext& ec)
	{
		ec.sub = ec.obj.size();
	}

#define define_json_is_type(json_type)							\
	void  json_is_##json_type(jsonRVM& rmvm, EntContext& ec)		\
	{															\
		ec.sub = ec.obj.is_##json_type();						\
	}

	void  json_is_not_null(jsonRVM& rmvm, EntContext& ec)
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

		void  jsonIntegerSequence(jsonRVM& rmvm, EntContext& ec)
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
			ec.throw_json(__FUNCTION__, "$obj must has 'from', 'to' and 'step' properties!"s);
	}

	void  jsonUnion(jsonRVM& rmvm, EntContext& ec)
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

		ec.its = ec.sub;
	}

	void  jsonNull(jsonRVM& rmvm, EntContext& ec) {}

	void  jsonInt32(jsonRVM& rmvm, EntContext& ec)
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

		ec.its = ec.sub;
	}

	void  jsonDouble(jsonRVM& rmvm, EntContext& ec)
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


	void  string_string(jsonRVM& rmvm, EntContext& ec)
	{
		ec.sub = ""s;
		string& result = ec.sub.get_ref<string&>();

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


	void  string_add(jsonRVM& rmvm, EntContext& ec)
	{
		if (!ec.sub.is_string())	ec.sub = ""s;
		string& result = ec.sub.get_ref<string&>();

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


	void  string_find(jsonRVM& rmvm, EntContext& ec)
	{
		if (!(ec.obj.is_string() && ec.sub.is_string())) ec.throw_json(__FUNCTION__, "$obj and $sub must be strings!"s);
		ec.its = static_cast<json::number_integer_t>(ec.obj.get_ref<string&>().find(ec.sub.get_ref<string&>().c_str()));
	}


	void  string_split(jsonRVM& rmvm, EntContext& ec)
	{
		if (ec.obj.is_string() && ec.sub.is_string())
		{
			const string& delim = ec.obj.get_ref<string&>();
			string str = ec.sub.get_ref<string&>();
			size_t prev = 0, pos = 0;
			ec.its = json::array();

			do
			{
				pos = str.find(delim, prev);
				if (pos == string::npos) pos = str.length();
				ec.its.push_back(str.substr(prev, pos - prev));
				prev = pos + delim.length();
			} while (pos < str.length() && prev < str.length());
		}
		else
			ec.throw_json(__FUNCTION__, "$obj and $sub must be string!"s);
	}

	void  string_join(jsonRVM& rmvm, EntContext& ec)
	{
		string	result = "";
		bool	first = true;

		if (ec.obj.is_string() && ec.sub.is_array())
		{
			string& splitter = ec.obj.get_ref<string&>();

			for (auto& it : ec.sub)
			{
				if (first) first = false;
				else       result += splitter;

				switch (it.type())
				{
				case json::value_t::object:
				case json::value_t::array:
					ec.throw_json(__FUNCTION__, "$sub must be array of simple type values!"s);

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

			ec.its = result;
		}
		else
			ec.throw_json(__FUNCTION__, "$obj must be string and $sub must be array!"s);
	}

	void  jsonGet(jsonRVM& rmvm, EntContext& ec)
	{
		if (ec.sub.is_array())
		{
			if (ec.obj.is_number())
				ec.its = ec.sub[ec.obj.get<size_t>()];
			else
				ec.throw_json(__FUNCTION__, "$obj must be unsigned number!"s);
		}
		else if (ec.sub.is_object())
		{
			if (ec.obj.is_string())
				ec.its = ec.sub[ec.obj.get_ref<string&>()];
			else
				ec.throw_json(__FUNCTION__, "$obj must be string!"s);
		}
		else
			ec.throw_json(__FUNCTION__, "$sub must be array or object!"s);
	}

	void  jsonSet(jsonRVM& rmvm, EntContext& ec)
	{
		if (ec.sub.is_array())
		{
			if (ec.obj.is_number_unsigned())
				ec.sub[ec.obj.get<size_t>()] = ec.its;
			else
				ec.throw_json(__FUNCTION__, "$obj must be unsigned number!"s);
		}
		else if (ec.sub.is_object())
		{
			if (ec.obj.is_string())
				ec.sub[ec.obj.get_ref<string&>()] = ec.its;
			else
				ec.throw_json(__FUNCTION__, "$obj must be string!"s);
		}
		else if (ec.sub.is_null())
		{
			if (ec.obj.is_number_unsigned())
				ec.sub[ec.obj.get<size_t>()] = ec.its;
			else if (ec.obj.is_string())
				ec.sub[ec.obj.get_ref<string&>()] = ec.its;
			else
				ec.throw_json(__FUNCTION__, "$obj must be unsigned number or string!"s);
		}
		else
			ec.throw_json(__FUNCTION__, "$sub must be array, object or null!"s);
	}

	void  jsonErase(jsonRVM& rmvm, EntContext& ec)
	{
		if (ec.sub.is_object())
		{
			if (ec.obj.is_string())
				ec.sub.erase(ec.obj.get_ref<string&>());
			else
				ec.throw_json(__FUNCTION__, "$obj must be string!"s);
		}
		else
			ec.throw_json(__FUNCTION__, "$sub must be object!"s);
	}

	void  jsonIsEq(jsonRVM& rmvm, EntContext& ec)
	{
		ec.its = ec.sub == ec.obj;
	}

	void  jsonIsNotEq(jsonRVM& rmvm, EntContext& ec)
	{
		ec.its = ec.sub != ec.obj;
	}

	void  jsonSum(jsonRVM& rmvm, EntContext& ec)
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
			ec.throw_json(__FUNCTION__, "$obj must be json array!"s);
	}

	void  jsonWhere(jsonRVM& rmvm, EntContext& ec)
	{
		ec.its = json::array();		//	подготовка выходного массива

		if (ec.obj.is_null()) return;

		if (ec.obj.is_array())
		{
			size_t i = 0;
			for (json& it : ec.obj)
			{
				try {
					json	boolres;
					rmvm.JSONExec(EntContext(boolres, it, boolres, ec.ent, ec.$), ec.sub);
					if (boolres.is_boolean())
						if (boolres.get<bool>())
							ec.its.push_back(it);	//	фильтруем
					i++;
				}
				catch (json& j) { ec.throw_json(__FUNCTION__, json({ {"["s + to_string(i) + "]"s, j} })); }
			}
		}
		else
			ec.throw_json(__FUNCTION__, "$obj must be json array!"s);
	}

	void  jsonBelow(jsonRVM& rmvm, EntContext& ec)	//	<
	{
		if (ec.sub.type() == ec.obj.type()) switch (ec.obj.type())
		{
		case json::value_t::array:
		case json::value_t::object:
			ec.its = json(ec.sub.size() < ec.obj.size());
			return;

		case json::value_t::string:
			ec.its = json(ec.sub.get<string>().size() < ec.obj.get<string>().size());
			return;

		case json::value_t::boolean:
			ec.its = json(int(ec.sub.get<bool>()) < int(ec.obj.get<bool>()));
			return;

		case json::value_t::number_float:
			ec.its = json(ec.sub.get<json::number_float_t>() < ec.obj.get<json::number_float_t>());
			return;

		case json::value_t::number_integer:
			ec.its = json(ec.sub.get<json::number_integer_t>() < ec.obj.get<json::number_integer_t>());
			return;

		case json::value_t::number_unsigned:
			ec.its = json(ec.sub.get<json::number_unsigned_t>() < ec.obj.get<json::number_unsigned_t>());
			return;

		default:
			ec.its = json();
			return;
		}
	}

	void  jsonAnd(jsonRVM& rmvm, EntContext& ec)
	{
		if (ec.sub.is_boolean() && ec.obj.is_boolean())
			ec.its = ec.sub.get<bool>() && ec.obj.get<bool>();
		else
			ec.throw_json(__FUNCTION__, "$obj and $sub must be boolean!"s);
	}

	void  IfObjTrueThenExecSub(jsonRVM& rmvm, EntContext& ec)
	{
		if (!ec.obj.is_boolean())
			ec.throw_json(__FUNCTION__, "$obj must be boolean!"s);

		try
		{
			if (ec.obj.get<bool>())
				rmvm.JSONExec(ec.$, ec.sub);
		}
		catch (json& j) { throw json({ { __FUNCTION__, j} }); }
	}

	void  IfObjFalseThenExecSub(jsonRVM& rmvm, EntContext& ec)
	{
		if (!ec.obj.is_boolean())
			ec.throw_json(__FUNCTION__, "$obj must be boolean!"s);

		try
		{
			if (!ec.obj.get<bool>())
				rmvm.JSONExec(ec.$, ec.sub);
		}
		catch (json& j) { throw json({ { __FUNCTION__, j} }); }
	}

	void  ExecSubWhileObjTrue(jsonRVM& rmvm, EntContext& ec)
	{
		while (true)
		{
			if (!ec.obj.is_boolean()) ec.throw_json(__FUNCTION__, "$obj must be boolean!"s);
			if (!ec.obj.get<bool>()) return;
			try
			{
				rmvm.JSONExec(ec.$, ec.sub);
			}
			catch (json& j) { throw json({ { __FUNCTION__, j} }); }
		}
	}

	void  json_switch_bool(jsonRVM& rmvm, EntContext& ec)
	{
		if (ec.obj.is_null())
			return;

		if (!ec.obj.is_boolean())
			ec.throw_json(__FUNCTION__, "$obj must be boolean!"s);

		if (!ec.sub.is_object())
			ec.throw_json(__FUNCTION__, "$sub must be json object!"s);

		try
		{
			if (ec.obj.get<bool>())	rmvm.JSONExec(ec.$, ec.sub["true"]);
			else	rmvm.JSONExec(ec.$, ec.sub["false"]);
		}
		catch (json& j) { throw json({ { __FUNCTION__, j} }); }
		catch (json::exception& e) { ec.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { ec.throw_json(__FUNCTION__, "std::exception: "s + e.what()); }
		catch (...) { ec.throw_json(__FUNCTION__, "unknown exception"s); }
	}

	void  json_switch_number(jsonRVM& rmvm, EntContext& ec)
	{
		if (ec.obj.is_null())
			return;

		if (!(ec.obj.is_number_unsigned() || ec.obj.is_number_integer()))
			ec.throw_json(__FUNCTION__, "$obj must be number!"s);

		if (!ec.sub.is_object())
			ec.throw_json(__FUNCTION__, "$sub must be json object!"s);

		try
		{
			string	key;
			if (ec.obj.is_number_unsigned())
				key = to_string(ec.obj.get<json::number_unsigned_t>());
			else if (ec.obj.is_number_integer())
				key = to_string(ec.obj.get<json::number_integer_t>());

			if (ec.sub.count(key)) rmvm.JSONExec(ec.$, ec.sub[key]);
			else rmvm.JSONExec(ec.$, ec.sub["default"]);
		}
		catch (json& j) { throw json({ { __FUNCTION__, j} }); }
		catch (json::exception& e) { ec.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { ec.throw_json(__FUNCTION__, "std::exception: "s + e.what()); }
		catch (...) { ec.throw_json(__FUNCTION__, "unknown exception"s); }
	}


	void  json_switch_string(jsonRVM& rmvm, EntContext& ec)
	{
		if (ec.obj.is_null())
			return;

		if (!ec.obj.is_string())
			ec.throw_json(__FUNCTION__, "$obj must be string!"s);

		if (!ec.sub.is_object())
			ec.throw_json(__FUNCTION__, "$sub must be json object!"s);

		try
		{
			if (ec.sub.count(ec.obj.get_ref<string&>())) rmvm.JSONExec(ec.$, ec.sub[ec.obj.get_ref<string&>()]);
			else rmvm.JSONExec(ec.$, ec.sub["default"]);
		}
		catch (json& j) { throw json({ { __FUNCTION__, j} }); }
		catch (json::exception& e) { ec.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { ec.throw_json(__FUNCTION__, "std::exception: "s + e.what()); }
		catch (...) { ec.throw_json(__FUNCTION__, "unknown exception"s); }
	}


	void  json_throw(jsonRVM& rmvm, EntContext& ec) { throw ec.obj; }


	void  json_catch(jsonRVM& rmvm, EntContext& ec)
	{
		//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
		//	проецируем во внешнем контексте
		try
		{
			rmvm.JSONExec(EntContext(ec.its, ec.$.obj, ec.$.sub, ec.$.ent, ec.$), ec.obj);
		}
		catch (json& j)
		{
			ec.its = j;
			rmvm.JSONExec(EntContext(ec.its, ec.$.obj, ec.$.sub, ec.$.ent, ec.$), ec.sub);
		}
	}


#define define_object_method(object_method)										\
		void  json_call_##object_method(jsonRVM& rmvm, EntContext& ec)	\
		{																		\
			ec.sub = ec.obj.##object_method();									\
		}

#define define_static_method(static_method)										\
		void  json_call_##static_method(jsonRVM& rmvm, EntContext& ec)	\
		{																		\
			ec.its = json::##static_method();									\
		}

	define_static_method(array)
		define_static_method(meta)
		define_static_method(object)

		void  json_call_null(jsonRVM& rmvm, EntContext& ec)
	{
		ec.its = json();
	}

	void  jsonPrint(jsonRVM& rmvm, EntContext& ec)
	{
		cout << ec.obj.dump(1) << endl;
	}


	void  jsonTAG(jsonRVM& rmvm, EntContext& ec)
	{
		if (!ec.sub.is_object())
			ec.throw_json(__FUNCTION__, "$sub must be json object!"s);

		string	tag = ec.sub["<>"];

		if (!ec.its.is_string())
			ec.its = ""s;

		string& body = ec.its.get_ref<string&>();
		body += "<" + tag;

		for (auto& it : ec.sub.items())
			if ("<>" != it.key())
				body += " " + it.key() + "=" + it.value().dump();

		body += ">";
		//	что бы выходной поток xml попадал в тоже значение ec.$its нужно исполнять в текущем контексitsV
		json	objview;
		rmvm.JSONExec(EntContext(objview, ec.obj, ec.sub, ec.ent, ec.$), ec.obj);
		if (objview.is_string()) body += objview.get_ref<string&>();
		else                     body += objview.dump();
		body += "</" + tag + ">";
	}


	void  jsonXML(jsonRVM& rmvm, EntContext& ec)
	{
		std::string res = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"s;

		if (!ec.sub.is_object())
			ec.throw_json(__FUNCTION__, "$sub must be json object!"s);

		string	tag = ec.sub["<>"];

		if (!ec.its.is_string())
			ec.its = res;
		else if (!ec.its.get_ref<string&>().size())
			ec.its = res;

		string& body = ec.its.get_ref<string&>();
		body += "<" + tag;

		for (auto& it : ec.sub.items())
			if ("<>" != it.key())
				body += " " + it.key() + "=" + it.value().dump();

		body += ">";
		//	что бы выходной поток xml попадал в тоже значение ec.$its нужно исполнять в текущем контексitsV
		json	objview;
		rmvm.JSONExec(EntContext(objview, ec.obj, ec.sub, ec.ent, ec.$), ec.obj);
		if (objview.is_string()) body += objview.get_ref<string&>();
		else                     body += objview.dump();
		body += "</" + tag + ">";
	}


	void  jsonHTML(jsonRVM& rmvm, EntContext& ec)
	{
		std::string res = "<!DOCTYPE html>"s;

		if (!ec.sub.is_object())
			ec.throw_json(__FUNCTION__, "$sub must be json object!"s);

		string	tag = ec.sub["<>"];

		if (!ec.its.is_string())
			ec.its = res;
		else if (!ec.its.get_ref<string&>().size())
			ec.its = res;

		string& body = ec.its.get_ref<string&>();
		body += "<" + tag;

		for (auto& it : ec.sub.items())
			if ("<>" != it.key())
				body += " " + it.key() + "=" + it.value().dump();

		body += ">";
		//	что бы выходной поток xml попадал в тоже значение ec.$its нужно исполнять в текущем контексitsV
		json	objview;
		rmvm.JSONExec(EntContext(objview, ec.obj, ec.sub, ec.ent, ec.$), ec.obj);
		if (objview.is_string()) body += objview.get_ref<string&>();
		else                     body += objview.dump();
		body += "</" + tag + ">";
	}

	void  jsonDump(jsonRVM& rmvm, EntContext& ec)
	{
		ec.sub = ec.obj.dump(3);
	}

	template<class duration>
	void steady_clock_(jsonRVM& rmvm, EntContext& ec) {
		ec.its = chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now().time_since_epoch()).count();
	}

	const string&	ImportRelationsModel(jsonRVM& rmvm)
	{
		import_help(rmvm);
		rmvm["rmvm"]["version"] = rmvm_version;
		rmvm.AddBaseEntity(rmvm["sleep"], "ms"s, sleep_ms, "sleep in milliconds"s);
		rmvm.AddBaseEntity(rmvm, "view"s, jsonView, "ViewEntity: View object model in parent ctx and then set subject value"s);
		rmvm.AddBaseEntity(rmvm, "="s, jsonCopy, "Copy: copy object model to subject value"s);

#define map_json_is_type(json_type)	rmvm.AddBaseEntity( rmvm, "is_"s + #json_type, json_is_##json_type, ""s );
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
#define map_json_static_method(static_method)	rmvm.AddBaseEntity( rmvm["json"], #static_method, json_call_##static_method, ""s );
		map_json_static_method(array);
		map_json_static_method(null);
		map_json_static_method(meta);
		map_json_static_method(object);

		rmvm.AddBaseEntity(rmvm["json"], "dump"s, jsonDump, ""s);

		//	convert
		rmvm.AddBaseEntity(rmvm, "integer"s, jsonInt32, ""s);
		rmvm.AddBaseEntity(rmvm, "int"s, jsonInt32, ""s);
		rmvm.AddBaseEntity(rmvm, "float"s, jsonDouble, ""s);
		rmvm.AddBaseEntity(rmvm, "double"s, jsonDouble, ""s);
		rmvm.AddBaseEntity(rmvm, "null"s, jsonNull, ""s);

		//	data operations
		rmvm.AddBaseEntity(rmvm, "where"s, jsonWhere, ""s);
		rmvm.AddBaseEntity(rmvm, "union"s, jsonUnion, ""s);
		rmvm.AddBaseEntity(rmvm, "size"s, jsonSize, ""s);
		rmvm.AddBaseEntity(rmvm, "get"s, jsonGet, ""s);
		rmvm.AddBaseEntity(rmvm, "set"s, jsonSet, ""s);
		rmvm.AddBaseEntity(rmvm, "erase"s, jsonErase, "Удаляет элементы, которые соответствуют заданному ключу."s);
		rmvm.AddBaseEntity(rmvm["sequence"], "integer"s, jsonIntegerSequence, ""s);

		//	math
		rmvm.AddBaseEntity(rmvm, "*"s, jsonMul, ""s);
		rmvm.AddBaseEntity(rmvm, ":"s, jsonDiv, "субъект делимое, объект делитель"s);
		rmvm.AddBaseEntity(rmvm, "+", jsonAdd, ""s);
		rmvm.AddBaseEntity(rmvm, "-", jsonSubstract, ""s);
		rmvm.AddBaseEntity(rmvm, "pow"s, jsonPower, ""s);
		rmvm.AddBaseEntity(rmvm, "sqrt"s, jsonSqrt, ""s);
		rmvm.AddBaseEntity(rmvm, "sum"s, jsonSum, ""s);

		//	logic
		rmvm.AddBaseEntity(rmvm, "^"s, jsonXOR, ""s);
		rmvm.AddBaseEntity(rmvm, "=="s, jsonIsEq, ""s);
		rmvm.AddBaseEntity(rmvm, "!="s, jsonIsNotEq, ""s);
		rmvm.AddBaseEntity(rmvm, "<"s, jsonBelow, ""s);
		rmvm.AddBaseEntity(rmvm, "&&"s, jsonAnd, ""s);

		//	strings
		rmvm.AddBaseEntity(rmvm["string"], "="s, string_string, ""s);
		rmvm.AddBaseEntity(rmvm["string"], "+="s, string_add, ""s);
		rmvm.AddBaseEntity(rmvm["string"], "find"s, string_find, ""s);
		rmvm.AddBaseEntity(rmvm["string"], "split"s, string_split, ""s);
		rmvm.AddBaseEntity(rmvm["string"], "join"s, string_join, ""s);

		//	control
		rmvm.AddBaseEntity(rmvm, "foreachobj"s, jsonForEachObject, ""s);
		rmvm.AddBaseEntity(rmvm, "foreachsub"s, jsonForEachSubject, ""s);
		rmvm.AddBaseEntity(rmvm, "then"s, IfObjTrueThenExecSub, ""s);
		rmvm.AddBaseEntity(rmvm, "else"s, IfObjFalseThenExecSub, "");
		rmvm.AddBaseEntity(rmvm, "while"s, ExecSubWhileObjTrue, ""s);
		rmvm.AddBaseEntity(rmvm["switch"], "bool"s, json_switch_bool, ""s);
		rmvm.AddBaseEntity(rmvm["switch"], "number"s, json_switch_number, ""s);
		rmvm.AddBaseEntity(rmvm["switch"], "string"s, json_switch_string, ""s);
		rmvm.AddBaseEntity(rmvm, "throw"s, json_throw, "");
		rmvm.AddBaseEntity(rmvm, "catch"s, json_catch, "Exec $obj in the parent ctx, if an exception is thrown, then it is written to the current view value and exec $sub in the parent ctx");

		//	display
		rmvm.AddBaseEntity(rmvm, "print"s, jsonPrint, ""s);

		//	browser
		rmvm.AddBaseEntity(rmvm, "tag"s, jsonTAG, ""s);
		rmvm.AddBaseEntity(rmvm, "xml"s, jsonXML, ""s);
		rmvm.AddBaseEntity(rmvm, "html"s, jsonHTML, ""s);

		//	steady clock
		rmvm.AddBaseEntity(rmvm["steady_clock"], "nanoseconds"s, steady_clock_<chrono::nanoseconds>, "Sets $res to time since epoch in nanoseconds"s);
		rmvm.AddBaseEntity(rmvm["steady_clock"], "microseconds"s, steady_clock_<chrono::microseconds>, "Sets $res to time since epoch in microseconds"s);
		rmvm.AddBaseEntity(rmvm["steady_clock"], "milliseconds"s, steady_clock_<chrono::milliseconds>, "Sets $res to time since epoch in milliseconds"s);
		rmvm.AddBaseEntity(rmvm["steady_clock"], "seconds"s, steady_clock_<chrono::seconds>, "Sets $res to time since epoch in seconds"s);
		rmvm.AddBaseEntity(rmvm["steady_clock"], "minutes"s, steady_clock_<chrono::minutes>, "Sets $res to time since epoch in minutes"s);
		rmvm.AddBaseEntity(rmvm["steady_clock"], "hours"s, steady_clock_<chrono::hours>, "Sets $res to time since epoch in hours"s);

		return rmvm_version;
	}
}