#pragma once
#include <iostream>
#include <chrono>
#include "JsonRVM.h"
#include "windows.h"

namespace rm
{
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

	template<typename rvm_impl_t>
	class base_vocabulary
	{

		static void  sleep_ms(rvm_impl_t& rvm, EntContext& ec)
		{
			Sleep(ec.obj.get<json::number_unsigned_t>());
		}

		static void  jsonCopy(rvm_impl_t& rvm, EntContext& ec)
		{	//	полное копированиее json значения объекта в субъект
			ec.sub = ec.obj;
		}

		static void  jsonView(rvm_impl_t& rvm, EntContext& ec)
		{	//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
			//	проецируем во внешнем контексте
			rvm.JSONExec(EntContext(ec.sub, ec.ctx.obj, ec.ctx.sub, ec.ctx.ent, ec.ctx), ec.obj);
		}

		static void  jsonXOR(rvm_impl_t& rvm, EntContext& ec)
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
static void  json##name (rvm_impl_t& rvm, EntContext& ec)			\
{																								\
	switch( (uint8_t(ec.sub.type()) << sub_field) | uint8_t(ec.obj.type()) )							\
	{ VM_OPP( operation ) default: ec.val = json(); }												\
}

		OP_BODY(Add, +);
		OP_BODY(Substract, -);
		OP_BODY(Mul, *);

		static void  jsonDiv(rvm_impl_t& rvm, EntContext& ec)
		{
			if (ec.obj.is_number())
				if (ec.obj.get<double>() == 0.0)
				{
					ec.val = json();
					return;
				}
			switch ((uint8_t(ec.sub.type()) << sub_field) | uint8_t(ec.obj.type()))
			{
			VM_OPP(/ ) default: ec.val = json();
			}
		}


		//////////////////////////////////////////////////////////////////////////////////////////////
		static void  jsonPower(rvm_impl_t& rvm, EntContext& ec)
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

		static void  jsonSqrt(rvm_impl_t& rvm, EntContext& ec)
		{
			if (ec.obj.is_number())
				ec.sub = json::number_float_t(sqrt(ec.obj.get<json::number_float_t>()));
			else
				ec.sub = json();
		}

		////////////////////////////////////////////////////////////////////////////////////


		static void  jsonForEachObject(rvm_impl_t& rvm, EntContext& ec)
		{	/*
				Множественный JSONExec для проекции объекта типа array
			*/
			if (ec.obj.is_array())
			{
				if (!ec.val.is_array()) ec.val = json::array();
				size_t i = 0;
				for (auto& it : ec.obj)
				{
					try {
						json& value = ec.val[i];
						rvm.JSONExec(EntContext(value, it, value, ec.ent, ec.ctx), ec.sub); i++;
					}
					catch (json & j) { ec.throw_json(__FUNCTION__, json({ {"["s + to_string(i) + "]"s, j} })); }
				}
			}
			else
				ec.throw_json(__FUNCTION__, "$obj must be array!"s);
		}

		static void  jsonForEachSubject(rvm_impl_t& rvm, EntContext& ec)
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
						rvm.JSONExec(EntContext(value, it, value, ec.ent, ec.ctx), ec.obj); i++;
					}
					catch (json & j) { ec.throw_json(__FUNCTION__, json({ {"["s + to_string(i) + "]"s, j} })); }
				}
			}
			else
				ec.throw_json(__FUNCTION__, "$sub must be array!"s);
		}

		static void  jsonSize(rvm_impl_t& rvm, EntContext& ec)
		{
			ec.sub = ec.obj.size();
		}

#define define_json_is_type(json_type)										\
		static void  json_is_##json_type(rvm_impl_t& rvm, EntContext& ec)	\
		{																	\
			ec.sub = ec.obj.is_##json_type();								\
		}

		static void  json_is_not_null(rvm_impl_t& rvm, EntContext& ec)
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

		static void  jsonIntegerSequence(rvm_impl_t& rvm, EntContext& ec)
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

		static void  jsonUnion(rvm_impl_t& rvm, EntContext& ec)
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

		static void  jsonNull(rvm_impl_t& rvm, EntContext& ec) {}

		static void  jsonInt32(rvm_impl_t& rvm, EntContext& ec)
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

		static void  jsonDouble(rvm_impl_t& rvm, EntContext& ec)
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


		static void  string_string(rvm_impl_t& rvm, EntContext& ec)
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


		static void  string_add(rvm_impl_t& rvm, EntContext& ec)
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


		static void  string_find(rvm_impl_t& rvm, EntContext& ec)
		{
			if (!(ec.obj.is_string() && ec.sub.is_string())) ec.throw_json(__FUNCTION__, "$obj and $sub must be strings!"s);
			ec.val = static_cast<json::number_integer_t>(ec.obj.get_ref<string&>().find(ec.sub.get_ref<string&>().c_str()));
		}


		static void  string_split(rvm_impl_t& rvm, EntContext& ec)
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
				ec.throw_json(__FUNCTION__, "$obj and $sub must be string!"s);
		}

		static void  string_join(rvm_impl_t& rvm, EntContext& ec)
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

				ec.val = result;
			}
			else
				ec.throw_json(__FUNCTION__, "$obj must be string and $sub must be array!"s);
		}

		static void  jsonGet(rvm_impl_t& rvm, EntContext& ec)
		{
			if (ec.sub.is_array())
			{
				if (ec.obj.is_number())
					ec.val = ec.sub[ec.obj.get<size_t>()];
				else
					ec.throw_json(__FUNCTION__, "$obj must be unsigned number!"s);
			}
			else if (ec.sub.is_object())
			{
				if (ec.obj.is_string())
					ec.val = ec.sub[ec.obj.get_ref<string&>()];
				else
					ec.throw_json(__FUNCTION__, "$obj must be string!"s);
			}
			else
				ec.throw_json(__FUNCTION__, "$sub must be array or object!"s);
		}

		static void  jsonSet(rvm_impl_t& rvm, EntContext& ec)
		{
			if (ec.sub.is_array())
			{
				if (ec.obj.is_number_unsigned())
					ec.sub[ec.obj.get<size_t>()] = ec.val;
				else
					ec.throw_json(__FUNCTION__, "$obj must be unsigned number!"s);
			}
			else if (ec.sub.is_object())
			{
				if (ec.obj.is_string())
					ec.sub[ec.obj.get_ref<string&>()] = ec.val;
				else
					ec.throw_json(__FUNCTION__, "$obj must be string!"s);
			}
			else if (ec.sub.is_null())
			{
				if (ec.obj.is_number_unsigned())
					ec.sub[ec.obj.get<size_t>()] = ec.val;
				else if (ec.obj.is_string())
					ec.sub[ec.obj.get_ref<string&>()] = ec.val;
				else
					ec.throw_json(__FUNCTION__, "$obj must be unsigned number or string!"s);
			}
			else
				ec.throw_json(__FUNCTION__, "$sub must be array, object or null!"s);
		}

		static void  jsonErase(rvm_impl_t& rvm, EntContext& ec)
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

		static void  jsonIsEq(rvm_impl_t& rvm, EntContext& ec)
		{
			ec.val = ec.sub == ec.obj;
		}

		static void  jsonIsNotEq(rvm_impl_t& rvm, EntContext& ec)
		{
			ec.val = ec.sub != ec.obj;
		}

		static void  jsonSum(rvm_impl_t& rvm, EntContext& ec)
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

		static void  jsonWhere(rvm_impl_t& rvm, EntContext& ec)
		{
			ec.val = json::array();		//	подготовка выходного массива

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
								ec.val.push_back(it);	//	фильтруем
						i++;
					}
					catch (json & j) { ec.throw_json(__FUNCTION__, json({ {"["s + to_string(i) + "]"s, j} })); }
				}
			}
			else
				ec.throw_json(__FUNCTION__, "$obj must be json array!"s);
		}

		static void  jsonBelow(rvm_impl_t& rvm, EntContext& ec)	//	<
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

		static void  jsonAnd(rvm_impl_t& rvm, EntContext& ec)
		{
			if (ec.sub.is_boolean() && ec.obj.is_boolean())
				ec.val = ec.sub.get<bool>() && ec.obj.get<bool>();
			else
				ec.throw_json(__FUNCTION__, "$obj and $sub must be boolean!"s);
		}

		static void  IfObjTrueThenExecSub(rvm_impl_t& rvm, EntContext& ec)
		{
			if (!ec.obj.is_boolean())
				ec.throw_json(__FUNCTION__, "$obj must be boolean!"s);

			try
			{
				if (ec.obj.get<bool>())
					rvm.JSONExec(ec.ctx, ec.sub);
			}
			catch (json & j) { throw json({ { __FUNCTION__, j} }); }
		}

		static void  IfObjFalseThenExecSub(rvm_impl_t& rvm, EntContext& ec)
		{
			if (!ec.obj.is_boolean())
				ec.throw_json(__FUNCTION__, "$obj must be boolean!"s);

			try
			{
				if (!ec.obj.get<bool>())
					rvm.JSONExec(ec.ctx, ec.sub);
			}
			catch (json & j) { throw json({ { __FUNCTION__, j} }); }
		}

		static void  ExecSubWhileObjTrue(rvm_impl_t& rvm, EntContext& ec)
		{
			while (true)
			{
				if (!ec.obj.is_boolean()) ec.throw_json(__FUNCTION__, "$obj must be boolean!"s);
				if (!ec.obj.get<bool>()) return;
				try
				{
					rvm.JSONExec(ec.ctx, ec.sub);
				}
				catch (json & j) { throw json({ { __FUNCTION__, j} }); }
			}
		}

		static void  json_switch_bool(rvm_impl_t& rvm, EntContext& ec)
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
			catch (json & j) { throw json({ { __FUNCTION__, j} }); }
			catch (json::exception & e) { ec.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
			catch (std::exception & e) { ec.throw_json(__FUNCTION__, "std::exception: "s + e.what()); }
			catch (...) { ec.throw_json(__FUNCTION__, "unknown exception"s); }
		}

		static void  json_switch_number(rvm_impl_t& rvm, EntContext& ec)
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
			catch (json & j) { throw json({ { __FUNCTION__, j} }); }
			catch (json::exception & e) { ec.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
			catch (std::exception & e) { ec.throw_json(__FUNCTION__, "std::exception: "s + e.what()); }
			catch (...) { ec.throw_json(__FUNCTION__, "unknown exception"s); }
		}


		static void  json_switch_string(rvm_impl_t& rvm, EntContext& ec)
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
			catch (json & j) { throw json({ { __FUNCTION__, j} }); }
			catch (json::exception & e) { ec.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
			catch (std::exception & e) { ec.throw_json(__FUNCTION__, "std::exception: "s + e.what()); }
			catch (...) { ec.throw_json(__FUNCTION__, "unknown exception"s); }
		}


		static void  json_throw(rvm_impl_t& rvm, EntContext& ec) { throw ec.obj; }


		static void  json_catch(rvm_impl_t& rvm, EntContext& ec)
		{
			//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
			//	проецируем во внешнем контексте
			try
			{
				rvm.JSONExec(EntContext(ec.val, ec.ctx.obj, ec.ctx.sub, ec.ctx.ent, ec.ctx), ec.obj);
			}
			catch (json & j)
			{
				ec.val = j;
				rvm.JSONExec(EntContext(ec.val, ec.ctx.obj, ec.ctx.sub, ec.ctx.ent, ec.ctx), ec.sub);
			}
		}


#define define_object_method(object_method)										\
		static void  json_call_##object_method(rvm_impl_t& rvm, EntContext& ec)	\
		{																		\
			ec.sub = ec.obj.##object_method();									\
		}

#define define_static_method(static_method)										\
		static void  json_call_##static_method(rvm_impl_t& rvm, EntContext& ec)	\
		{																		\
			ec.val = json::##static_method();									\
		}

		define_static_method(array)
		define_static_method(meta)
		define_static_method(object)

		static void  json_call_null(rvm_impl_t& rvm, EntContext& ec)
		{
			ec.val = json();
		}

		static void  jsonPrint(rvm_impl_t& rvm, EntContext& ec)
		{
			cout << ec.obj.dump(1) << endl;
		}


		static void  jsonTAG(rvm_impl_t& rvm, EntContext& ec)
		{
			if (!ec.sub.is_object())
				ec.throw_json(__FUNCTION__, "$sub must be json object!"s);

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
			rvm.JSONExec(EntContext(objview, ec.obj, ec.sub, ec.ent, ec.ctx), ec.obj);
			if (objview.is_string()) body += objview.get_ref<string&>();
			else                     body += objview.dump();
			body += "</" + tag + ">";
		}


		static void  jsonXML(rvm_impl_t& rvm, EntContext& ec)
		{
			std::string res = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"s;

			if (!ec.sub.is_object())
				ec.throw_json(__FUNCTION__, "$sub must be json object!"s);

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
			rvm.JSONExec(EntContext(objview, ec.obj, ec.sub, ec.ent, ec.ctx), ec.obj);
			if (objview.is_string()) body += objview.get_ref<string&>();
			else                     body += objview.dump();
			body += "</" + tag + ">";
		}


		static void  jsonHTML(rvm_impl_t& rvm, EntContext& ec)
		{
			std::string res = "<!DOCTYPE html>"s;

			if (!ec.sub.is_object())
				ec.throw_json(__FUNCTION__, "$sub must be json object!"s);

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
			rvm.JSONExec(EntContext(objview, ec.obj, ec.sub, ec.ent, ec.ctx), ec.obj);
			if (objview.is_string()) body += objview.get_ref<string&>();
			else                     body += objview.dump();
			body += "</" + tag + ">";
		}


		static void  jsonDump(rvm_impl_t& rvm, EntContext& ec)
		{
			ec.sub = ec.obj.dump(3);
		}

		/*
		using nanoseconds  = duration<long long, nano>;
    using microseconds = duration<long long, micro>;
    using milliseconds = duration<long long, milli>;
    using seconds      = duration<long long>;
    using minutes      = duration<int, ratio<60>>;
    using hours        = duration<int, ratio<3600>>;
		*/

		static void steady_clock_nanoseconds(rvm_impl_t& rvm, EntContext& ec) {
			ec.val = chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now().time_since_epoch()).count();
		}

		static void steady_clock_microseconds(rvm_impl_t& rvm, EntContext& ec) {
			ec.val = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now().time_since_epoch()).count();
		}

		static void steady_clock_milliseconds(rvm_impl_t& rvm, EntContext& ec) {
			ec.val = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
		}

		static void steady_clock_seconds(rvm_impl_t& rvm, EntContext& ec) {
			ec.val = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now().time_since_epoch()).count();
		}

		static void steady_clock_minutes(rvm_impl_t& rvm, EntContext& ec) {
			ec.val = chrono::duration_cast<chrono::minutes>(chrono::steady_clock::now().time_since_epoch()).count();
		}

		static void steady_clock_hours(rvm_impl_t& rvm, EntContext& ec) {
			ec.val = chrono::duration_cast<chrono::hours>(chrono::steady_clock::now().time_since_epoch()).count();
		}

	public:
		base_vocabulary(rvm_impl_t& rvm)
		{
			import_help(rvm);
			rvm["RVM_version"] = RVM_version;
			Addx86Entity(rvm, rvm["RVM"]["sleep"], "ms"s, sleep_ms, "sleep in milliconds"s);
			Addx86Entity(rvm, rvm, "view"s, jsonView, "ViewEntity: View object model in parent ctx and then set subject value"s);
			Addx86Entity(rvm, rvm, "="s, jsonCopy, "Copy: copy object model to subject value"s);

#define map_json_is_type(json_type)	Addx86Entity(rvm, rvm, "is_"s + #json_type, json_is_##json_type, ""s );
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
#define map_json_static_method(static_method)	Addx86Entity(rvm, rvm["json"], #static_method, json_call_##static_method, ""s );
			map_json_static_method(array);
			map_json_static_method(null);
			map_json_static_method(meta);
			map_json_static_method(object);

			Addx86Entity(rvm, rvm["json"], "dump"s, jsonDump, ""s);

			//	convert
			Addx86Entity(rvm, rvm, "integer"s, jsonInt32, ""s);
			Addx86Entity(rvm, rvm, "int"s, jsonInt32, ""s);
			Addx86Entity(rvm, rvm, "float"s, jsonDouble, ""s);
			Addx86Entity(rvm, rvm, "double"s, jsonDouble, ""s);
			Addx86Entity(rvm, rvm, "null"s, jsonNull, ""s);

			//	data operations
			Addx86Entity(rvm, rvm, "where"s, jsonWhere, ""s);
			Addx86Entity(rvm, rvm, "union"s, jsonUnion, ""s);
			Addx86Entity(rvm, rvm, "size"s, jsonSize, ""s);
			Addx86Entity(rvm, rvm, "get"s, jsonGet, ""s);
			Addx86Entity(rvm, rvm, "set"s, jsonSet, ""s);
			Addx86Entity(rvm, rvm, "erase"s, jsonErase, "Удаляет элементы, которые соответствуют заданному ключу."s);
			Addx86Entity(rvm, rvm["sequence"], "integer"s, jsonIntegerSequence, ""s);

			//	math
			Addx86Entity(rvm, rvm, "*"s, jsonMul, ""s);
			Addx86Entity(rvm, rvm, ":"s, jsonDiv, "субъект делимое, объект делитель"s);
			Addx86Entity(rvm, rvm, "+", jsonAdd, ""s);
			Addx86Entity(rvm, rvm, "-", jsonSubstract, ""s);
			Addx86Entity(rvm, rvm, "pow"s, jsonPower, ""s);
			Addx86Entity(rvm, rvm, "sqrt"s, jsonSqrt, ""s);
			Addx86Entity(rvm, rvm, "sum"s, jsonSum, ""s);

			//	logic
			Addx86Entity(rvm, rvm, "^"s, jsonXOR, ""s);
			Addx86Entity(rvm, rvm, "=="s, jsonIsEq, ""s);
			Addx86Entity(rvm, rvm, "!="s, jsonIsNotEq, ""s);
			Addx86Entity(rvm, rvm, "<"s, jsonBelow, ""s);
			Addx86Entity(rvm, rvm, "&&"s, jsonAnd, ""s);

			//	strings
			Addx86Entity(rvm, rvm["string"], "="s, string_string, ""s);
			Addx86Entity(rvm, rvm["string"], "+="s, string_add, ""s);
			Addx86Entity(rvm, rvm["string"], "find"s, string_find, ""s);
			Addx86Entity(rvm, rvm["string"], "split"s, string_split, ""s);
			Addx86Entity(rvm, rvm["string"], "join"s, string_join, ""s);

			//	control
			Addx86Entity(rvm, rvm, "foreachobj"s, jsonForEachObject, ""s);
			Addx86Entity(rvm, rvm, "foreachsub"s, jsonForEachSubject, ""s);
			Addx86Entity(rvm, rvm, "then"s, IfObjTrueThenExecSub, ""s);
			Addx86Entity(rvm, rvm, "else"s, IfObjFalseThenExecSub, "");
			Addx86Entity(rvm, rvm, "while"s, ExecSubWhileObjTrue, ""s);
			Addx86Entity(rvm, rvm["switch"], "bool"s, json_switch_bool, ""s);
			Addx86Entity(rvm, rvm["switch"], "number"s, json_switch_number, ""s);
			Addx86Entity(rvm, rvm["switch"], "string"s, json_switch_string, ""s);
			Addx86Entity(rvm, rvm, "throw"s, json_throw, "");
			Addx86Entity(rvm, rvm, "catch"s, json_catch, "Exec $obj in the parent ctx, if an exception is thrown, then it is written to the current view value and exec $sub in the parent ctx");

			//	display
			Addx86Entity(rvm, rvm, "print"s, jsonPrint, ""s);

			//	browser
			Addx86Entity(rvm, rvm, "tag"s, jsonTAG, ""s);
			Addx86Entity(rvm, rvm, "xml"s, jsonXML, ""s);
			Addx86Entity(rvm, rvm, "html"s, jsonHTML, ""s);

			//	steady clock
			Addx86Entity(rvm, rvm["steady_clock"], "nanoseconds"s, steady_clock_nanoseconds, "Sets $val to time since epoch in nanoseconds"s);
			Addx86Entity(rvm, rvm["steady_clock"], "microseconds"s, steady_clock_microseconds, "Sets $val to time since epoch in microseconds"s);
			Addx86Entity(rvm, rvm["steady_clock"], "milliseconds"s, steady_clock_milliseconds, "Sets $val to time since epoch in milliseconds"s);
			Addx86Entity(rvm, rvm["steady_clock"], "seconds"s, steady_clock_seconds, "Sets $val to time since epoch in seconds"s);
			Addx86Entity(rvm, rvm["steady_clock"], "minutes"s, steady_clock_minutes, "Sets $val to time since epoch in minutes"s);
			Addx86Entity(rvm, rvm["steady_clock"], "hours"s, steady_clock_hours, "Sets $val to time since epoch in hours"s);
		}

	};

}