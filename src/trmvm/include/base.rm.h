#pragma once
#include <iostream>
#include <chrono>
#include "JsonRVM.h"
#include "windows.h"

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

	void  sleep_ms(jsonRVM& rvm, EntContext& ec)
	{
		Sleep(ec.obj.get<json::number_unsigned_t>());
	}

	void  jsonCopy(jsonRVM& rvm, EntContext& ec)
	{	//	полное копированиее json значения объекта в субъект
		ec.sub = ec.obj;
	}

	void  jsonView(jsonRVM& rvm, EntContext& ec)
	{	//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
		//	проецируем во внешнем контексте
		rvm.JSONExec(EntContext(ec.sub, ec.ctx.obj, ec.ctx.sub, ec.ctx.ent, ec.ctx), ec.obj);
	}

	void  jsonXOR(jsonRVM& rvm, EntContext& ec)
	{
		ec.res = ec.sub ^ ec.obj;
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
	{ ec.res = (ec.sub.get_ref<stype&>()) operation (ec.obj.get_ref<otype&>()); return; }

#define OPP_ANYTO(operation,type,type_id)														\
	OPP_STO(operation, jf, number_float,    type, type_id)										\
	OPP_STO(operation, ji, number_integer,  type, type_id)										\
	OPP_STO(operation, ju, number_unsigned, type, type_id)

#define VM_OPP(operation)																		\
	OPP_ANYTO(operation, jf, number_float)														\
	OPP_ANYTO(operation, ji, number_integer)													\
	OPP_ANYTO(operation, ju, number_unsigned)

#define	OP_BODY( name, operation )																\
void  json##name (jsonRVM& rvm, EntContext& ec)			\
{																								\
	switch( (uint8_t(ec.sub.type()) << sub_field) | uint8_t(ec.obj.type()) )							\
	{ VM_OPP( operation ) default: ec.res = json(); }												\
}

	OP_BODY(Add, +);
	OP_BODY(Substract, -);
	OP_BODY(Mul, *);

	void  jsonDiv(jsonRVM& rvm, EntContext& ec)
	{
		if (ec.obj.is_number())
			if (ec.obj.get<double>() == 0.0)
			{
				ec.res = json();
				return;
			}
		switch ((uint8_t(ec.sub.type()) << sub_field) | uint8_t(ec.obj.type()))
		{
		VM_OPP(/ ) default: ec.res = json();
		}
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	void  jsonPower(jsonRVM& rvm, EntContext& ec)
	{
		if (ec.sub.is_number() && ec.obj.is_number())
		{
			json::number_float_t db = ec.obj.is_number_float() ? ec.obj.get<json::number_float_t>() : json::number_float_t(ec.obj.get<json::number_integer_t>());

			if (!ec.sub.is_number_float())
			{	//	result must be integer
				json::number_float_t da = json::number_float_t(ec.sub.get<json::number_integer_t>());
				ec.res = json::number_integer_t(pow(da, db) + .5);
			}
			else
			{	//	result must be double
				json::number_float_t da = ec.sub.get<json::number_float_t>();
				ec.res = json::number_float_t(pow(da, db));
			}
		}
		else
			ec.res = json();
	}

	void  jsonSqrt(jsonRVM& rvm, EntContext& ec)
	{
		if (ec.obj.is_number())
			ec.sub = json::number_float_t(sqrt(ec.obj.get<json::number_float_t>()));
		else
			ec.sub = json();
	}

	////////////////////////////////////////////////////////////////////////////////////


	void  jsonForEachObject(jsonRVM& rvm, EntContext& ec)
	{	/*
			Множественный JSONExec для проекции объекта типа array
		*/
		if (ec.obj.is_array())
		{
			if (!ec.res.is_array()) ec.res = json::array();
			size_t i = 0;
			for (auto& it : ec.obj)
			{
				try {
					json& value = ec.res[i];
					rvm.JSONExec(EntContext(value, it, value, ec.ent, ec.ctx), ec.sub); i++;
				}
				catch (json& j) { ec.throw_json(__FUNCTION__, json({ {"["s + to_string(i) + "]"s, j} })); }
			}
		}
		else
			ec.throw_json(__FUNCTION__, "$obj must be array!"s);
	}

	void  jsonForEachSubject(jsonRVM& rvm, EntContext& ec)
	{	/*
			Множественный JSONExec для субъекта типа array
		*/
		if (ec.sub.is_array())
		{
			if (!ec.res.is_array()) ec.res = json::array();
			size_t i = 0;
			for (auto& it : ec.sub)
			{
				try {
					json& value = ec.res[i];
					rvm.JSONExec(EntContext(value, it, value, ec.ent, ec.ctx), ec.obj); i++;
				}
				catch (json& j) { ec.throw_json(__FUNCTION__, json({ {"["s + to_string(i) + "]"s, j} })); }
			}
		}
		else
			ec.throw_json(__FUNCTION__, "$sub must be array!"s);
	}

	void  jsonSize(jsonRVM& rvm, EntContext& ec)
	{
		ec.sub = ec.obj.size();
	}

#define define_json_is_type(json_type)							\
	void  json_is_##json_type(jsonRVM& rvm, EntContext& ec)		\
	{															\
		ec.sub = ec.obj.is_##json_type();						\
	}

	void  json_is_not_null(jsonRVM& rvm, EntContext& ec)
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

		void  jsonIntegerSequence(jsonRVM& rvm, EntContext& ec)
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

	void  jsonUnion(jsonRVM& rvm, EntContext& ec)
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

		ec.res = ec.sub;
	}

	void  jsonNull(jsonRVM& rvm, EntContext& ec) {}

	void  jsonInt32(jsonRVM& rvm, EntContext& ec)
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

		ec.res = ec.sub;
	}

	void  jsonDouble(jsonRVM& rvm, EntContext& ec)
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


	void  string_string(jsonRVM& rvm, EntContext& ec)
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


	void  string_add(jsonRVM& rvm, EntContext& ec)
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


	void  string_find(jsonRVM& rvm, EntContext& ec)
	{
		if (!(ec.obj.is_string() && ec.sub.is_string())) ec.throw_json(__FUNCTION__, "$obj and $sub must be strings!"s);
		ec.res = static_cast<json::number_integer_t>(ec.obj.get_ref<string&>().find(ec.sub.get_ref<string&>().c_str()));
	}


	void  string_split(jsonRVM& rvm, EntContext& ec)
	{
		if (ec.obj.is_string() && ec.sub.is_string())
		{
			const string& delim = ec.obj.get_ref<string&>();
			string str = ec.sub.get_ref<string&>();
			size_t prev = 0, pos = 0;
			ec.res = json::array();

			do
			{
				pos = str.find(delim, prev);
				if (pos == string::npos) pos = str.length();
				ec.res.push_back(str.substr(prev, pos - prev));
				prev = pos + delim.length();
			} while (pos < str.length() && prev < str.length());
		}
		else
			ec.throw_json(__FUNCTION__, "$obj and $sub must be string!"s);
	}

	void  string_join(jsonRVM& rvm, EntContext& ec)
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

			ec.res = result;
		}
		else
			ec.throw_json(__FUNCTION__, "$obj must be string and $sub must be array!"s);
	}

	void  jsonGet(jsonRVM& rvm, EntContext& ec)
	{
		if (ec.sub.is_array())
		{
			if (ec.obj.is_number())
				ec.res = ec.sub[ec.obj.get<size_t>()];
			else
				ec.throw_json(__FUNCTION__, "$obj must be unsigned number!"s);
		}
		else if (ec.sub.is_object())
		{
			if (ec.obj.is_string())
				ec.res = ec.sub[ec.obj.get_ref<string&>()];
			else
				ec.throw_json(__FUNCTION__, "$obj must be string!"s);
		}
		else
			ec.throw_json(__FUNCTION__, "$sub must be array or object!"s);
	}

	void  jsonSet(jsonRVM& rvm, EntContext& ec)
	{
		if (ec.sub.is_array())
		{
			if (ec.obj.is_number_unsigned())
				ec.sub[ec.obj.get<size_t>()] = ec.res;
			else
				ec.throw_json(__FUNCTION__, "$obj must be unsigned number!"s);
		}
		else if (ec.sub.is_object())
		{
			if (ec.obj.is_string())
				ec.sub[ec.obj.get_ref<string&>()] = ec.res;
			else
				ec.throw_json(__FUNCTION__, "$obj must be string!"s);
		}
		else if (ec.sub.is_null())
		{
			if (ec.obj.is_number_unsigned())
				ec.sub[ec.obj.get<size_t>()] = ec.res;
			else if (ec.obj.is_string())
				ec.sub[ec.obj.get_ref<string&>()] = ec.res;
			else
				ec.throw_json(__FUNCTION__, "$obj must be unsigned number or string!"s);
		}
		else
			ec.throw_json(__FUNCTION__, "$sub must be array, object or null!"s);
	}

	void  jsonErase(jsonRVM& rvm, EntContext& ec)
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

	void  jsonIsEq(jsonRVM& rvm, EntContext& ec)
	{
		ec.res = ec.sub == ec.obj;
	}

	void  jsonIsNotEq(jsonRVM& rvm, EntContext& ec)
	{
		ec.res = ec.sub != ec.obj;
	}

	void  jsonSum(jsonRVM& rvm, EntContext& ec)
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

	void  jsonWhere(jsonRVM& rvm, EntContext& ec)
	{
		ec.res = json::array();		//	подготовка выходного массива

		if (ec.obj.is_null()) return;

		if (ec.obj.is_array())
		{
			size_t i = 0;
			for (json& it : ec.obj)
			{
				try {
					json	boolres;
					rvm.JSONExec(EntContext(boolres, it, boolres, ec.ent, ec.ctx), ec.sub);
					if (boolres.is_boolean())
						if (boolres.get<bool>())
							ec.res.push_back(it);	//	фильтруем
					i++;
				}
				catch (json& j) { ec.throw_json(__FUNCTION__, json({ {"["s + to_string(i) + "]"s, j} })); }
			}
		}
		else
			ec.throw_json(__FUNCTION__, "$obj must be json array!"s);
	}

	void  jsonBelow(jsonRVM& rvm, EntContext& ec)	//	<
	{
		if (ec.sub.type() == ec.obj.type()) switch (ec.obj.type())
		{
		case json::value_t::array:
		case json::value_t::object:
			ec.res = json(ec.sub.size() < ec.obj.size());
			return;

		case json::value_t::string:
			ec.res = json(ec.sub.get<string>().size() < ec.obj.get<string>().size());
			return;

		case json::value_t::boolean:
			ec.res = json(int(ec.sub.get<bool>()) < int(ec.obj.get<bool>()));
			return;

		case json::value_t::number_float:
			ec.res = json(ec.sub.get<json::number_float_t>() < ec.obj.get<json::number_float_t>());
			return;

		case json::value_t::number_integer:
			ec.res = json(ec.sub.get<json::number_integer_t>() < ec.obj.get<json::number_integer_t>());
			return;

		case json::value_t::number_unsigned:
			ec.res = json(ec.sub.get<json::number_unsigned_t>() < ec.obj.get<json::number_unsigned_t>());
			return;

		default:
			ec.res = json();
			return;
		}
	}

	void  jsonAnd(jsonRVM& rvm, EntContext& ec)
	{
		if (ec.sub.is_boolean() && ec.obj.is_boolean())
			ec.res = ec.sub.get<bool>() && ec.obj.get<bool>();
		else
			ec.throw_json(__FUNCTION__, "$obj and $sub must be boolean!"s);
	}

	void  IfObjTrueThenExecSub(jsonRVM& rvm, EntContext& ec)
	{
		if (!ec.obj.is_boolean())
			ec.throw_json(__FUNCTION__, "$obj must be boolean!"s);

		try
		{
			if (ec.obj.get<bool>())
				rvm.JSONExec(ec.ctx, ec.sub);
		}
		catch (json& j) { throw json({ { __FUNCTION__, j} }); }
	}

	void  IfObjFalseThenExecSub(jsonRVM& rvm, EntContext& ec)
	{
		if (!ec.obj.is_boolean())
			ec.throw_json(__FUNCTION__, "$obj must be boolean!"s);

		try
		{
			if (!ec.obj.get<bool>())
				rvm.JSONExec(ec.ctx, ec.sub);
		}
		catch (json& j) { throw json({ { __FUNCTION__, j} }); }
	}

	void  ExecSubWhileObjTrue(jsonRVM& rvm, EntContext& ec)
	{
		while (true)
		{
			if (!ec.obj.is_boolean()) ec.throw_json(__FUNCTION__, "$obj must be boolean!"s);
			if (!ec.obj.get<bool>()) return;
			try
			{
				rvm.JSONExec(ec.ctx, ec.sub);
			}
			catch (json& j) { throw json({ { __FUNCTION__, j} }); }
		}
	}

	void  json_switch_bool(jsonRVM& rvm, EntContext& ec)
	{
		if (ec.obj.is_null())
			return;

		if (!ec.obj.is_boolean())
			ec.throw_json(__FUNCTION__, "$obj must be boolean!"s);

		if (!ec.sub.is_object())
			ec.throw_json(__FUNCTION__, "$sub must be json object!"s);

		try
		{
			if (ec.obj.get<bool>())	rvm.JSONExec(ec.ctx, ec.sub["true"]);
			else	rvm.JSONExec(ec.ctx, ec.sub["false"]);
		}
		catch (json& j) { throw json({ { __FUNCTION__, j} }); }
		catch (json::exception& e) { ec.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { ec.throw_json(__FUNCTION__, "std::exception: "s + e.what()); }
		catch (...) { ec.throw_json(__FUNCTION__, "unknown exception"s); }
	}

	void  json_switch_number(jsonRVM& rvm, EntContext& ec)
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

			if (ec.sub.count(key)) rvm.JSONExec(ec.ctx, ec.sub[key]);
			else rvm.JSONExec(ec.ctx, ec.sub["default"]);
		}
		catch (json& j) { throw json({ { __FUNCTION__, j} }); }
		catch (json::exception& e) { ec.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { ec.throw_json(__FUNCTION__, "std::exception: "s + e.what()); }
		catch (...) { ec.throw_json(__FUNCTION__, "unknown exception"s); }
	}


	void  json_switch_string(jsonRVM& rvm, EntContext& ec)
	{
		if (ec.obj.is_null())
			return;

		if (!ec.obj.is_string())
			ec.throw_json(__FUNCTION__, "$obj must be string!"s);

		if (!ec.sub.is_object())
			ec.throw_json(__FUNCTION__, "$sub must be json object!"s);

		try
		{
			if (ec.sub.count(ec.obj.get_ref<string&>())) rvm.JSONExec(ec.ctx, ec.sub[ec.obj.get_ref<string&>()]);
			else rvm.JSONExec(ec.ctx, ec.sub["default"]);
		}
		catch (json& j) { throw json({ { __FUNCTION__, j} }); }
		catch (json::exception& e) { ec.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
		catch (std::exception& e) { ec.throw_json(__FUNCTION__, "std::exception: "s + e.what()); }
		catch (...) { ec.throw_json(__FUNCTION__, "unknown exception"s); }
	}


	void  json_throw(jsonRVM& rvm, EntContext& ec) { throw ec.obj; }


	void  json_catch(jsonRVM& rvm, EntContext& ec)
	{
		//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
		//	проецируем во внешнем контексте
		try
		{
			rvm.JSONExec(EntContext(ec.res, ec.ctx.obj, ec.ctx.sub, ec.ctx.ent, ec.ctx), ec.obj);
		}
		catch (json& j)
		{
			ec.res = j;
			rvm.JSONExec(EntContext(ec.res, ec.ctx.obj, ec.ctx.sub, ec.ctx.ent, ec.ctx), ec.sub);
		}
	}


#define define_object_method(object_method)										\
		void  json_call_##object_method(jsonRVM& rvm, EntContext& ec)	\
		{																		\
			ec.sub = ec.obj.##object_method();									\
		}

#define define_static_method(static_method)										\
		void  json_call_##static_method(jsonRVM& rvm, EntContext& ec)	\
		{																		\
			ec.res = json::##static_method();									\
		}

	define_static_method(array)
		define_static_method(meta)
		define_static_method(object)

		void  json_call_null(jsonRVM& rvm, EntContext& ec)
	{
		ec.res = json();
	}

	void  jsonPrint(jsonRVM& rvm, EntContext& ec)
	{
		cout << ec.obj.dump(1) << endl;
	}


	void  jsonTAG(jsonRVM& rvm, EntContext& ec)
	{
		if (!ec.sub.is_object())
			ec.throw_json(__FUNCTION__, "$sub must be json object!"s);

		string	tag = ec.sub["<>"];

		if (!ec.res.is_string())
			ec.res = ""s;

		string& body = ec.res.get_ref<string&>();
		body += "<" + tag;

		for (auto& it : ec.sub.items())
			if ("<>" != it.key())
				body += " " + it.key() + "=" + it.value().dump();

		body += ">";
		//	что бы выходной поток xml попадал в тоже значение ec.res нужно исполнять в текущем контексте EV
		json	objview;
		rvm.JSONExec(EntContext(objview, ec.obj, ec.sub, ec.ent, ec.ctx), ec.obj);
		if (objview.is_string()) body += objview.get_ref<string&>();
		else                     body += objview.dump();
		body += "</" + tag + ">";
	}


	void  jsonXML(jsonRVM& rvm, EntContext& ec)
	{
		std::string res = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"s;

		if (!ec.sub.is_object())
			ec.throw_json(__FUNCTION__, "$sub must be json object!"s);

		string	tag = ec.sub["<>"];

		if (!ec.res.is_string())
			ec.res = res;
		else if (!ec.res.get_ref<string&>().size())
			ec.res = res;

		string& body = ec.res.get_ref<string&>();
		body += "<" + tag;

		for (auto& it : ec.sub.items())
			if ("<>" != it.key())
				body += " " + it.key() + "=" + it.value().dump();

		body += ">";
		//	что бы выходной поток xml попадал в тоже значение ec.res нужно исполнять в текущем контексте EV
		json	objview;
		rvm.JSONExec(EntContext(objview, ec.obj, ec.sub, ec.ent, ec.ctx), ec.obj);
		if (objview.is_string()) body += objview.get_ref<string&>();
		else                     body += objview.dump();
		body += "</" + tag + ">";
	}


	void  jsonHTML(jsonRVM& rvm, EntContext& ec)
	{
		std::string res = "<!DOCTYPE html>"s;

		if (!ec.sub.is_object())
			ec.throw_json(__FUNCTION__, "$sub must be json object!"s);

		string	tag = ec.sub["<>"];

		if (!ec.res.is_string())
			ec.res = res;
		else if (!ec.res.get_ref<string&>().size())
			ec.res = res;

		string& body = ec.res.get_ref<string&>();
		body += "<" + tag;

		for (auto& it : ec.sub.items())
			if ("<>" != it.key())
				body += " " + it.key() + "=" + it.value().dump();

		body += ">";
		//	что бы выходной поток xml попадал в тоже значение ec.res нужно исполнять в текущем контексте EV
		json	objview;
		rvm.JSONExec(EntContext(objview, ec.obj, ec.sub, ec.ent, ec.ctx), ec.obj);
		if (objview.is_string()) body += objview.get_ref<string&>();
		else                     body += objview.dump();
		body += "</" + tag + ">";
	}

	void  jsonDump(jsonRVM& rvm, EntContext& ec)
	{
		ec.sub = ec.obj.dump(3);
	}

	void steady_clock_nanoseconds(jsonRVM& rvm, EntContext& ec) {
		ec.res = chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now().time_since_epoch()).count();
	}

	void steady_clock_microseconds(jsonRVM& rvm, EntContext& ec) {
		ec.res = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now().time_since_epoch()).count();
	}

	void steady_clock_milliseconds(jsonRVM& rvm, EntContext& ec) {
		ec.res = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
	}

	void steady_clock_seconds(jsonRVM& rvm, EntContext& ec) {
		ec.res = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now().time_since_epoch()).count();
	}

	void steady_clock_minutes(jsonRVM& rvm, EntContext& ec) {
		ec.res = chrono::duration_cast<chrono::minutes>(chrono::steady_clock::now().time_since_epoch()).count();
	}

	void steady_clock_hours(jsonRVM& rvm, EntContext& ec) {
		ec.res = chrono::duration_cast<chrono::hours>(chrono::steady_clock::now().time_since_epoch()).count();
	}

	void ImportRelationsModel(jsonRVM& rvm)
	{
		import_help(rvm);
		rvm["RVM_version"] = RVM_version;
		rvm.AddBaseEntity(rvm["RVM"]["sleep"], "ms"s, sleep_ms, "sleep in milliconds"s);
		rvm.AddBaseEntity(rvm, "view"s, jsonView, "ViewEntity: View object model in parent ctx and then set subject value"s);
		rvm.AddBaseEntity(rvm, "="s, jsonCopy, "Copy: copy object model to subject value"s);

#define map_json_is_type(json_type)	rvm.AddBaseEntity( rvm, "is_"s + #json_type, json_is_##json_type, ""s );
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
#define map_json_static_method(static_method)	rvm.AddBaseEntity( rvm["json"], #static_method, json_call_##static_method, ""s );
		map_json_static_method(array);
		map_json_static_method(null);
		map_json_static_method(meta);
		map_json_static_method(object);

		rvm.AddBaseEntity(rvm["json"], "dump"s, jsonDump, ""s);

		//	convert
		rvm.AddBaseEntity(rvm, "integer"s, jsonInt32, ""s);
		rvm.AddBaseEntity(rvm, "int"s, jsonInt32, ""s);
		rvm.AddBaseEntity(rvm, "float"s, jsonDouble, ""s);
		rvm.AddBaseEntity(rvm, "double"s, jsonDouble, ""s);
		rvm.AddBaseEntity(rvm, "null"s, jsonNull, ""s);

		//	data operations
		rvm.AddBaseEntity(rvm, "where"s, jsonWhere, ""s);
		rvm.AddBaseEntity(rvm, "union"s, jsonUnion, ""s);
		rvm.AddBaseEntity(rvm, "size"s, jsonSize, ""s);
		rvm.AddBaseEntity(rvm, "get"s, jsonGet, ""s);
		rvm.AddBaseEntity(rvm, "set"s, jsonSet, ""s);
		rvm.AddBaseEntity(rvm, "erase"s, jsonErase, "Удаляет элементы, которые соответствуют заданному ключу."s);
		rvm.AddBaseEntity(rvm["sequence"], "integer"s, jsonIntegerSequence, ""s);

		//	math
		rvm.AddBaseEntity(rvm, "*"s, jsonMul, ""s);
		rvm.AddBaseEntity(rvm, ":"s, jsonDiv, "субъект делимое, объект делитель"s);
		rvm.AddBaseEntity(rvm, "+", jsonAdd, ""s);
		rvm.AddBaseEntity(rvm, "-", jsonSubstract, ""s);
		rvm.AddBaseEntity(rvm, "pow"s, jsonPower, ""s);
		rvm.AddBaseEntity(rvm, "sqrt"s, jsonSqrt, ""s);
		rvm.AddBaseEntity(rvm, "sum"s, jsonSum, ""s);

		//	logic
		rvm.AddBaseEntity(rvm, "^"s, jsonXOR, ""s);
		rvm.AddBaseEntity(rvm, "=="s, jsonIsEq, ""s);
		rvm.AddBaseEntity(rvm, "!="s, jsonIsNotEq, ""s);
		rvm.AddBaseEntity(rvm, "<"s, jsonBelow, ""s);
		rvm.AddBaseEntity(rvm, "&&"s, jsonAnd, ""s);

		//	strings
		rvm.AddBaseEntity(rvm["string"], "="s, string_string, ""s);
		rvm.AddBaseEntity(rvm["string"], "+="s, string_add, ""s);
		rvm.AddBaseEntity(rvm["string"], "find"s, string_find, ""s);
		rvm.AddBaseEntity(rvm["string"], "split"s, string_split, ""s);
		rvm.AddBaseEntity(rvm["string"], "join"s, string_join, ""s);

		//	control
		rvm.AddBaseEntity(rvm, "foreachobj"s, jsonForEachObject, ""s);
		rvm.AddBaseEntity(rvm, "foreachsub"s, jsonForEachSubject, ""s);
		rvm.AddBaseEntity(rvm, "then"s, IfObjTrueThenExecSub, ""s);
		rvm.AddBaseEntity(rvm, "else"s, IfObjFalseThenExecSub, "");
		rvm.AddBaseEntity(rvm, "while"s, ExecSubWhileObjTrue, ""s);
		rvm.AddBaseEntity(rvm["switch"], "bool"s, json_switch_bool, ""s);
		rvm.AddBaseEntity(rvm["switch"], "number"s, json_switch_number, ""s);
		rvm.AddBaseEntity(rvm["switch"], "string"s, json_switch_string, ""s);
		rvm.AddBaseEntity(rvm, "throw"s, json_throw, "");
		rvm.AddBaseEntity(rvm, "catch"s, json_catch, "Exec $obj in the parent ctx, if an exception is thrown, then it is written to the current view value and exec $sub in the parent ctx");

		//	display
		rvm.AddBaseEntity(rvm, "print"s, jsonPrint, ""s);

		//	browser
		rvm.AddBaseEntity(rvm, "tag"s, jsonTAG, ""s);
		rvm.AddBaseEntity(rvm, "xml"s, jsonXML, ""s);
		rvm.AddBaseEntity(rvm, "html"s, jsonHTML, ""s);

		//	steady clock
		rvm.AddBaseEntity(rvm["steady_clock"], "nanoseconds"s, steady_clock_nanoseconds, "Sets $res to time since epoch in nanoseconds"s);
		rvm.AddBaseEntity(rvm["steady_clock"], "microseconds"s, steady_clock_microseconds, "Sets $res to time since epoch in microseconds"s);
		rvm.AddBaseEntity(rvm["steady_clock"], "milliseconds"s, steady_clock_milliseconds, "Sets $res to time since epoch in milliseconds"s);
		rvm.AddBaseEntity(rvm["steady_clock"], "seconds"s, steady_clock_seconds, "Sets $res to time since epoch in seconds"s);
		rvm.AddBaseEntity(rvm["steady_clock"], "minutes"s, steady_clock_minutes, "Sets $res to time since epoch in minutes"s);
		rvm.AddBaseEntity(rvm["steady_clock"], "hours"s, steady_clock_hours, "Sets $res to time since epoch in hours"s);
	}
}