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
Copyright © 2016 Vertushkin Roman Pavlovich <https://vk.com/earthbirthbook>.

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
/*
------------------------------------------------------------------------------

------------------------------------------------------------------------------
#/json
Формат json не является языком программирования МО, поэтому для исполнения
МО требуется десериализация json в сегмент данных РВМ, а так же загрузка базовых
словарей сущностей для данного типа РВМ.

------------------------------------------------------------------------------
#/RVM/exec
Вычисление результата исполнения экземпляра отношения происходит через интерпретацию
сущности (либо исполнение её json проекции) выступающей в качестве сущности-отношения
в данном экземпляре отношения. Что бы исполнить сущность, у неё должна быть проекция в
язык который понимает вычислитель (РВМ), например:
если используется JavaScript виртуальная машина, то у сущности должна быть
проекция в JavaScript. Если у сущности нет проекции в язык понимаемый имеющимся
вычислителем, то необходимо либо скомпилировать тело сущности (т.е. создать
проекцию в нужный язык), либо интерпретировать экземпляр отношения которым она является.
Сущность при исполнении в качестве отношения в экземпляре отношения может
создать в контексте свойство с именем языка исполнителя и скомпилировать себя
туда для непосредственного исполнения сейчас и/или в следующий раз.

При исполнении МО существует два потока:
	иерархически вертикальный - это поток байткода json
	иерархически горизонтальный - это поток данных в json.
Горизонтальные потоки это межслойное преобразование данных контроллерами MVC.
Вертикальные потоки это потоки управления интерпретацией json-байткода
контроллеров MVC виртуальной машиной.

------------------------------------------------------------------------------
#/Entity/ExecContext
Контекст исполнения сущности необходим для её исполнения, т.е. исполнения экземпляра отношения.
При исполнении МО контексты образуют собой древовидный стэк. Это достигается за счет того, что
каждый контект имеет ссылку на родительский контекст.

	Строковая адресация в будущей версии jsonRVM

Строка содержит в себе последовательную запись инструкций по перемещению указателя сущности в МО.
По умолчанию указатель установлен на текущую проекцию сущности.
Путь сущности — это набор действий пути. Действия пути могут содержать следующие элементы и операторы.

Знак доллара ($) указывает на смену контекстного адресного пространства.
После него должно следовать имя контекста.

Имена ключей. Например, $.name или $."first name". Если имя ключа начинается со знака доллара или
содержит специальные символы, например пробелы, заключите его в кавычки.
Элементы массива. Например, $.product[3]. Массивы отсчитываются от нуля.
Оператор "точка" (.) указывает на свойство сущности.
Например, в $.people[1].surname surname является дочерним элементом people.

Знак октоторп (#) указывает на текущая модель отношений в глобальном пространство имён, примеры: "#.global_var"

"$ent" - контекстная сущность, примеры: "$ent.$obj", "$ent.some_flag"
"$sub" - контекстный субъект , примеры: "$sub.some_attribue", "$sub[43]"
"$obj" - контекстный объект, примеры: "$obj.arg1", "$obj[1]"
"", "$view" - (по умолчанию) локальное адресное пространство , т.е. текущее значение проекции сущности, примеры: ".local_var", "$view.[353]"

"$$ent" - родительская контекстная сущность, примеры: "$$ent.$obj", "$$ent.some_flag"
"$$sub" - родительская контекстный субъект , примеры: "$$sub.some_attribue", "$$sub[43]"
"$$obj" - родительская контекстный объект, примеры: "$$obj.arg1", "$$obj[1]"
"$$view" - родительское локальное адресное пространство, т.е. текущее значение проекции сущности, примеры: "$$view.local_var", "$$view.[353]"

------------------------------------------------------------------------------
#/RM/ExecutionStack
Стэк исполнения МО это древовидный стэк контекстов исполнения сущностей - экземпляров отношений.

------------------------------------------------------------------------------
#/RM/jsonView/types
		Соответсвие json значений и проекций Модели Отношений:

1. Number  - стабильная топологически замкнутая (т.е. сама в себя) проекция отношения double|int32|int64|uint32|uint64
2. Boolean - стабильная топологически замкнутая проекция отношения bool
3. String  - стабильная топологически замкнутая проекция отношения string, либо имя сущности
4. Object  - стабильная топологически замкнутая проекция отношения вмещения структуры, либо сущность если есть поля "$obj" или "$rel" или "$sub" или "/"
5. Array   - стабильная топологически замкнутая проекция цепочки отношений последовательного следования - ","
6. Null    - пустое пространство как потенциал вмещения проекции

Null и Array - это не совсем типы, они принципиально отличаются от типов и их обработка идёт отдельными кейсами if в РВМ:
1. Array - это контейнер проекцией одного или разного типа
2. Null - это отсутствие проекции любого типа

Начало цепочки отношений "," всегда есть Null:
((Null --","-> SomeEnt1) --","-> SomeEnt2) === jsonArray[SomeEnt1, SomeEnt2]
Это объясняется тем, что начало цепочки это то место перед которым нет другого
начального элемента, т.е. пустое пространство обозначаемое через Null.

------------------------------------------------------------------------------
#/RVM/init
		Алгоритм исполнения json после загрузки:

1. json после десериализации непостредственно представляет собой сегментом данных RVM
2. корневое значение json всегда есть корневая сущность загруженной Модели Отношений,
	которую необходимо исполнить
3. поля указывающие на субъект, отношение и объект могут либо содержать
	сущность по значению (типы json значений: объект, массив)
	либо содержать ссылки на сущность (типы json значений: строка, число)
	либо признак ссылки на контекстную сущность (типы json значений: null)
	либо значение boolean
4. Местоимения при нахождении их проекции возвращают ссылку на элементы контекста,
	проекции которых были найдены до вызова контроллера.

------------------------------------------------------------------------------
#/exec
Самое основное отношение это отношение "#/exec", оно исполняет сущность объекта указанного
в поле "$obj" и записывает полученную проекцию в поле "$sub" субъекта.
Именно это отношение исполняется для кажого поля json объекта, когда он указан
в качестве отношения сущности.

------------------------------------------------------------------------------
		Правила json скрипта (проекции МО в json):

1. json представляет собой агрегат проекций сущностей т.е. иерархию МО по сущности-субъекту,
2. местоимение субъекта "$sub" может являеться сущностью агрегатом проекции
3. запуск скрипта это вычисление проекции корневой сущности
4. внутри корневой json-сущности лежат разные проекции её свойств, либо...
5. внутри корневой json проекции лежит jsonView корневой сущности
6. у локальных не корневых проекций субъект "$sub" равен сущности в которой они лежат
7. если в json объекте есть поле "$rel", то значит этот объект описывает сущность МО
8. проекция состоит из проекций, т.о. получается иерархичность агрегирования

Внутри сущности находится иерархия по сущности-субъекту.

Новые сущности могут создаваться в результате проецирования, т.е. по сути являются тоже проекциями.

------------------------------------------------------------------------------
		Варианты взаимоотношений узлов json:

1. Проекция в проекции
2. Сущность в сущности (в полях "$obj", "$rel", "$sub")
3. Сущность в проекции (в объекте или массиве)
4. Проекция в сущности (в полях "$obj", "$rel", "$sub")
5. Строковая или адресная ссылка на сущность в сущности (в полях "$obj", "$rel", "$sub")

------------------------------------------------------------------------------
		О свойствах (атрибутах) и элементах сущности:

У сущности кроме её внутренней троичной структуры есть свойства (её проекции) и элементы свойств.
Элементами сущности являются разные её свойства.
Элементы объекта и есть его свойства если этот объект не сущность.
Свойства (properties) сущности есть элементы (elements) её представления (view).
(json properties are elements of its view).

https://books.google.ru/books?id=VfcX9wJEH3YC&pg=PT42&redir_esc=y&hl=ru#v=onepage&q&f=false

Сущность это то что является субъекту при наблюдении объекта, через призму их связывающего отношения.
Таким образом сущьность есть результат отношения связывающего объект и субъект.
Субъект вмещает в себя явление сущности объекта.

Результат исполнения сущности это проекция в триаде MVC, но т.к. МО это самокомпилирующийся иерархический многослойный MVC,
то субъект у сущности это родительский view вмещающий проекцию данной сущности.
Таким образом результат проецирования сущности сохраняется в поле "", значение которого состоит из отдельных
проекций агрегируемых сущностей. Следовательно если объект json имеет признак того, что это сущность,
то данный объект json не должен иметь в себе полей с текстовыми названиями, а только: "id", "$obj", "$rel", "$sub", "/".
Решением данной проблемы может быть вариант когда текстовые поля внутри объекта json будут хранить в себе кэши проекций
данной сущности другими контроллерами (сущностями-отношениями), например:

{
   "id": "this entity english name",
   "RusView": "русское название этой сущности",
   "jsView": "this entity java script view",
   "C#view": "this entity C# code view",
   "$obj": "object entity model",
   "$rel": "relation entity model",
   "$sub": "subject entity model"
}

------------------------------------------------------------------------------
		Виды топологий сущностей:

1. EntId = SubId = RelId = ObjId - статическое собственное значение,
	сущность топологически замкнута сама на себя через себя
2. EntId = SubId = ObjId - динамическое собственное значение,
	сущность топологически замкнута сама на себя,
	через другую сущность проектор - сущность отношение
3. EntId = SubId - ссылка или хранилище проекции,
	сущность топологически не замкнута сама на себя,
	сущность сохраняет в себе проекцию другой сущности формируемой
	другой сущность проектором - сущность отношением
4. EntId = ObjId - источник проекции для субъективного восприятия другими сущностями,
	сущность топологически не замкнута сама на себя,
	сущность формирует собственную проекцию,
	через другую сущность проектор - сущность отношение
------------------------------------------------------------------------------

		Семантика значений разных типов в полях "$sub", "$rel", "$obj" json объекта описывающего сущность:
------------------------------------------------------------------------------
 тип значения |                     сементика значения
------------------------------------------------------------------------------
  Number      | адрес объекта типа json в памяти
------------------------------------------------------------------------------
  Boolean     | непосредственное значение = true/false
------------------------------------------------------------------------------
  String      | иерархический путь к сущности относительно текущего контекста
			  | исполнения
------------------------------------------------------------------------------
  Object      | json объект определяющий непосредственное значение сущности
------------------------------------------------------------------------------
  Array       | json массив определяющий непосредственное значение сущности
------------------------------------------------------------------------------
  Null        | значение по умолчанию, пустой указатель на сущность
			  | обозначающий топологическая замкнутость на текущую проекцию
			  | сущности в контексте исполнения EntView[""]
------------------------------------------------------------------------------
  Entity      | непосредственное описание другой сущности
------------------------------------------------------------------------------

		Семантика json значений при исполнении  (json как байткод RVM):
------------------------------------------------------------------------------
 тип значения |                     сементика значения
------------------------------------------------------------------------------
  Number      | адрес скомпилированного тела сущности, которое необходимо исполнить RVM
------------------------------------------------------------------------------
  Boolean     | флаг разрешения проецирования объекта в субъект
			  | true : спроецировать
			  | false : не проецировать
------------------------------------------------------------------------------
  String      | иерархический путь к json значению относительно контекста исполнения,
			  | которое необходимо исполнить RVM
------------------------------------------------------------------------------
  Object      | множество пар "имя поля": значение поля, где:
			  |  значение поля - иерархический путь к json значению относительно
			  |   контекста исполнения, которое необходимо исполнить
			  |  имя поля - иерархический путь к json значению относительно
			  |   контекста исполнения, куда необходимо спроецировать сущность
			  | вычисление проекций множества пар может происходить параллельно, в многопоточном режиме
------------------------------------------------------------------------------
  Array       | массив значений json которые необходимо последовательность исполнить
			  | в текущем контексте исполнения, при этом результат предыщущего
			  | исполнения всегда доступен для следующей исполняющейся сущности
			  | как текущее значение проекции
------------------------------------------------------------------------------
  Null        | возврат текущей контекстной проекции в саму себя, т.е. по сути
			  | отсутствие действия над текущей проекцией в контексте исполнения
------------------------------------------------------------------------------
  Entity      | сущность, для которой надо создать дочерний контекст и исполнить
------------------------------------------------------------------------------

		Программирование на МО

Контекстно зависимая проекция сущности может быть получена в результате
исполнения (или вычисления) отношения "$rel" в контексте субъекта "$sub", объекта "$obj".
Доступ непосредственно к текущего значения проекции осуществляется через строковый адрес "$view".
Сущность может использовать контекстно зависимые местоимения в своей модели,
таким образом реализуется контекстная зависимость проекции сущности.

Аналогичное выражение на языке С++ выглядело бы следующим образом:

json   context_entity_value;
context_entity_value = $sub->$rel( context_entity_value, $obj );

В отличие от обычных языков программирования, в МО стек контекстов вызовов и стэк
локальных адресных пространств (локальные переменные) принципиально разделены.

Локальное адресное пространство при исполнении сущности-отношения указывается всегда явно,
а стэк контекстов исполнения в общем случае древовидный, т.е. по сути и не является стэком,
что обеспечивает лёгкое многопоточное исполнение сущностей.

Так же при исполнении сущности-отношения можно указать в каком контексте она будет исполняться, в текущем или родительском
аргументы тоже группируются в независимый от вызова пакет, и может быть многократно использован в контексте исполнения.
В обычных языках стэк контекстов (агрументы вызова, адреса возвратов из функции) объединён
с локальными адресными пространствами функций, поэтому очень важно четко знать где и
что записано в стэке, четко соблюдать call convention что бы случайно не использовать некую переменную или аргумент как адрес возврата.

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
	////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// Загрузка руководства по использованию виртуальной машины
	/// </summary>
	/// <param name="j">Ссылка на модель отношений куда необходимо подгрузить руководство</param>
	inline void	import_rmvm_help_to(json& j)
	{
		//j["/Help/template/term"_json_pointer] = R"(
		//)";

		////////////////////////////////////////////////////////////////////////////////

		j["/Help/RM/Dictionary"_json_pointer] = R"(
		Сокращения принятые в тексте

		СОП - сущностно ориентированное программирование
		МО - модель отношений
		РВМ - реляционная виртуальная машина, исполняющая модель отношений

		EOP - entity oriented programming
		RVM - relations virtual machine
		RM - relations model)";

		////////////////////////////////////////////////////////////////////////////////

		j["/Help/RM/jsonView"_json_pointer] = R"(
	Представление МО в json(json проекция МО) - это объектное представление модели отношений в сегменте данных RVM.
	Проекция сущности - это то как сущность представляется субъекту наблюдения, поэтому json представление хранится в контексте отображения.

	Представление МО в json бывает 2х видов :
	
	1. развёрнутая модель : отдельный json объект для описания каждой сущности в виде экземпляра отношения

	2. компактная модель : в виде json объектов и массивов

	В древовидной структуре json, там где значения проекций сущностей известно и закэшировано,
находятся обычные значения json.В той части json структуры, где требуется вычислить json проекцию МО,
находится json объект с дополнительными свойствами "$obj", "$rel", "$sub", "/".
Такой объект интерпретируется RVM как сущность МО, которую возможно исполнить и
найти её json проекцию.Сущности могут иметь дополнительные поля в которых
закэшированы проекции свойств сущности или заданы сущности определяющие проекции данных свойств.

Любое значение json можно представить эквивалентной МО, json проекция которой
будет идентична этому json значению.)";

	
		////////////////////////////////////////////////////////////////////////////////

		j["/Help/RM/MHMVC"_json_pointer] = R"(
	MHMVC(Multilayered Hierarchical Model - View - Controller), т.е.Многослойный
Иерархический Модель – Вид – Контроллер.В данной концепции иерархия представлений
одного слоя может служить в качестве иерархии модели для следующего слоя и наоборот,
что позволяет гибко реализовать многослойную архитектуру приложения.

При реализации этой концепции в коде, было так же применено тактическое
решение, объединить в каждом слое данных классы представлений и классы контроллеров
через наследование представлений от контроллеров для упрощения архитектуры и
уменьшения общего количества классов в коде в 2 раза.)";

		////////////////////////////////////////////////////////////////////////////////

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
