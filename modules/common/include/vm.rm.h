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
Copyright © 2016-2021 Vertushkin Roman Pavlovich <https://vk.com/earthbirthbook>.

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
#include "database_api.h"
#include "json.hpp"

namespace rm
{

	using namespace std;
	using namespace nlohmann;

	////////////////////////////////////////////////////////////////////////////////
	//	https://habr.com/ru/post/166201/
#define SWITCH(str)  switch(s_s::str_hash_for_switch(str))
#define CASE(str)    static_assert(s_s::str_is_correct(str) && (s_s::str_len(str) <= s_s::MAX_LEN),\
"CASE string contains wrong characters, or its length is greater than 9");\
case s_s::str_hash(str, s_s::str_len(str))
#define DEFAULT  default

	namespace s_s
	{
		typedef unsigned char uchar;
		typedef unsigned long long ullong;
		const uchar MAX_LEN = 7;
		const ullong N_HASH = static_cast<ullong>(-1);
		constexpr ullong raise_128_to(const uchar power) { return 1ULL << (7 * power); }
		constexpr bool str_is_correct(const char* const str) { return (static_cast<signed char>(*str) > 0) ? str_is_correct(str + 1) : (*str ? false : true); }
		constexpr uchar str_len(const char* const str) { return *str ? (1 + str_len(str + 1)) : 0; }
		constexpr ullong str_hash(const char* const str, const uchar current_len) { return *str ? (raise_128_to(current_len - 1) * static_cast<uchar>(*str) + str_hash(str + 1, current_len - 1)) : 0; }
		inline ullong str_hash_for_switch(const char* const str) { return str_hash(str, str_len(str)); }
		inline ullong str_hash_for_switch(const std::string& str) { return str_hash(str.c_str(), str.length()); }
	}

	
	////////////////////////////// VERSION //////////////////////////////
	const string rmvm_version = "3.0.0"s;
	////////////////////////////// VERSION //////////////////////////////

	/*
			Mapping OOP to EOP:

		(&result ? result : *this) = this->class::method( &args ? args : *this );
		-------------------------------------------------------------------------
	                                 ||
									 ||
									 ||
									\||/
									 \/
        -------------------------------------------------------------------------
		(&$.sub ? $.sub : $.its) = $.its->$.ent::$.rel( &$.obj ? $.obj : $.its );
	*/

	//	Контекст исполнения сущности
	//	инстанцированная проекция модели сущности
	//	экземпляр сущности
	struct vm_ctx
	{
		json& its;	//	instance projection reference
		json& obj;	//	context object reference
		json& sub;	//	context subject reference
		json& ent;	//	context entity referenceсущность, is model for instance
		vm_ctx& $;	//	parent context reference

		vm_ctx(json& Its, json& Obj, json& Sub, json& Ent, vm_ctx& Ctx)
			: its(Its), obj(Obj), sub(Sub), ent(Ent), $(Ctx) {}

		vm_ctx(json& Its, json& Ent)
			: its(Its), obj(Its), sub(Its), ent(Ent), $(*this) {}

		vm_ctx(json& Its)
			: its(Its), obj(Its), sub(Its), ent(Its), $(*this) {}

		void throw_json(const string& function, const json& error) const
		{
			json	j;
			j["__FUNCTION__"] = function;
			j["exception"] = error;
			j["vm_ctx"]["$ent/"] = ent;
			j["vm_ctx"]["$sub/"] = sub;
			j["vm_ctx"]["$obj/"] = obj;
			j["vm_ctx"]["$its/"] = its;
			throw j;
		}
	};

	class vm;
	//	64 bit
#ifdef WIN32
	#define IMPORT_RELATIONS_MODEL		"?ImportRelationsModel@rm@@YAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAVjsonRVM@1@@Z"
	__declspec(dllexport) const string& ImportRelationsModel(vm& rmvm);
	typedef const string& (*InitDict)(vm& rmvm);
	static InitDict	You_must_define_ImportRelationsModel_function_in_your_RM_dictionary = ImportRelationsModel;
#endif

	using binary_view = void (*)(vm& rmvm, vm_ctx& $);
	using binary_view_map_t = map<json const*, binary_view>;
	
	class vm : protected database_api, public json, public binary_view_map_t
	{
	private:
		struct rval { static const bool is_lval{ false }; };
		struct lval { static const bool is_lval{ true }; };

		template<class val_type>
		static void	ref_in_json_to(json*& jptr, const string& it)
		{
			json& ref = *jptr;

			switch (ref.type())
			{
			case json::value_t::object:
				jptr = &ref[it];
				return;

			case json::value_t::array:
				jptr = &ref[std::stoul(it)];
				return;

			case json::value_t::null:
				if constexpr (val_type::is_lval)
				{
					int& _Errno_ref = errno; // Nonzero cost, pay it once
					const char* _Ptr = it.c_str();
					char* _Eptr;
					_Errno_ref = 0;
					unsigned index = strtoul(_Ptr, &_Eptr, 10);

					if (_Errno_ref == ERANGE)
						throw json({ {__FUNCTION__, it} });

					if (_Ptr == _Eptr)
						jptr = &ref[it];
					else
						jptr = &ref[index];

					return;
				}

			default:
				throw json({ {__FUNCTION__, it} });
			}
		}

		template<class val_type>
		json& string_ref_to(vm_ctx& $, const string& str)
		{
			json* jptr;
			size_t	len = str.length();
			size_t	pos = str.find_first_of('/', 0);

			if (pos == str.npos) pos = len;

			size_t	prev = pos + 1;
			string	it = str.substr(0, pos);

			SWITCH(it)
			{
				CASE("$$$$ent") : jptr = &$.$.$.$.ent;	break;
				CASE("$$$$sub") : jptr = &$.$.$.$.sub;	break;
				CASE("$$$$obj") : jptr = &$.$.$.$.obj;	break;
				CASE("$$$$its") : jptr = &$.$.$.$.its;	break;
				CASE("$$$ent") : jptr = &$.$.$.ent;	break;
				CASE("$$$sub") : jptr = &$.$.$.sub;	break;
				CASE("$$$obj") : jptr = &$.$.$.obj;	break;
				CASE("$$$its") : jptr = &$.$.$.its;	break;
				CASE("$$ent") : jptr = &$.$.ent;	break;
				CASE("$$sub") : jptr = &$.$.sub;	break;
				CASE("$$obj") : jptr = &$.$.obj;	break;
				CASE("$$its") : jptr = &$.$.its;	break;
				CASE("$ent") : jptr = &$.ent;	break;
				CASE("$sub") : jptr = &$.sub;	break;
				CASE("$obj") : jptr = &$.obj;	break;
				CASE("$its") : jptr = &$.its;	break;

			DEFAULT:
				json& ref = *this;
				assert(ref.is_object());
				auto res = ref.find(it);

				if (res == ref.end())
				{
					try { this->get_entity(ref[it], it); }
					catch (json& j) { throw json({ {__FUNCTION__, j} }); }
					catch (invalid_argument& e) { throw json({ {__FUNCTION__, "property '"s + str + "' invalid_argument, " + e.what()} }); }
					catch (out_of_range& e) { throw json({ {__FUNCTION__, "property '"s + str + "' out_of_range, " + e.what()} }); }
					catch (...) { throw json({ {__FUNCTION__, "property '"s + str + "' does not exist!"} }); }

					res = ref.find(it);
					if (res == ref.end())
						throw json({ {__FUNCTION__, "entity '"s + it + "' does not exist in relations model!"s} });
				}
				jptr = &res.value();
			}

			while (prev < len)
			{
				pos = str.find_first_of('/', prev);
				if (pos == string::npos) pos = len;
				string it = str.substr(prev, pos - prev);
				prev = pos + 1;
				try { ref_in_json_to<val_type>(jptr, it); }
				catch (json& j) { throw json({ {__FUNCTION__, j} }); }
				catch (invalid_argument& e) { throw json({ {__FUNCTION__, "property '"s + str + "' invalid_argument, " + e.what()} }); }
				catch (out_of_range& e) { throw json({ {__FUNCTION__, "property '"s + str + "' out_of_range, " + e.what()} }); }
				catch (...) { throw json({ {__FUNCTION__, "property '"s + str + "' does not exist!"} }); }
			}

			return *jptr;
		}

		template<class val_type>
		json& val_or_ref_to(vm_ctx& $, json& ref)
		{
			switch (ref.type())
			{
			case json::value_t::string:	//	иерархический путь к json значению
				return string_ref_to<val_type>($, ref.get_ref<const string&>());

			case json::value_t::null:	//	местоимение проекции контекстной сущности
				return $.its;

			default:					//	если это не адрес то возвращаем значение
				return ref;
			}
		}

		static void  base_add_entity(vm& rmvm, vm_ctx& $)
		{
			string	ent_id = "";
			rmvm.add_entity($.obj, ent_id);
			$.sub = ent_id;
		}

	public:
		vm(database_api* db = nullptr)
			: json(json::object())
		{
			database_api::link(db);
			//	database_api
			add_binary_view(*this, "add_entity"s, base_add_entity, "Add new entity to database"s);
		}

		json& exec(json& its, json& ent)
		{
			vm_ctx $(its);
			try
			{
				exec($, ent);
			}
			catch (json& j) { $.throw_json(__FUNCTION__, j); }
			catch (json::exception& e) { $.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
			catch (std::exception& e) { $.throw_json(__FUNCTION__, "std::exception: "s + e.what()); }
			catch (...) { $.throw_json(__FUNCTION__, "unknown exception"s); }
			return its;
		}

		//	Исполнение сущности либо json байткода
		//	имеет прототип отличный от других контроллеров и не является контроллером
		//	рекурсивно раскручивает структуру проекции контроллера доходя до простых json или вызовов скомпилированных сущностей
		void exec(vm_ctx& $, json& ent)
		{
			binary_view_map_t& dict = *this;
			auto it = dict.find(&ent);

			if (it != dict.end())	//	это скомпилированная сущность?
			{
				try
				{
					auto x86view = it->second;
					(*x86view)(*this, $);
					return;
				}
				catch (json& j) { throw j; }
				catch (json::exception& e) { throw json("json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
				catch (std::exception& e) { throw json("std::exception: "s + e.what()); }
				catch (...) { throw json("unknown exception"s); }
			}
			else switch (ent.type())
			{
			case json::value_t::string:	//	иерархический путь к json значению
			{
				try
				{
					exec($, string_ref_to<rval>($, ent.get_ref<string const&>()));
					return;
				}
				catch (json& j) { throw json({ {ent.get_ref<string const&>(), j} }); }
			}

			case json::value_t::array:	//	лямбда вектор, который управляет последовательным изменением проекции сущности
			{
				int i = 0;
				for (auto& it : ent)
				{
					try
					{
						exec($, it);
						i++;
					}
					catch (json & j) { throw json({ {"["s + to_string(i) + "]"s, j} }); }
				}
				return;
			}

			case json::value_t::object:
			{
				auto& rel = ent.find("$rel");
				auto& end = ent.end();

				if (rel != end) //	это сущность, которую надо исполнить в новом контексте?
				{
					auto& obj = ent.find("$obj");
					auto& sub = ent.find("$sub");

					try {
						exec(
							vm_ctx(
								$.its,
								obj == end ? $.its : val_or_ref_to<rval>($, *obj),
								sub == end ? $.its : val_or_ref_to<lval>($, *sub),
								ent,
								$),
							val_or_ref_to<rval>($, *rel)
						);
					}
					catch (json & j) { throw json({ {"$rel"s, j} }); }
				}
				else //	контроллер это лямбда структура, которая управляет параллельным проецированием сущностей
				{
					for (auto& it : ent.items())
					{
						try
						{
							exec(
								vm_ctx(
									string_ref_to<lval>($, it.key()),
									$.obj,
									$.sub,
									$.ent,
									$.$),
								val_or_ref_to<rval>($, it.value())
							);
						}
						catch (json & j) { throw json({ {it.key(), j} }); }
					}

					/*ToDo:	надо переделать на параллельное проецирование
					struct callctx
					{
						vm_ctx	$;
						string&		key;
						json&		rel;
						callctx(vm_ctx& c, string& k, json& r) : $(c), key(k), rel(r) {}
					};

					vector<callctx>	vec;
					for (auto& it : rel.items())
					{
						string&	key = it.key();
						try { vec.push_back(callctx(vm_ctx(val_or_ref_to($, key), $.obj, $.sub, $.ent, $.ctx), key, val_or_ref_to($, it.value()))); }
						catch (string& error) { throw("\n view "s + key + " : "s + error); }
						catch (json::exception& e) { throw("\n view "s + key + " : "s + "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
						catch (std::exception& e) { throw("\n view "s + key + " : "s + "std::exception: "s + e.what()); }
						catch (...) { throw("\n view "s + key + " : "s + "unknown exception"s); }
					}

					parallel_for_each(begin(vec), end(vec), [](callctx& it) {
						try { exec(it.$, it.rel); }
						catch (string& error) { throw("\n view "s + it.key + " : "s + error); }
						catch (json::exception& e) { throw("\n view "s + it.key + " : "s + "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
						catch (std::exception& e) { throw("\n view "s + it.key + " : "s + "std::exception: "s + e.what()); }
						catch (...) { throw("\n view "s + it.key + " : "s + "unknown exception"s); }
					});*/
				}
				return;
			}

			case json::value_t::null:	//	null - означает отсутствие отношения, т.е. неизменность проекции
				return;

			default:	//	остальные простые типы есть результат исполнения отношения
				$.its = ent;
				return;
			}
		}

		//	добавление сущности с закэшированной x86 проекцией
		json& add_binary_view(json& entity, const string& name, const binary_view view, const string& description)
		{
			entity[name] = json::object();
			entity[name]["description"] = description;
			static_cast<binary_view_map_t&>(*this)[&(entity[name])] = view;
			return entity[name];
		}
	};
}
