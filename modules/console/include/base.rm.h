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
#include "vm.rm.h"

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
		json $.obj;

		for (auto it = a.begin(); it != a.end(); it++)
		{
			const string& key = it.key();
			if (b.count(key)) $.obj[key] = _OP(it.value(), b[key]);
		}
		return $.obj;
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

	void  sleep_ms(vm& rmvm, vm_ctx& $)
	{
		this_thread::sleep_for(chrono::milliseconds($.obj.get<json::number_unsigned_t>()));
	}

	void  jsonCopy(vm& rmvm, vm_ctx& $)
	{	//	полное копированиее json значения объекта в субъект
		$.sub = $.obj;
	}

	void  jsonView(vm& rmvm, vm_ctx& $)
	{	//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
		//	проецируем во внешнем контексте
		rmvm.exec(vm_ctx($.sub, $.$.obj, $.$.sub, $.$.ent, $.$), $.obj);
	}

	void  jsonXOR(vm& rmvm, vm_ctx& $)
	{
		$.its = $.sub ^ $.obj;
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
	{ $.its = ($.sub.get<stype>()) operation ($.obj.get<otype>()); return; }

#define OPP_ANYTO(operation,type,type_id)														\
	OPP_STO(operation, jf, number_float,    type, type_id)										\
	OPP_STO(operation, ji, number_integer,  type, type_id)										\
	OPP_STO(operation, ju, number_unsigned, type, type_id)

#define VM_OPP(operation)																		\
	OPP_ANYTO(operation, jf, number_float)														\
	OPP_ANYTO(operation, ji, number_integer)													\
	OPP_ANYTO(operation, ju, number_unsigned)

#define	OP_BODY( name, operation )																\
void  json##name (vm& rmvm, vm_ctx& $)			\
{																								\
	switch( (uint8_t($.sub.type()) << sub_field) | uint8_t($.obj.type()) )							\
	{ VM_OPP( operation ) default: $.its = json(); }												\
}

	OP_BODY(Add, +);
	OP_BODY(Substract, -);
	OP_BODY(Mul, *);

	void  jsonDiv(vm& rmvm, vm_ctx& $)
	{
		if ($.obj.is_number())
			if ($.obj.get<double>() == 0.0)
			{
				$.its = json();
				return;
			}
		switch ((uint8_t($.sub.type()) << sub_field) | uint8_t($.obj.type()))
		{
			VM_OPP(/ );
			default: $.its = json();
		}
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	void  jsonPower(vm& rmvm, vm_ctx& $)
	{
		if ($.sub.is_number() && $.obj.is_number())
		{
			json::number_float_t db = $.obj.is_number_float() ? $.obj.get<json::number_float_t>() : json::number_float_t($.obj.get<json::number_integer_t>());

			if (!$.sub.is_number_float())
			{	//	result must be integer
				json::number_float_t da = json::number_float_t($.sub.get<json::number_integer_t>());
				$.its = json::number_integer_t(pow(da, db) + .5);
			}
			else
			{	//	result must be double
				json::number_float_t da = $.sub.get<json::number_float_t>();
				$.its = json::number_float_t(pow(da, db));
			}
		}
		else
			$.its = json();
	}

	void  jsonSqrt(vm& rmvm, vm_ctx& $)
	{
		if ($.obj.is_number())
			$.sub = json::number_float_t(sqrt($.obj.get<json::number_float_t>()));
		else
			$.sub = json();
	}

	////////////////////////////////////////////////////////////////////////////////////


	void  jsonForEachObject(vm& rmvm, vm_ctx& $)
	{	/*
			Множественный exec для проекции объекта типа array
		*/
		if ($.obj.is_array())
		{
			if (!$.its.is_array()) $.its = json::array();
			size_t i = 0;
			for (auto& it : $.obj)
			{
				try {
					json& value = $.its[i];
					rmvm.exec(vm_ctx(value, it, value, $.ent, $.$), $.sub); i++;
				}
				catch (json& j) { $.throw_json(__FUNCTION__, json({ {"["s + to_string(i) + "]"s, j} })); }
			}
		}
		else
			$.throw_json(__FUNCTION__, "$obj must be array!"s);
	}

	void  jsonForEachSubject(vm& rmvm, vm_ctx& $)
	{	/*
			Множественный exec для субъекта типа array
		*/
		if ($.sub.is_array())
		{
			if (!$.its.is_array()) $.its = json::array();
			size_t i = 0;
			for (auto& it : $.sub)
			{
				try {
					json& value = $.its[i];
					rmvm.exec(vm_ctx(value, it, value, $.ent, $.$), $.obj); i++;
				}
				catch (json& j) { $.throw_json(__FUNCTION__, json({ {"["s + to_string(i) + "]"s, j} })); }
			}
		}
		else
			$.throw_json(__FUNCTION__, "$sub must be array!"s);
	}

	void  jsonSize(vm& rmvm, vm_ctx& $)
	{
		$.sub = $.obj.size();
	}

#define define_json_is_type(json_type)							\
	void  json_is_##json_type(vm& rmvm, vm_ctx& $)		\
	{															\
		$.sub = $.obj.is_##json_type();						\
	}

	void  json_is_not_null(vm& rmvm, vm_ctx& $)
	{
		$.sub = !$.obj.is_null();
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

		void  jsonIntegerSequence(vm& rmvm, vm_ctx& $)
	{
		if ($.obj.is_object())
		{
			json::number_integer_t	from = $.obj["from"];
			json::number_integer_t	to = $.obj["to"];
			json::number_integer_t	step = $.obj.count("step") ? $.obj["step"] : 1;
			$.sub = json::array();

			for (json::number_integer_t i = from; i <= to; i += step)
				$.sub.push_back(i);
		}
		else
			$.throw_json(__FUNCTION__, "$obj must has 'from', 'to' and 'step' properties!"s);
	}

	void  jsonUnion(vm& rmvm, vm_ctx& $)
	{
		if (!$.sub.is_array())
		{
			json	tmp = $.sub;
			$.sub = json::array();
			if (!tmp.is_null())
				$.sub[0] = tmp;
		}

		if (!$.obj.is_array())
		{
			if (!$.obj.is_null())
				$.sub.push_back($.obj);
		}
		else
			for (auto& it : $.obj)
				$.sub.push_back(it);

		$.its = $.sub;
	}

	void  jsonNull(vm& rmvm, vm_ctx& $) {}

	void  jsonInt32(vm& rmvm, vm_ctx& $)
	{
		switch ($.obj.type())
		{
		case json::value_t::array:
		case json::value_t::object:
			$.sub = $.obj.size();
			break;

		case json::value_t::string:
			$.sub = atoi($.obj.get<string>().c_str());
			break;

		case json::value_t::boolean:
			if ($.obj.get<bool>())
				$.sub = int(1);
			else
				$.sub = int(0);
			break;

		case json::value_t::number_float:
		case json::value_t::number_integer:
		case json::value_t::number_unsigned:
			$.sub = $.obj.get<int>();
			break;

		default:
			$.sub = int(0);
		}

		$.its = $.sub;
	}

	void  jsonDouble(vm& rmvm, vm_ctx& $)
	{
		switch ($.obj.type())
		{
		case json::value_t::array:
		case json::value_t::object:
			$.sub = json::number_float_t($.obj.size());
			break;

		case json::value_t::string:
			$.sub = json::number_float_t(std::stod($.obj.get_ref<string const&>()));
			break;

		case json::value_t::boolean:
			if ($.obj.get<bool>())
				$.sub = json::number_float_t(1.0);
			else
				$.sub = json::number_float_t(0.0);
			break;

		case json::value_t::number_float:
			$.sub = $.obj;
			break;

		case json::value_t::number_integer:
			$.sub = json::number_float_t($.obj.get<json::number_integer_t>());
			break;

		case json::value_t::number_unsigned:
			$.sub = json::number_float_t($.obj.get<json::number_unsigned_t>());
			break;

		default:
			$.sub = json::number_float_t(0);
		}
	}


	void  string_string(vm& rmvm, vm_ctx& $)
	{
		$.sub = ""s;
		string& result = $.sub.get_ref<string&>();

		switch ($.obj.type())
		{
		case json::value_t::number_float:
			result = to_string($.obj.get<json::number_float_t>());
			break;

		case json::value_t::number_integer:
			result = to_string($.obj.get<json::number_integer_t>());
			break;

		case json::value_t::number_unsigned:
			result = to_string($.obj.get<json::number_unsigned_t>());
			break;

		case json::value_t::string:
			result = $.obj.get_ref<string const&>();
			break;

		default:
			result = $.obj.dump();
			break;
		}
	}


	void  string_add(vm& rmvm, vm_ctx& $)
	{
		if (!$.sub.is_string())	$.sub = ""s;
		string& result = $.sub.get_ref<string&>();

		switch ($.obj.type())
		{
		case json::value_t::number_float:
			result += to_string($.obj.get<json::number_float_t>());
			break;

		case json::value_t::number_integer:
			result += to_string($.obj.get<json::number_integer_t>());
			break;

		case json::value_t::number_unsigned:
			result += to_string($.obj.get<json::number_unsigned_t>());
			break;

		case json::value_t::string:
			result += $.obj.get_ref<string const&>();
			break;

		default:
			result += $.obj.dump();
			break;
		}
	}


	void  string_find(vm& rmvm, vm_ctx& $)
	{
		if (!($.obj.is_string() && $.sub.is_string())) $.throw_json(__FUNCTION__, "$obj and $sub must be strings!"s);
		$.its = static_cast<json::number_integer_t>($.obj.get_ref<string const&>().find($.sub.get_ref<string&>().c_str()));
	}


	void  string_split(vm& rmvm, vm_ctx& $)
	{
		if ($.obj.is_string() && $.sub.is_string())
		{
			const string& delim = $.obj.get_ref<string const&>();
			string str = $.sub.get_ref<string&>();
			size_t prev = 0, pos = 0;
			$.its = json::array();

			do
			{
				pos = str.find(delim, prev);
				if (pos == string::npos) pos = str.length();
				$.its.push_back(str.substr(prev, pos - prev));
				prev = pos + delim.length();
			} while (pos < str.length() && prev < str.length());
		}
		else
			$.throw_json(__FUNCTION__, "$obj and $sub must be string!"s);
	}

	void  string_join(vm& rmvm, vm_ctx& $)
	{
		string	result = "";
		bool	first = true;

		if ($.obj.is_string() && $.sub.is_array())
		{
			auto& splitter = $.obj.get_ref<string const&>();

			for (auto& it : $.sub)
			{
				if (first) first = false;
				else       result += splitter;

				switch (it.type())
				{
				case json::value_t::object:
				case json::value_t::array:
					$.throw_json(__FUNCTION__, "$sub must be array of simple type values!"s);

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

			$.its = result;
		}
		else
			$.throw_json(__FUNCTION__, "$obj must be string and $sub must be array!"s);
	}

	void  jsonGet(vm& rmvm, vm_ctx& $)
	{
		if ($.sub.is_array())
		{
			if ($.obj.is_number())
				$.its = $.sub[$.obj.get<size_t>()];
			else
				$.throw_json(__FUNCTION__, "$obj must be unsigned number!"s);
		}
		else if ($.sub.is_object())
		{
			if ($.obj.is_string())
				$.its = $.sub[$.obj.get_ref<string const&>()];
			else
				$.throw_json(__FUNCTION__, "$obj must be string!"s);
		}
		else
			$.throw_json(__FUNCTION__, "$sub must be array or object!"s);
	}

	void  jsonSet(vm& rmvm, vm_ctx& $)
	{
		if ($.sub.is_array())
		{
			if ($.obj.is_number_unsigned())
				$.sub[$.obj.get<size_t>()] = $.its;
			else
				$.throw_json(__FUNCTION__, "$obj must be unsigned number!"s);
		}
		else if ($.sub.is_object())
		{
			if ($.obj.is_string())
				$.sub[$.obj.get_ref<string const&>()] = $.its;
			else
				$.throw_json(__FUNCTION__, "$obj must be string!"s);
		}
		else if ($.sub.is_null())
		{
			if ($.obj.is_number_unsigned())
				$.sub[$.obj.get<size_t>()] = $.its;
			else if ($.obj.is_string())
				$.sub[$.obj.get_ref<string const&>()] = $.its;
			else
				$.throw_json(__FUNCTION__, "$obj must be unsigned number or string!"s);
		}
		else
			$.throw_json(__FUNCTION__, "$sub must be array, object or null!"s);
	}

	void  jsonErase(vm& rmvm, vm_ctx& $)
	{
		if ($.sub.is_object())
		{
			if ($.obj.is_string())
				$.sub.erase($.obj.get_ref<string const&>());
			else
				$.throw_json(__FUNCTION__, "$obj must be string!"s);
		}
		else
			$.throw_json(__FUNCTION__, "$sub must be object!"s);
	}

	void  jsonIsEq(vm& rmvm, vm_ctx& $)
	{
		$.its = $.sub == $.obj;
	}

	void  jsonIsNotEq(vm& rmvm, vm_ctx& $)
	{
		$.its = $.sub != $.obj;
	}

	void  jsonSum(vm& rmvm, vm_ctx& $)
	{
		__int64	isum = 0;
		double	dsum = 0.0;

		if ($.obj.is_array())
		{
			for (auto& it : $.obj) switch (it.type())
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

			if (0.0 == dsum)	$.sub = json::number_integer_t(isum);
			else if (0 == isum)	$.sub = json::number_float_t(dsum);
			else				$.sub = json::number_float_t(dsum + json::number_float_t(isum));
			return;
		}
		else
			$.throw_json(__FUNCTION__, "$obj must be json array!"s);
	}

	void  jsonWhere(vm& rmvm, vm_ctx& $)
	{
		$.its = json::array();		//	подготовка выходного массива

		if ($.obj.is_null()) return;

		if ($.obj.is_array())
		{
			size_t i = 0;
			for (auto& it : $.obj)
			{
				try {
					json	boolres;
					rmvm.exec(vm_ctx(boolres, it, boolres, $.ent, $.$), $.sub);
					if (boolres.is_boolean())
						if (boolres.get<bool>())
							$.its.push_back(it);	//	фильтруем
					i++;
				}
				catch (json& j) { $.throw_json(__FUNCTION__, json({ {"["s + to_string(i) + "]"s, j} })); }
			}
		}
		else
			$.throw_json(__FUNCTION__, "$obj must be json array!"s);
	}

	void  jsonBelow(vm& rmvm, vm_ctx& $)	//	<
	{
		if ($.sub.type() == $.obj.type()) switch ($.obj.type())
		{
		case json::value_t::array:
		case json::value_t::object:
			$.its = json($.sub.size() < $.obj.size());
			return;

		case json::value_t::string:
			$.its = json($.sub.get<string>().size() < $.obj.get<string>().size());
			return;

		case json::value_t::boolean:
			$.its = json(int($.sub.get<bool>()) < int($.obj.get<bool>()));
			return;

		case json::value_t::number_float:
			$.its = json($.sub.get<json::number_float_t>() < $.obj.get<json::number_float_t>());
			return;

		case json::value_t::number_integer:
			$.its = json($.sub.get<json::number_integer_t>() < $.obj.get<json::number_integer_t>());
			return;

		case json::value_t::number_unsigned:
			$.its = json($.sub.get<json::number_unsigned_t>() < $.obj.get<json::number_unsigned_t>());
			return;

		default:
			$.its = json();
			return;
		}
	}

	void  jsonAnd(vm& rmvm, vm_ctx& $)
	{
		if ($.sub.is_boolean() && $.obj.is_boolean())
			$.its = $.sub.get<bool>() && $.obj.get<bool>();
		else
			$.throw_json(__FUNCTION__, "$obj and $sub must be boolean!"s);
	}

	void  IfObjTrueThenExecSub(vm& rmvm, vm_ctx& $)
	{
		if (!$.obj.is_boolean())
			$.throw_json(__FUNCTION__, "$obj must be boolean!"s);

		try
		{
			if ($.obj.get<bool>())
				rmvm.exec($.$, $.sub);
		}
		catch (json& j) { throw json({ { __FUNCTION__, j} }); }
	}

	void  IfObjFalseThenExecSub(vm& rmvm, vm_ctx& $)
	{
		if (!$.obj.is_boolean())
			$.throw_json(__FUNCTION__, "$obj must be boolean!"s);

		try
		{
			if (!$.obj.get<bool>())
				rmvm.exec($.$, $.sub);
		}
		catch (json& j) { throw json({ { __FUNCTION__, j} }); }
	}

	void  ExecSubWhileObjTrue(vm& rmvm, vm_ctx& $)
	{
		while (true)
		{
			if (!$.obj.is_boolean()) $.throw_json(__FUNCTION__, "$obj must be boolean!"s);
			if (!$.obj.get<bool>()) return;
			try
			{
				rmvm.exec($.$, $.sub);
			}
			catch (json& j) { throw json({ { __FUNCTION__, j} }); }
		}
	}

	void  json_switch_bool(vm& rmvm, vm_ctx& $)
	{
		if ($.obj.is_null())
			return;

		if (!$.obj.is_boolean())
			$.throw_json(__FUNCTION__, "$obj must be boolean!"s);

		if (!$.sub.is_object())
			$.throw_json(__FUNCTION__, "$sub must be json object!"s);

		try
		{
			if ($.obj.get<bool>())	rmvm.exec($.$, $.sub["true"]);
			else	rmvm.exec($.$, $.sub["false"]);
		}
		catch (json& j) { throw json({ { __FUNCTION__, j} }); }
		catch (json::exception& e) { $.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { $.throw_json(__FUNCTION__, "std::exception: "s + e.what()); }
		catch (...) { $.throw_json(__FUNCTION__, "unknown exception"s); }
	}

	void  json_switch_number(vm& rmvm, vm_ctx& $)
	{
		if ($.obj.is_null())
			return;

		if (!($.obj.is_number_unsigned() || $.obj.is_number_integer()))
			$.throw_json(__FUNCTION__, "$obj must be number!"s);

		if (!$.sub.is_object())
			$.throw_json(__FUNCTION__, "$sub must be json object!"s);

		try
		{
			string	key;
			if ($.obj.is_number_unsigned())
				key = to_string($.obj.get<json::number_unsigned_t>());
			else if ($.obj.is_number_integer())
				key = to_string($.obj.get<json::number_integer_t>());

			if ($.sub.count(key)) rmvm.exec($.$, $.sub[key]);
			else rmvm.exec($.$, $.sub["default"]);
		}
		catch (json& j) { throw json({ { __FUNCTION__, j} }); }
		catch (json::exception& e) { $.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { $.throw_json(__FUNCTION__, "std::exception: "s + e.what()); }
		catch (...) { $.throw_json(__FUNCTION__, "unknown exception"s); }
	}


	void  json_switch_string(vm& rmvm, vm_ctx& $)
	{
		if ($.obj.is_null())
			return;

		if (!$.obj.is_string())
			$.throw_json(__FUNCTION__, "$obj must be string!"s);

		if (!$.sub.is_object())
			$.throw_json(__FUNCTION__, "$sub must be json object!"s);

		try
		{
			if ($.sub.count($.obj.get_ref<string const&>())) rmvm.exec($.$, $.sub[$.obj.get_ref<string const&>()]);
			else rmvm.exec($.$, $.sub["default"]);
		}
		catch (json& j) { throw json({ { __FUNCTION__, j} }); }
		catch (json::exception& e) { $.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { $.throw_json(__FUNCTION__, "std::exception: "s + e.what()); }
		catch (...) { $.throw_json(__FUNCTION__, "unknown exception"s); }
	}


	void  json_throw(vm& rmvm, vm_ctx& $) { throw $.obj; }


	void  json_catch(vm& rmvm, vm_ctx& $)
	{
		//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
		//	проецируем во внешнем контексте
		try
		{
			rmvm.exec(vm_ctx($.its, $.$.obj, $.$.sub, $.$.ent, $.$), $.obj);
		}
		catch (json& j)
		{
			$.its = j;
			rmvm.exec(vm_ctx($.its, $.$.obj, $.$.sub, $.$.ent, $.$), $.sub);
		}
	}


#define define_object_method(object_method)										\
		void  json_call_##object_method(vm& rmvm, vm_ctx& $)	\
		{																		\
			$.sub = $.obj.##object_method();									\
		}

#define define_static_method(static_method)										\
		void  json_call_##static_method(vm& rmvm, vm_ctx& $)	\
		{																		\
			$.its = json::##static_method();									\
		}

	define_static_method(array)
		define_static_method(meta)
		define_static_method(object)

		void  json_call_null(vm& rmvm, vm_ctx& $)
	{
		$.its = json();
	}

	void  jsonPrint(vm& rmvm, vm_ctx& $)
	{
		cout << $.obj.dump(1) << endl;
	}


	void  jsonTAG(vm& rmvm, vm_ctx& $)
	{
		if (!$.sub.is_object())
			$.throw_json(__FUNCTION__, "$sub must be json object!"s);

		string	tag = $.sub["<>"];

		if (!$.its.is_string())
			$.its = ""s;

		string& body = $.its.get_ref<string&>();
		body += "<" + tag;

		for (auto& it : $.sub.items())
			if ("<>" != it.key())
				body += " " + it.key() + "=" + it.value().dump();

		body += ">";
		//	что бы выходной поток xml попадал в тоже значение $.$its нужно исполнять в текущем контексitsV
		json	objview;
		rmvm.exec(vm_ctx(objview, $.obj, $.sub, $.ent, $.$), $.obj);
		if (objview.is_string()) body += objview.get_ref<string&>();
		else                     body += objview.dump();
		body += "</" + tag + ">";
	}


	void  jsonXML(vm& rmvm, vm_ctx& $)
	{
		std::string res = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"s;

		if (!$.sub.is_object())
			$.throw_json(__FUNCTION__, "$sub must be json object!"s);

		string	tag = $.sub["<>"];

		if (!$.its.is_string())
			$.its = res;
		else if (!$.its.get_ref<string&>().size())
			$.its = res;

		string& body = $.its.get_ref<string&>();
		body += "<" + tag;

		for (auto& it : $.sub.items())
			if ("<>" != it.key())
				body += " " + it.key() + "=" + it.value().dump();

		body += ">";
		//	что бы выходной поток xml попадал в тоже значение $.$its нужно исполнять в текущем контексitsV
		json	objview;
		rmvm.exec(vm_ctx(objview, $.obj, $.sub, $.ent, $.$), $.obj);
		if (objview.is_string()) body += objview.get_ref<string&>();
		else                     body += objview.dump();
		body += "</" + tag + ">";
	}


	void  jsonHTML(vm& rmvm, vm_ctx& $)
	{
		std::string res = "<!DOCTYPE html>"s;

		if (!$.sub.is_object())
			$.throw_json(__FUNCTION__, "$sub must be json object!"s);

		string	tag = $.sub["<>"];

		if (!$.its.is_string())
			$.its = res;
		else if (!$.its.get_ref<string&>().size())
			$.its = res;

		string& body = $.its.get_ref<string&>();
		body += "<" + tag;

		for (auto& it : $.sub.items())
			if ("<>" != it.key())
				body += " " + it.key() + "=" + it.value().dump();

		body += ">";
		//	что бы выходной поток xml попадал в тоже значение $.$its нужно исполнять в текущем контексitsV
		json	objview;
		rmvm.exec(vm_ctx(objview, $.obj, $.sub, $.ent, $.$), $.obj);
		if (objview.is_string()) body += objview.get_ref<string&>();
		else                     body += objview.dump();
		body += "</" + tag + ">";
	}

	void  jsonDump(vm& rmvm, vm_ctx& $)
	{
		$.sub = $.obj.dump(3);
	}

	template<class duration>
	void steady_clock_(vm& rmvm, vm_ctx& $) {
		$.its = chrono::duration_cast<duration>(chrono::steady_clock::now().time_since_epoch()).count();
	}

	const string&	ImportRelationsModel(vm& rmvm)
	{
		import_help(rmvm);
		rmvm["rmvm"]["version"] = rmvm_version;
		rmvm.add_binary_view(rmvm["sleep"], "ms"s, sleep_ms, "sleep in milliconds"s);
		rmvm.add_binary_view(rmvm, "view"s, jsonView, "ViewEntity: View object model in parent ctx and then set subject value"s);
		rmvm.add_binary_view(rmvm, "="s, jsonCopy, "Copy: copy object model to subject value"s);

#define map_json_is_type(json_type)	rmvm.add_binary_view( rmvm, "is_"s + #json_type, json_is_##json_type, ""s );
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
#define map_json_static_method(static_method)	rmvm.add_binary_view( rmvm["json"], #static_method, json_call_##static_method, ""s );
		map_json_static_method(array);
		map_json_static_method(null);
		map_json_static_method(meta);
		map_json_static_method(object);

		rmvm.add_binary_view(rmvm["json"], "dump"s, jsonDump, ""s);

		//	convert
		rmvm.add_binary_view(rmvm, "integer"s, jsonInt32, ""s);
		rmvm.add_binary_view(rmvm, "int"s, jsonInt32, ""s);
		rmvm.add_binary_view(rmvm, "float"s, jsonDouble, ""s);
		rmvm.add_binary_view(rmvm, "double"s, jsonDouble, ""s);
		rmvm.add_binary_view(rmvm, "null"s, jsonNull, ""s);

		//	data operations
		rmvm.add_binary_view(rmvm, "where"s, jsonWhere, ""s);
		rmvm.add_binary_view(rmvm, "union"s, jsonUnion, ""s);
		rmvm.add_binary_view(rmvm, "size"s, jsonSize, ""s);
		rmvm.add_binary_view(rmvm, "get"s, jsonGet, ""s);
		rmvm.add_binary_view(rmvm, "set"s, jsonSet, ""s);
		rmvm.add_binary_view(rmvm, "erase"s, jsonErase, "Удаляет элементы, которые соответствуют заданному ключу."s);
		rmvm.add_binary_view(rmvm["sequence"], "integer"s, jsonIntegerSequence, ""s);

		//	math
		rmvm.add_binary_view(rmvm, "*"s, jsonMul, ""s);
		rmvm.add_binary_view(rmvm, ":"s, jsonDiv, "субъект делимое, объект делитель"s);
		rmvm.add_binary_view(rmvm, "+", jsonAdd, ""s);
		rmvm.add_binary_view(rmvm, "-", jsonSubstract, ""s);
		rmvm.add_binary_view(rmvm, "pow"s, jsonPower, ""s);
		rmvm.add_binary_view(rmvm, "sqrt"s, jsonSqrt, ""s);
		rmvm.add_binary_view(rmvm, "sum"s, jsonSum, ""s);

		//	logic
		rmvm.add_binary_view(rmvm, "^"s, jsonXOR, ""s);
		rmvm.add_binary_view(rmvm, "=="s, jsonIsEq, ""s);
		rmvm.add_binary_view(rmvm, "!="s, jsonIsNotEq, ""s);
		rmvm.add_binary_view(rmvm, "<"s, jsonBelow, ""s);
		rmvm.add_binary_view(rmvm, "&&"s, jsonAnd, ""s);

		//	strings
		rmvm.add_binary_view(rmvm["string"], "="s, string_string, ""s);
		rmvm.add_binary_view(rmvm["string"], "+="s, string_add, ""s);
		rmvm.add_binary_view(rmvm["string"], "find"s, string_find, ""s);
		rmvm.add_binary_view(rmvm["string"], "split"s, string_split, ""s);
		rmvm.add_binary_view(rmvm["string"], "join"s, string_join, ""s);

		//	control
		rmvm.add_binary_view(rmvm, "foreachobj"s, jsonForEachObject, ""s);
		rmvm.add_binary_view(rmvm, "foreachsub"s, jsonForEachSubject, ""s);
		rmvm.add_binary_view(rmvm, "then"s, IfObjTrueThenExecSub, ""s);
		rmvm.add_binary_view(rmvm, "else"s, IfObjFalseThenExecSub, "");
		rmvm.add_binary_view(rmvm, "while"s, ExecSubWhileObjTrue, ""s);
		rmvm.add_binary_view(rmvm["switch"], "bool"s, json_switch_bool, ""s);
		rmvm.add_binary_view(rmvm["switch"], "number"s, json_switch_number, ""s);
		rmvm.add_binary_view(rmvm["switch"], "string"s, json_switch_string, ""s);
		rmvm.add_binary_view(rmvm, "throw"s, json_throw, "");
		rmvm.add_binary_view(rmvm, "catch"s, json_catch, "Exec $obj in the parent ctx, if an exception is thrown, then it is written to the current view value and exec $sub in the parent ctx");

		//	display
		rmvm.add_binary_view(rmvm, "print"s, jsonPrint, ""s);

		//	browser
		rmvm.add_binary_view(rmvm, "tag"s, jsonTAG, ""s);
		rmvm.add_binary_view(rmvm, "xml"s, jsonXML, ""s);
		rmvm.add_binary_view(rmvm, "html"s, jsonHTML, ""s);

		//	steady clock
		rmvm.add_binary_view(rmvm["steady_clock"], "nanoseconds"s, steady_clock_<chrono::nanoseconds>, "Sets $res to time since epoch in nanoseconds"s);
		rmvm.add_binary_view(rmvm["steady_clock"], "microseconds"s, steady_clock_<chrono::microseconds>, "Sets $res to time since epoch in microseconds"s);
		rmvm.add_binary_view(rmvm["steady_clock"], "milliseconds"s, steady_clock_<chrono::milliseconds>, "Sets $res to time since epoch in milliseconds"s);
		rmvm.add_binary_view(rmvm["steady_clock"], "seconds"s, steady_clock_<chrono::seconds>, "Sets $res to time since epoch in seconds"s);
		rmvm.add_binary_view(rmvm["steady_clock"], "minutes"s, steady_clock_<chrono::minutes>, "Sets $res to time since epoch in minutes"s);
		rmvm.add_binary_view(rmvm["steady_clock"], "hours"s, steady_clock_<chrono::hours>, "Sets $res to time since epoch in hours"s);

		return rmvm_version;
	}
}