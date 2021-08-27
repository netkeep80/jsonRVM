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
Copyright (c) 2016-2021 Vertushkin Roman Pavlovich <https://vk.com/earthbirthbook>.

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

	inline json operator ^ (const json& a, const json& b);

	template<typename type_t>
	json diff_operation(const json& a, const json& b)
	{
		type_t res = a.get<type_t>() - b.get<type_t>();
		if (res == type_t()) return json();
		return json(res);
	}

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

	inline json xor_number_float(const json& a, const json& b) { return diff_operation<json::number_float_t>(a, b); }
	inline json xor_number_integer(const json& a, const json& b) { return diff_operation<json::number_integer_t>(a, b); }
	inline json xor_number_unsigned(const json& a, const json& b) { return diff_operation<json::number_unsigned_t>(a, b); }

	inline json xor_object(const json& a, const json& b)
	{
		json obj = json(), field;

		for (auto& it = a.begin(); it != a.end(); ++it)
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

		for (auto& it = b.begin(); it != b.end(); ++it)
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
		case json::value_t::array:
			return xor_array(a, b);
		case json::value_t::boolean:
			return xor_boolean(a, b);
		case json::value_t::number_float:
			return xor_number_float(a, b);
		case json::value_t::number_integer:
			return xor_number_integer(a, b);
		case json::value_t::number_unsigned:
			return xor_number_unsigned(a, b);
		case json::value_t::object:
			return xor_object(a, b);
		case json::value_t::string:
			return xor_string(a, b);
		default:
			return json();
		}
		else if (a.is_array()) return array_xor_value(a, b);
		else if (b.is_array()) return value_xor_array(a, b);
		else return json::array({ a, b });
	}

	/// <summary>
	/// Усыпляет этот поток на $.obj миллисекунд
	/// </summary>
	/// <param name="rmvm">Виртуальная машины</param>
	/// <param name="$">Контекст исполнения</param>
	void  sleep_ms(vm& rmvm, vm_ctx& $) {
		this_thread::sleep_for(chrono::milliseconds($.obj.get<json::number_unsigned_t>()));
	}

	void  jsonCopy(vm& rmvm, vm_ctx& $)
	{	//	полное копированиее json значения объекта в субъект
		$.sub = $.obj;
	}

	void  jsonView(vm& rmvm, vm_ctx& $)
	{	//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
		//	проецируем во внешнем контексте
		rmvm.exec_ent(vm_ctx($.sub, $.$.obj, $.$.sub, $.$.ent, $.$), $.obj);
	}

	void  jsonXOR(vm& rmvm, vm_ctx& $)
	{
		$.rel = $.sub ^ $.obj;
	}

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
	binary,           ///< binary array (ordered collection of bytes)
	discarded         ///< discarded by the parser callback function
};
*/

#define     sub_field       4
#define     obj_field       4

#define OPP_STO(operation,stype,stype_id,otype,otype_id)										\
	case (uint8_t(json::value_t::stype_id) << sub_field) | uint8_t(json::value_t::otype_id):	\
	{ $.rel = ($.sub.get<stype>()) operation ($.obj.get<otype>()); return; }

#define OPP_ANYTO(operation,type,type_id)														\
	OPP_STO(operation, json::number_float_t, number_float,    type, type_id)					\
	OPP_STO(operation, json::number_integer_t, number_integer,  type, type_id)					\
	OPP_STO(operation, json::number_unsigned_t, number_unsigned, type, type_id)

#define VM_OPP(operation)																		\
	OPP_ANYTO(operation, json::number_float_t, number_float)									\
	OPP_ANYTO(operation, json::number_integer_t, number_integer)								\
	OPP_ANYTO(operation, json::number_unsigned_t, number_unsigned)

#define	OP_BODY( name, operation )																\
void  json##name (vm& rmvm, vm_ctx& $)															\
{																								\
	switch( (uint8_t($.sub.type()) << sub_field) | uint8_t($.obj.type()) )						\
	{ VM_OPP( operation ) default: $.rel = json(); }											\
}

	OP_BODY(Add, +);
	OP_BODY(Substract, -);
	OP_BODY(Mul, *);
	OP_BODY(Div, /);

	//////////////////////////////////////////////////////////////////////////////////////////////
	void  jsonPower(vm& rmvm, vm_ctx& $)
	{
		if ($.sub.is_number() && $.obj.is_number())
		{
			json::number_float_t db = $.obj.is_number_float() ? $.obj.get<json::number_float_t>() : json::number_float_t($.obj.get<json::number_integer_t>());

			if (!$.sub.is_number_float())
			{	//	result must be integer
				json::number_float_t da = json::number_float_t($.sub.get<json::number_integer_t>());
				$.rel = json::number_integer_t(pow(da, db) + .5);
			}
			else
			{	//	result must be double
				json::number_float_t da = $.sub.get<json::number_float_t>();
				$.rel = json::number_float_t(pow(da, db));
			}
		}
		else
			$.rel = json();
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
			Множественный exec_ent для проекции объекта типа array
		*/
		if ($.obj.is_array())
		{
			if (!$.rel.is_array()) $.rel = json::array();
			size_t i = 0;
			for (auto& it : $.obj)
			{
				try {
					json& value = $.rel[i];
					rmvm.exec_ent(vm_ctx(value, it, value, $.ent, $.$), $.sub); i++;
				}
				catch (json& j) { $.throw_json(__func__, json({ {"["s + to_string(i) + "]"s, j} })); }
			}
		}
		else
			$.throw_json(__func__, "$obj must be array!"s);
	}

	void  jsonForEachSubject(vm& rmvm, vm_ctx& $)
	{	/*
			Множественный exec_ent для субъекта типа array
		*/
		if ($.sub.is_array())
		{
			if (!$.rel.is_array()) $.rel = json::array();
			size_t i = 0;
			for (auto& it : $.sub)
			{
				try {
					json& value = $.rel[i];
					rmvm.exec_ent(vm_ctx(value, it, value, $.ent, $.$), $.obj); i++;
				}
				catch (json& j) { $.throw_json(__func__, json({ {"["s + to_string(i) + "]"s, j} })); }
			}
		}
		else
			$.throw_json(__func__, "$sub must be array!"s);
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
			$.throw_json(__func__, "$obj must has 'from', 'to' and 'step' properties!"s);
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

		$.rel = $.sub;
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

		$.rel = $.sub;
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
		if (!($.obj.is_string() && $.sub.is_string())) $.throw_json(__func__, "$obj and $sub must be strings!"s);
		$.rel = static_cast<json::number_integer_t>($.obj.get_ref<string const&>().find($.sub.get_ref<string&>().c_str()));
	}


	void  string_split(vm& rmvm, vm_ctx& $)
	{
		if ($.obj.is_string() && $.sub.is_string())
		{
			const string& delim = $.obj.get_ref<string const&>();
			string str = $.sub.get_ref<string&>();
			size_t prev = 0, pos = 0;
			$.rel = json::array();

			do
			{
				pos = str.find(delim, prev);
				if (pos == string::npos) pos = str.length();
				$.rel.push_back(str.substr(prev, pos - prev));
				prev = pos + delim.length();
			} while (pos < str.length() && prev < str.length());
		}
		else
			$.throw_json(__func__, "$obj and $sub must be string!"s);
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
					$.throw_json(__func__, "$sub must be array of simple type values!"s);

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

			$.rel = result;
		}
		else
			$.throw_json(__func__, "$obj must be string and $sub must be array!"s);
	}

	void  jsonGet(vm& rmvm, vm_ctx& $)
	{
		if ($.sub.is_array())
		{
			if ($.obj.is_number())
				$.rel = $.sub[$.obj.get<size_t>()];
			else
				$.throw_json(__func__, "$obj must be unsigned number!"s);
		}
		else if ($.sub.is_object())
		{
			if ($.obj.is_string())
				$.rel = $.sub[$.obj.get_ref<string const&>()];
			else
				$.throw_json(__func__, "$obj must be string!"s);
		}
		else
			$.throw_json(__func__, "$sub must be array or object!"s);
	}

	void  jsonSet(vm& rmvm, vm_ctx& $)
	{
		if ($.sub.is_array())
		{
			if ($.obj.is_number_unsigned())
				$.sub[$.obj.get<size_t>()] = $.rel;
			else
				$.throw_json(__func__, "$obj must be unsigned number!"s);
		}
		else if ($.sub.is_object())
		{
			if ($.obj.is_string())
				$.sub[$.obj.get_ref<string const&>()] = $.rel;
			else
				$.throw_json(__func__, "$obj must be string!"s);
		}
		else if ($.sub.is_null())
		{
			if ($.obj.is_number_unsigned())
				$.sub[$.obj.get<size_t>()] = $.rel;
			else if ($.obj.is_string())
				$.sub[$.obj.get_ref<string const&>()] = $.rel;
			else
				$.throw_json(__func__, "$obj must be unsigned number or string!"s);
		}
		else
			$.throw_json(__func__, "$sub must be array, object or null!"s);
	}

	void  jsonErase(vm& rmvm, vm_ctx& $)
	{
		if ($.sub.is_object())
		{
			if ($.obj.is_string())
				$.sub.erase($.obj.get_ref<string const&>());
			else
				$.throw_json(__func__, "$obj must be string!"s);
		}
		else
			$.throw_json(__func__, "$sub must be object!"s);
	}

	void  jsonIsEq(vm& rmvm, vm_ctx& $)
	{
		$.rel = $.sub == $.obj;
	}

	void  jsonIsNotEq(vm& rmvm, vm_ctx& $)
	{
		$.rel = $.sub != $.obj;
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
			$.throw_json(__func__, "$obj must be json array!"s);
	}

	void  jsonWhere(vm& rmvm, vm_ctx& $)
	{
		$.rel = json::array();		//	подготовка выходного массива

		if ($.obj.is_null()) return;

		if ($.obj.is_array())
		{
			size_t i = 0;
			for (auto& it : $.obj)
			{
				try {
					json	boolres;
					rmvm.exec_ent(vm_ctx(boolres, it, boolres, $.ent, $.$), $.sub);
					if (boolres.is_boolean())
						if (boolres.get<bool>())
							$.rel.push_back(it);	//	фильтруем
					i++;
				}
				catch (json& j) { $.throw_json(__func__, json({ {"["s + to_string(i) + "]"s, j} })); }
			}
		}
		else
			$.throw_json(__func__, "$obj must be json array!"s);
	}

	void  jsonBelow(vm& rmvm, vm_ctx& $)	//	<
	{
		if ($.sub.type() == $.obj.type()) switch ($.obj.type())
		{
		case json::value_t::array:
		case json::value_t::object:
			$.rel = json($.sub.size() < $.obj.size());
			return;

		case json::value_t::string:
			$.rel = json($.sub.get<string>().size() < $.obj.get<string>().size());
			return;

		case json::value_t::boolean:
			$.rel = json(int($.sub.get<bool>()) < int($.obj.get<bool>()));
			return;

		case json::value_t::number_float:
			$.rel = json($.sub.get<json::number_float_t>() < $.obj.get<json::number_float_t>());
			return;

		case json::value_t::number_integer:
			$.rel = json($.sub.get<json::number_integer_t>() < $.obj.get<json::number_integer_t>());
			return;

		case json::value_t::number_unsigned:
			$.rel = json($.sub.get<json::number_unsigned_t>() < $.obj.get<json::number_unsigned_t>());
			return;

		default:
			$.rel = json();
			return;
		}
	}

	void  jsonAnd(vm& rmvm, vm_ctx& $)
	{
		if ($.sub.is_boolean() && $.obj.is_boolean())
			$.rel = $.sub.get<bool>() && $.obj.get<bool>();
		else
			$.throw_json(__func__, "$obj and $sub must be boolean!"s);
	}

	void  IfObjTrueThenExecSub(vm& rmvm, vm_ctx& $)
	{
		if (!$.obj.is_boolean())
			$.throw_json(__func__, "$obj must be boolean!"s);

		try
		{
			if ($.obj.get<bool>())
				rmvm.exec_ent($.$, $.sub);
		}
		catch (json& j) { throw json({ { __func__, j} }); }
	}

	void  IfObjFalseThenExecSub(vm& rmvm, vm_ctx& $)
	{
		if (!$.obj.is_boolean())
			$.throw_json(__func__, "$obj must be boolean!"s);

		try
		{
			if (!$.obj.get<bool>())
				rmvm.exec_ent($.$, $.sub);
		}
		catch (json& j) { throw json({ { __func__, j} }); }
	}

	void  ExecSubWhileObjTrue(vm& rmvm, vm_ctx& $)
	{
		while (true)
		{
			if (!$.obj.is_boolean()) $.throw_json(__func__, "$obj must be boolean!"s);
			if (!$.obj.get<bool>()) return;
			try
			{
				rmvm.exec_ent($.$, $.sub);
			}
			catch (json& j) { throw json({ { __func__, j} }); }
		}
	}

	void  json_switch_bool(vm& rmvm, vm_ctx& $)
	{
		if ($.obj.is_null())
			return;

		if (!$.obj.is_boolean())
			$.throw_json(__func__, "$obj must be boolean!"s);

		if (!$.sub.is_object())
			$.throw_json(__func__, "$sub must be json object!"s);

		try
		{
			if ($.obj.get<bool>())	rmvm.exec_ent($.$, $.sub["true"]);
			else	rmvm.exec_ent($.$, $.sub["false"]);
		}
		catch (json& j) { throw json({ { __func__, j} }); }
		catch (json::exception& e) { $.throw_json(__func__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { $.throw_json(__func__, "std::exception: "s + e.what()); }
		catch (...) { $.throw_json(__func__, "unknown exception"s); }
	}

	void  json_switch_number(vm& rmvm, vm_ctx& $)
	{
		if ($.obj.is_null())
			return;

		if (!($.obj.is_number_unsigned() || $.obj.is_number_integer()))
			$.throw_json(__func__, "$obj must be number!"s);

		if (!$.sub.is_object())
			$.throw_json(__func__, "$sub must be json object!"s);

		try
		{
			string	key;
			if ($.obj.is_number_unsigned())
				key = to_string($.obj.get<json::number_unsigned_t>());
			else if ($.obj.is_number_integer())
				key = to_string($.obj.get<json::number_integer_t>());

			if ($.sub.count(key))
				rmvm.exec_ent($.$, $.sub[key]);
			else
				rmvm.exec_ent($.$, $.sub["default"]);
		}
		catch (json& j) { throw json({ { __func__, j} }); }
		catch (json::exception& e) { $.throw_json(__func__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { $.throw_json(__func__, "std::exception: "s + e.what()); }
		catch (...) { $.throw_json(__func__, "unknown exception"s); }
	}


	void  json_switch_string(vm& rmvm, vm_ctx& $)
	{
		if ($.obj.is_null())
			return;

		if (!$.obj.is_string())
			$.throw_json(__func__, "$obj must be string!"s);

		if (!$.sub.is_object())
			$.throw_json(__func__, "$sub must be json object!"s);

		try
		{
			if ($.sub.count($.obj.get_ref<string const&>())) rmvm.exec_ent($.$, $.sub[$.obj.get_ref<string const&>()]);
			else rmvm.exec_ent($.$, $.sub["default"]);
		}
		catch (json& j) { throw json({ { __func__, j} }); }
		catch (json::exception& e) { $.throw_json(__func__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { $.throw_json(__func__, "std::exception: "s + e.what()); }
		catch (...) { $.throw_json(__func__, "unknown exception"s); }
	}


	void  json_throw(vm& rmvm, vm_ctx& $) { throw $.obj; }


	void  json_catch(vm& rmvm, vm_ctx& $)
	{
		//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
		//	проецируем во внешнем контексте
		try
		{
			rmvm.exec_ent(vm_ctx($.rel, $.$.obj, $.$.sub, $.$.ent, $.$), $.obj);
		}
		catch (json& j)
		{
			$.rel = j;
			rmvm.exec_ent(vm_ctx($.rel, $.$.obj, $.$.sub, $.$.ent, $.$), $.sub);
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
			$.rel = json::##static_method();									\
		}

	define_static_method(array)
		define_static_method(meta)
		define_static_method(object)

		void  json_call_null(vm& rmvm, vm_ctx& $)
	{
		$.rel = json();
	}

	void  jsonPrint(vm& rmvm, vm_ctx& $)
	{
		cout << $.obj.dump(1) << endl;
	}


	void  jsonTAG(vm& rmvm, vm_ctx& $)
	{
		if (!$.sub.is_object())
			$.throw_json(__func__, "$sub must be json object!"s);

		string	tag = $.sub["<>"];

		if (!$.rel.is_string())
			$.rel = ""s;

		string& body = $.rel.get_ref<string&>();
		body += "<" + tag;

		for (auto& it : $.sub.items())
			if ("<>" != it.key())
				body += " " + it.key() + "=" + it.value().dump();

		body += ">";
		//	что бы выходной поток xml попадал в тоже значение $.$rel нужно исполнять в текущем контексitsV
		json	objview;
		rmvm.exec_ent(vm_ctx(objview, $.obj, $.sub, $.ent, $.$), $.obj);
		if (objview.is_string()) body += objview.get_ref<string&>();
		else                     body += objview.dump();
		body += "</" + tag + ">";
	}


	void  jsonXML(vm& rmvm, vm_ctx& $)
	{
		std::string res = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"s;

		if (!$.sub.is_object())
			$.throw_json(__func__, "$sub must be json object!"s);

		string	tag = $.sub["<>"];

		if (!$.rel.is_string())
			$.rel = res;
		else if (!$.rel.get_ref<string&>().size())
			$.rel = res;

		string& body = $.rel.get_ref<string&>();
		body += "<" + tag;

		for (auto& it : $.sub.items())
			if ("<>" != it.key())
				body += " " + it.key() + "=" + it.value().dump();

		body += ">";
		//	что бы выходной поток xml попадал в тоже значение $.$rel нужно исполнять в текущем контексitsV
		json	objview;
		rmvm.exec_ent(vm_ctx(objview, $.obj, $.sub, $.ent, $.$), $.obj);
		if (objview.is_string()) body += objview.get_ref<string&>();
		else                     body += objview.dump();
		body += "</" + tag + ">";
	}


	void  jsonHTML(vm& rmvm, vm_ctx& $)
	{
		std::string res = "<!DOCTYPE html>"s;

		if (!$.sub.is_object())
			$.throw_json(__func__, "$sub must be json object!"s);

		string	tag = $.sub["<>"];

		if (!$.rel.is_string())
			$.rel = res;
		else if (!$.rel.get_ref<string&>().size())
			$.rel = res;

		string& body = $.rel.get_ref<string&>();
		body += "<" + tag;

		for (auto& it : $.sub.items())
			if ("<>" != it.key())
				body += " " + it.key() + "=" + it.value().dump();

		body += ">";
		//	что бы выходной поток xml попадал в тоже значение $.$rel нужно исполнять в текущем контексitsV
		json	objview;
		rmvm.exec_ent(vm_ctx(objview, $.obj, $.sub, $.ent, $.$), $.obj);
		if (objview.is_string()) body += objview.get_ref<string&>();
		else                     body += objview.dump();
		body += "</" + tag + ">";
	}

	struct json_dump : public base_entity {
		const json_pointer<json> path{ "/json/dump" };
		const string description{ "Dump $obj to $sub" };
		static void	view(vm& rmvm, vm_ctx& $) { $.sub = $.obj.dump(3); }
	};


	template<class duration_t, class name_t>
	struct steady_clock : public base_entity {
		const json_pointer<json> path{ "/steady_clock/"s + name_t::str };
		const string description{ "Sets $rel to time since epoch in "s + name_t::str };
		static void	view(vm& rmvm, vm_ctx& $) {
			$.rel = chrono::duration_cast<duration_t>(chrono::steady_clock::now().time_since_epoch()).count();
		}
	};


	const string&	import_relations_model_to(vm& rmvm)
	{
		rmvm.add_base_entity(rmvm["sleep"], "ms"s, sleep_ms, "sleep in milliconds"s);
		rmvm.add_base_entity(rmvm, "view"s, jsonView, "ViewEntity: View object model in parent ctx and then set subject value"s);
		rmvm.add_base_entity(rmvm, "="s, jsonCopy, "Copy: copy object model to subject value"s);

#define map_json_is_type(json_type)	rmvm.add_base_entity( rmvm, "is_"s + #json_type, json_is_##json_type, ""s );
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
#define map_json_static_method(static_method)	rmvm.add_base_entity( rmvm["json"], #static_method, json_call_##static_method, ""s );
		map_json_static_method(array);
		map_json_static_method(null);
		map_json_static_method(meta);
		map_json_static_method(object);

		rmvm << json_dump();

		//	convert
		rmvm.add_base_entity(rmvm, "integer"s, jsonInt32, ""s);
		rmvm.add_base_entity(rmvm, "int"s, jsonInt32, ""s);
		rmvm.add_base_entity(rmvm, "float"s, jsonDouble, ""s);
		rmvm.add_base_entity(rmvm, "double"s, jsonDouble, ""s);
		rmvm.add_base_entity(rmvm, "null"s, jsonNull, ""s);

		//	data operations
		rmvm.add_base_entity(rmvm, "where"s, jsonWhere, ""s);
		rmvm.add_base_entity(rmvm, "union"s, jsonUnion, ""s);
		rmvm.add_base_entity(rmvm, "size"s, jsonSize, ""s);
		rmvm.add_base_entity(rmvm, "get"s, jsonGet, ""s);
		rmvm.add_base_entity(rmvm, "set"s, jsonSet, ""s);
		rmvm.add_base_entity(rmvm, "erase"s, jsonErase, "Удаляет элементы, которые соответствуют заданному ключу."s);
		rmvm.add_base_entity(rmvm["sequence"], "integer"s, jsonIntegerSequence, ""s);

		//	math
		rmvm.add_base_entity(rmvm, "*"s, jsonMul, ""s);
		rmvm.add_base_entity(rmvm, ":"s, jsonDiv, "субъект делимое, объект делитель"s);
		rmvm.add_base_entity(rmvm, "+", jsonAdd, ""s);
		rmvm.add_base_entity(rmvm, "-", jsonSubstract, ""s);
		rmvm.add_base_entity(rmvm, "pow"s, jsonPower, ""s);
		rmvm.add_base_entity(rmvm, "sqrt"s, jsonSqrt, ""s);
		rmvm.add_base_entity(rmvm, "sum"s, jsonSum, ""s);

		//	logic
		rmvm.add_base_entity(rmvm, "^"s, jsonXOR, ""s);
		rmvm.add_base_entity(rmvm, "=="s, jsonIsEq, ""s);
		rmvm.add_base_entity(rmvm, "!="s, jsonIsNotEq, ""s);
		rmvm.add_base_entity(rmvm, "<"s, jsonBelow, ""s);
		rmvm.add_base_entity(rmvm, "&&"s, jsonAnd, ""s);

		//	strings
		rmvm.add_base_entity(rmvm["string"], "="s, string_string, ""s);
		rmvm.add_base_entity(rmvm["string"], "+="s, string_add, ""s);
		rmvm.add_base_entity(rmvm["string"], "find"s, string_find, ""s);
		rmvm.add_base_entity(rmvm["string"], "split"s, string_split, ""s);
		rmvm.add_base_entity(rmvm["string"], "join"s, string_join, ""s);

		//	control
		rmvm.add_base_entity(rmvm, "foreachobj"s, jsonForEachObject, ""s);
		rmvm.add_base_entity(rmvm, "foreachsub"s, jsonForEachSubject, ""s);
		rmvm.add_base_entity(rmvm, "then"s, IfObjTrueThenExecSub, ""s);
		rmvm.add_base_entity(rmvm, "else"s, IfObjFalseThenExecSub, "");
		rmvm.add_base_entity(rmvm, "while"s, ExecSubWhileObjTrue, ""s);
		rmvm.add_base_entity(rmvm["switch"], "bool"s, json_switch_bool, ""s);
		rmvm.add_base_entity(rmvm["switch"], "number"s, json_switch_number, ""s);
		rmvm.add_base_entity(rmvm["switch"], "string"s, json_switch_string, ""s);
		rmvm.add_base_entity(rmvm, "throw"s, json_throw, "");
		rmvm.add_base_entity(rmvm, "catch"s, json_catch, "Exec $obj in the parent ctx, if an exception is thrown, then it is written to the current view value and exec $sub in the parent ctx");

		//	display
		rmvm.add_base_entity(rmvm, "print"s, jsonPrint, ""s);

		//	browser
		rmvm.add_base_entity(rmvm, "tag"s, jsonTAG, ""s);
		rmvm.add_base_entity(rmvm, "xml"s, jsonXML, ""s);
		rmvm.add_base_entity(rmvm, "html"s, jsonHTML, ""s);

		//	steady clock
		rmvm << steady_clock<chrono::nanoseconds, as_type("nanoseconds")>()	<< steady_clock<chrono::microseconds, as_type("microseconds")>()
			<< steady_clock<chrono::milliseconds, as_type("milliseconds")>() << steady_clock<chrono::seconds, as_type("seconds")>()
			<< steady_clock<chrono::minutes, as_type("minutes")>() << steady_clock<chrono::hours, as_type("hours")>();

		return vm::version;
	}
}