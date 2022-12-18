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
#include <mutex>
#include <execution>
#include <algorithm>
#include "database_api.h"
#include "nlohmann/json.hpp"
#include "str_switch/str_switch.h"
#include "string_type.h"

/*
		План переделки:

1. заменяем 
{
	$obj:
	$rel:
	$sub:
}
на
{
	$ent: {
		$sub:
		$obj:
	}
	$rel: {
		$obj:
		$sub:
	}
}
т.е. на 
{
	"$source":
	"$target":
}
2. заменяем адреса типа
	#/point/x
на 
	point[x]

3. переделываем структуру контекста на использование типа json
*/

namespace rm
{
	using namespace std;
	using namespace nlohmann;

	class vm;
	//	64 bit
#ifdef WIN32
#define IMPORT_RELATIONS_MODEL		"?import_relations_model_to@rm@@YAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEAVvm@1@@Z"
	__declspec(dllexport) const string& import_relations_model_to(vm& rmvm);
	typedef const string& (*InitDict)(vm& rmvm);
	static InitDict	You_must_define_import_relations_model_to_function_in_your_RM_dictionary = import_relations_model_to;
#endif

	//	Старая версия: 
	//	Контекст исполнения сущности
	//	инстанцированная проекция модели сущности
	//	экземпляр сущности
	struct vm_rel
	{
		vm_rel& $;	//	parent context reference
		json& ent;	//	context entity reference сущность, is model for instance
		json& obj;	//	context object reference
		json& rel;	//	relation instance value
		json& sub;	//	context subject reference

		vm_rel(json& Rel, json& Obj, json& Sub, json& Ent, vm_rel& Ctx)
			: rel(Rel), obj(Obj), sub(Sub), ent(Ent), $(Ctx) {}

		vm_rel(json& Rel, json& Ent)
			: rel(Rel), obj(Rel), sub(Rel), ent(Ent), $(*this) {}

		vm_rel(json& Rel)
			: rel(Rel), obj(Rel), sub(Rel), ent(Rel), $(*this) {}

		void throw_json(const string& function, const json& error) const
		{
			json	j;
			j[function] = error;
			j["vm_rel"]["$rel/"] = rel;
			if (obj != rel) j["vm_rel"]["$obj/"] = obj;
			if (sub != rel) j["vm_rel"]["$sub/"] = sub;
			if (ent != rel) j["vm_rel"]["$ent/"] = ent;
			throw j;
		}
	};

	/*struct vm_sub;	//	is rel->SO
	struct vm_rel;	//	is ER->sub

	//	Субъект отношения
	struct vm_sub	//	is rel->SO
	{
		vm_rel* rel{nullptr};	//	parent context reference
		json&	SO;		//	entity
		
		vm_sub(json& ent) : SO(ent) {}
		vm_sub(vm_rel* Rel, json& ent) : rel(Rel), SO(ent) {}

		void throw_json(const string& function, const json& error) const {
			json	j;
			j[function] = error;
			j["vm_sub"]["SO/"] = SO;
			throw j;
		}
	};

	//	Контекст отношения есть отношение объекта субъекту
	struct vm_rel	//	is ER->sub
	{
		json&	ER;	//	construct object
		vm_sub* sub{nullptr};	//	parent subject reference

		vm_rel(json& obj) : ER(obj) {}
		vm_rel(json& obj, vm_sub* Sub) : ER(obj), sub(Sub) {}

		void throw_json(const string& function, const json& error) const {
			json	j;
			j[function] = error;
			j["vm_rel"]["ER/"] = ER;
			throw j;
		}
	};*/

	using binary_view = void (*)(vm& rmvm, vm_rel& $);
	using binary_view_map_t = map<json const*, binary_view>;

	struct base_entity {
		const json_pointer<json> path{};
		const string description{};
		static void	view(vm& rmvm, vm_rel& $) {}
	};
	
	/*
	Судя по тому что в json проекции семантика уровней дерева чередуется, работа авм имеет 2 фазы: работа с древовидным контекстом исполнения и работа с амо 
	Это проецирование и исполнение
	Анализ и синтез
	Авм это субъект
	Амо это объект
	Субъективация это фаза анализа АМО, она приводит к созданию новых сущностей и нового контекста исполнения
	Объективация это фаза синтеза АМО, это исполнение сущности отношения, т.е. создание новых отношений в АМО
	Из-за чередования семантики уровней дерева json, похоже семантика местоимений тоже чередуется
	Местоимения вышестоящих уровней реализуются через выход в вышестоящие контексты закрывающей скобкой "]"
	Местоимениями объекта и субъекта являются спецсимволы "," и "."
	так как верхний уровень АВМ это связь субъект->объект то местоимения могут адресоваться либо в иерархии субъекта либо в иерархии объекта 
	*/


	class vm : protected database_api, public json, public binary_view_map_t
	{
	////////////////////////////// VERSION //////////////////////////////
	public: static const inline string version = "3.0.0"s;
	////////////////////////////// VERSION //////////////////////////////

	private:
		struct rval { static const bool is_lval{ false }; };
		struct lval { static const bool is_lval{ true }; };

		template<class val_type>
		void	ref_in_json_to(json*& jptr, const string& it) {
			json& ref = *jptr;

			switch (ref.type()) {
			case json::value_t::object:
				if constexpr (val_type::is_lval) {
					jptr = &ref[it];
					return;
				} else {
					auto rel = ref.find(it);

					if (rel == ref.end())
						throw json({ {it, *jptr} });
					
					jptr = &*rel;
					return;
				}

			case json::value_t::array:
				if constexpr (val_type::is_lval) {
					jptr = &ref[std::stoul(it)];
					return;
				} else {
					auto id = std::stoul(it);
					if (id >= ref.size())
						throw json({ {it, *jptr} });

					jptr = &ref[id];
					return;
				}

			case json::value_t::null:
				if constexpr (val_type::is_lval) {
					int& _Errno_ref = errno; // Nonzero cost, pay it once
					const char* _Ptr = it.c_str();
					char* _Eptr;
					_Errno_ref = 0;
					unsigned index = strtoul(_Ptr, &_Eptr, 10);

					if (_Errno_ref == ERANGE)
						throw json({ {it, *jptr} });

					if (_Ptr == _Eptr)
						jptr = &ref[it];
					else
						jptr = &ref[index];

					return;
				}

			default:
				throw json({ {it, *jptr} });
			}
		}

		template<class val_type>
		json& string_ref_to(vm_rel& $, const string& str) {
			json* jptr;
			size_t	len = str.length();
			size_t	pos = str.find_first_of('/', 0);

			if (pos == str.npos) pos = len;

			size_t	prev = pos + 1;
			string	it = str.substr(0, pos);

			SWITCH(it) {
				CASE("$$$$ent") : jptr = &$.$.$.$.ent;	break;
				CASE("$$$$sub") : jptr = &$.$.$.$.sub;	break;
				CASE("$$$$obj") : jptr = &$.$.$.$.obj;	break;
				CASE("$$$$rel") : jptr = &$.$.$.$.rel;	break;
				CASE("$$$ent") : jptr = &$.$.$.ent;	break;
				CASE("$$$sub") : jptr = &$.$.$.sub;	break;
				CASE("$$$obj") : jptr = &$.$.$.obj;	break;
				CASE("$$$rel") : jptr = &$.$.$.rel;	break;
				CASE("$$ent") : jptr = &$.$.ent;	break;
				CASE("$$sub") : jptr = &$.$.sub;	break;
				CASE("$$obj") : jptr = &$.$.obj;	break;
				CASE("$$rel") : jptr = &$.$.rel;	break;
				CASE("$ent") : jptr = &$.ent;	break;
				CASE("$sub") : jptr = &$.sub;	break;
				CASE("$obj") : jptr = &$.obj;	break;
				CASE("$rel") : jptr = &$.rel;	break;

			DEFAULT:
				json& ref = *this;
				assert(ref.is_object());
				auto res = ref.find(it);

				if (res == ref.end()) {
					try { this->get_entity(ref[it], it); }
					catch (json& j) { $.throw_json(__func__, j); }
					catch (invalid_argument& e) { $.throw_json(__func__, "entity '"s + str + "' invalid_argument, " + e.what()); }
					catch (out_of_range& e) { $.throw_json(__func__, "entity '"s + str + "' out_of_range, " + e.what()); }
					catch (...) { $.throw_json(__func__, "entity '"s + str + "' does not exist!"); }

					res = ref.find(it);
					if (res == ref.end())
						$.throw_json(__func__, "entity '"s + it + "' does not exist in relations model!"s);
				}
				jptr = &res.value();
			}

			while (prev < len) {
				pos = str.find_first_of('/', prev);
				if (pos == string::npos) pos = len;
				string it = str.substr(prev, pos - prev);
				prev = pos + 1;
				try { ref_in_json_to<val_type>(jptr, it); }
				catch (json& j) { $.throw_json(__func__, j); }
				catch (invalid_argument& e) { $.throw_json(__func__, "property '"s + str + "' invalid_argument, " + e.what()); }
				catch (out_of_range& e) { $.throw_json(__func__, "property '"s + str + "' out_of_range, " + e.what()); }
				catch (...) { $.throw_json(__func__, "property '"s + str + "' does not exist!"); }
			}

			return *jptr;
		}

		template<class val_type>
		json& val_or_ref_to(vm_rel& $, json& val) {
			if (val.is_object()) {
				//	это ссылка на json значение?
				if (auto ref = val.find("$ref"); ref != val.end()) {
					//	иерархический путь к json значению?
					if (ref->is_string())
						return string_ref_to<val_type>($, ref->get_ref<const string&>());
					else
						throw json({ {"$ref", *ref} });
				}
			}

			//	если не ссылка, возвращаем значение
			return val;
		}

	public:
		struct database_api_add_entity : public base_entity	{
			const json_pointer path{ "/add_entity" };
			const string description{ "Add new entity to database" };
			static void	view(vm& rmvm, vm_rel& $) {
				string	ent_id = "";
				rmvm.add_entity($.obj, ent_id);
				$.sub = ent_id;
			}
		};

		struct rmvm_version : public base_entity {
			const json_pointer path{ "/rmvm/version" };
			const string description{ "Version of rmvm" };
			static void	view(vm& rmvm, vm_rel& $) { $.rel = vm::version; }
		};

		vm(database_api* db = nullptr) : json(json::object()) {
			database_api::link(db);
			//	rmvm base voc
			*this << rmvm_version() << database_api_add_entity();
		}

		//	объективация сущности в отношение: Ent->Rel
		json& objectify(json& ent, json& rel) {
			//	инициализируем контекстное отношение
			vm_rel $(rel);
			try { objectify(ent, $); }
			catch (json& j) { $.throw_json(__func__, j); }
			catch (json::exception& e) { $.throw_json(__func__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
			catch (std::exception& e) { $.throw_json(__func__, "std::exception: "s + e.what()); }
			catch (...) { $.throw_json(__func__, "unknown exception"s); }
			return rel;
		}

		/*
		в часть субъекта относятся сущности, которые есть шаблоны смешивающие контекстный субъект и объект

		контекст исполнения это отношение двух текущих регистров $obj/$sub
		в этом контексте объективируется сущность

		было бы неплохо в проекции json иметь возможность задавать боковые связи для $obj/$sub
		*/

		//	Исполнение сущности либо json байткода
		//	имеет прототип отличный от других контроллеров и не является контроллером
		//	рекурсивно раскручивает структуру проекции контроллера доходя до простых json или вызовов скомпилированных сущностей
		//	отношение есть контекст
		void objectify(json& ent, vm_rel& $) {
			binary_view_map_t& dict = *this;
			auto it = dict.find(&ent);

			if (it != dict.end()) {	//	это скомпилированная сущность?
				try	{ (*it->second)(*this, $); }
				catch (json& j) { throw j; }
				catch (json::exception& e) { throw json("json::exception: "s + e.what() + ", id: "s + to_string(e.id)); }
				catch (std::exception& e) { throw json("std::exception: "s + e.what()); }
				catch (...) { throw json("unknown exception"s); }
				return;
			} else switch (ent.type()) {
			//	иерархический путь к json значению
			//	пример: point[x]
			case json::value_t::string:
				try	{ objectify(string_ref_to<rval>($, ent.get_ref<string const&>()), $); }
				catch (json& j) { throw json({ {ent.get_ref<string const&>(), j} }); }
				return;

			//	лямбда вектор, который управляет последовательным изменением проекции сущности
			//	[a,b,c] === [a][b][c]
			case json::value_t::array: {
				auto it = ent.begin();
				auto end = ent.end();
				try { for (; it != end; ++it) objectify(*it, $); }
				catch (json& j) { throw json({ {"["s + to_string(it - ent.begin()) + "]"s, j} }); }
				return;
			}

			case json::value_t::object:	{
				auto end = ent.end();

				if (auto ref = ent.find("$ref"); ref != end) {	//	это ссылка на json значение
					if (ref->is_string()) {
						try { objectify(string_ref_to<rval>($, ref->get_ref<string const&>()),$); }
						catch (json& j) { throw json({ {ref->get_ref<string const&>(), j} }); }
					} else
						throw json({ {"$ref", *ref} });
				} else if (auto rel = ent.find("$rel"); rel != end) {	//	это сущность, которую надо исполнить в новом контексте?
					auto obj = ent.find("$obj");
					auto sub = ent.find("$sub");

					try {
						objectify(
							val_or_ref_to<rval>($, *rel),
							vm_rel(
								$.rel,
								obj == end ? $.rel : val_or_ref_to<rval>($, *obj),
								sub == end ? $.rel : val_or_ref_to<lval>($, *sub),
								ent,
								$)
						);
					}
					catch (json & j) { throw json({ {"$rel"s, j} }); }
				} else {	//	контроллер это лямбда структура, которая управляет параллельным проецированием сущностей
					auto it = ent.begin();

					try {
						for (; it != end; ++it) {
							objectify(
								val_or_ref_to<rval>($, it.value()),
								vm_rel(
									string_ref_to<lval>($, it.key()),
									$.obj,
									$.sub,
									$.ent,
									$.$)
							);
						}
					}
					catch (json& j) { throw json({ {it.key(), j} }); }					
				}
				return;
			}

			default:	//	остальные простые типы есть результат исполнения отношения
				$.rel = ent;	//	???

			case json::value_t::null:	//	null - означает отсутствие отношения, т.е. неизменность проекции
				return;
			}
		}

		/// <summary>
		/// Добавление в базовый словарь РВМ сущности с закэшированной бинарной проекцией
		/// </summary>
		/// <typeparam name="base_entity_t">Тип базовый сущности</typeparam>
		/// <param name="bent">Экземпляр базовой сущности</param>
		/// <returns>Ссылка на виртуальную машину</returns>
		template<class base_entity_t = base_entity>
		vm&	operator << (const base_entity_t& bent)	{
			json& ent = (*this)[bent.path] = { { "description", bent.description } };
			static_cast<binary_view_map_t&>(*this)[&(ent)] = base_entity_t::view;
			return *this;
		}

		/// <summary>
		/// Добавление в базовый словарь РВМ сущности с закэшированной бинарной проекцией
		/// </summary>
		/// <param name="entity"></param>
		/// <param name="name"></param>
		/// <param name="view"></param>
		/// <param name="description"></param>
		/// <returns></returns>
		json& add_base_entity(json& entity, const string& name, const binary_view view, const string& description) {
			entity[name] = { { "description", description } };
			static_cast<binary_view_map_t&>(*this)[&(entity[name])] = view;
			return entity[name];
		}
	};
}
