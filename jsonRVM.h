/*
The MIT License (MIT)

Copyright © 2016 Vertushkin Roman Pavlovich

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the “Software”), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

jsonRVM - json Relation (Model) Virtual Machine
https://
*/
/*
Представление модели отношений в json это есть шаблон проекции модели отношений,
там где структура определена там находятся стандартные узлы json дерева, в том же узле где требуется
достроить json проекцию, находится объект с дополнительными свойствами "<-", "()", "->", такой объект
трактуется виртуальной машиной как сущность, для которой требуется найти отношение. Сущности так же
могут иметь дополнительные собственные свойства в которых закэшированы проекции или заданы сущности
создающие проекции данных свойств.

После загрузки json файла, каждый узел дерева json:

1. есть проекция сущности, если это не объект с полем "<-", "()", "->", "="
2. является агрегатором свойств - разультатов проекций сущностей элементов
3. если это сущность, может опционально содержать шаблон проекции в поле "jsonView"
4. если это сущность, может опционально содержать субъект "->"
5. если это сущность, может опционально содержать объект "<-"
6. если это сущность и она содержит "jsonView", то её собственная проекция закэширована, и её не надо вычислять, а только подставить значения в шаблон
7. подразумевается что корневой объект это либо сущность, либо шаблон её проекции, т.е. только значение поля "jsonView"

Любой узел json можно представить эквивалентной моделью отношений, проекция которой будет равна значению этого узла.

Под термином «сущность» следует понимать «сущую вещь», проще говоря, «что-то». (Из словаря)
В нашем случае это может быть и объект и функция (например конструктор объекта).

Соответсвие типов json экземплярам отношений:

1. Number  - топологически замкнутая проекция отношения double|int32|int64
2. Boolean - топологически замкнутая проекция отношения bool
3. String  - топологически замкнутая проекция отношения string_t, либо имя сущности
4. Object  - топологически замкнутая проекция отношения, либо сущность если есть поля "<-" или "()" или "->"
5. Array   - топологически замкнутая проекция цепочки отношений "," или "next"
6. Null    - топологически замкнутая пустая проекция

Null и Array - это не совсем типы, они принципиально отличаются от типов и их обработка идёт отдельными кейсами if
1. Array - это контейнер объектов одного или разного типа
2. Null - это отсутствие объекта любого типа

Начало цепочки отношений "," всегда есть Null:
Null --","-> SomeEnt === jsonArray = [SomeEnt]
Это объясняется тем что начало цепочки это то место перед которым нет другого начала, т.е. пустое место.

Алгоритм исполнения скрипта после загрузки:

1. добавление каждой json сущности уникального имени-идентификатора
2. кэширование адресов всех узлов дерева json, которые являются сущностями

Пояснения:

1. дефолтное отношение это отношение =, оно сохраняет проекцию сущности объекта указанного в поле "<-" в поле "jsonView" субъекта, указанного в поле "->"
"jsonView" это результат проецирования, который контроллер записывает в:
->["jsonView"] т.е.	View[controller_name]
что и куда и нужно ли записывать в сущности субъекте, контроллер определяет сам
перед исполнением, необходимо проверить наличие закэшированного значения в View[controller_name]
View агрегирует View не по значению а по ссылке, само значение лежит в экземпляре отношения.

Правила json скрипта:

1. json представляет собой агрегат представлений т.е. иерархию по сущности-субъекту,
2. местоимение субъекта "->" является сущностью агрегатом проекции
3. запуск скрипта это получение проекции корневого объекта(ов)
4. внутри корневой json сущности лежат разные её проекции, либо...
5. внутри корневой json проекции лежит jsonView корневой сущности
6. у локальных не корневых представлений "->" = сущности в которой они лежат
7. если в объекте json есть поле "()", то значит это сущность, проекцию которой надо подставить вместо этого объекта
8. проекция состоит из проекций, т.о. получается иерархичность
Внутри проекции находятся другие проекции.
Внутри сущности находится иерархия по сущности-субъекту.

Новые сущности могут создаваться в результате проецирования, т.е. по сути являются тоже проекциями.

5 варианта взаимоотношений узлов json:

1. Проекция в проекции
2. Сущность в сущности (в полях "<-", "()", "->")
3. Сущность в проекции (в объекте или массиве)
4. Проекция в сущности (в полях "<-", "()", "->", "jsonView")
5. Ссылка на сущность в сущности (в полях "<-", "()", "->")
6. Ссылка на сущность в проекции (в поле "=")

О свойствах (атрибутах) и элементах сущности:

У сущности есть как свойства так и элементы.
Элементами сущности являются разные её свойства.
Элементы объекта и есть его свойства если этот объект не сущность.
Свойства (properties) сущности есть элементы (elements) её представления (view).
(Entity properties are elements of its view).

https://books.google.ru/books?id=VfcX9wJEH3YC&pg=PT42&redir_esc=y&hl=ru#v=onepage&q&f=false

Результат исполнения сущности это проекция в триаде MVC, но т.к. МО это иерархический многослойный MVC,
то субъект у сущности это родительский View вмещающий проекцию данной сущности.
Таким образом результат проецирования сущности сохраняется в поле "jsonView", значение которого состоит из отдельных
проекций агрегируемых сущностей. Следовательно если объект json имеет признак того, что это сущность,
то данный объект json не должен иметь в себе полей с текстовыми названиями, а только: "id", "<-", "()", "->", "=".
Решением данной проблемы может быть вариант когда текстовые поля внутри объекта json будут хранить в себе кэши проекций
данной сущности другими контроллерами (сущностями-отношениями), например:

{
"id": "english name of entity 1",
"<-": "some object",
"()": "some relation",
"->": "some subject",
"jsonView": "some json value",
"RusView": "русское название сущности 1",
"jsView": "some java script",
"C#View": "some C# code"
}

Об адресации в json:

1. сущности имеют уникальные названия - английскую проекцию
2. представления имеют иерархическую адресацию по дереву представлнией, имена разделены точкой

Заметки:

1. кэширование проекции сущности осуществляется только явным образом, через отношение jsonView?
2. для сущностей указанных в качестве значений полей должна быть найдена и инстанцирована шаблонная проекция
3. для символьных ссылок указанных в качестве субъекта, отношения или объекта сущности должна быть найдена указанная сущность
4. если субъект, отношение или объект заданы непосредственным своим значением,
то для этой проекции должна быть создана сущность и возвращён её адрес

Общая схема модели отношений:

В МО может быть множество корневых сущностей.
У корневых сущностей EntId === SubId.

План атомарных переделок:

1. переделать адресацию в полях по умолчанию относительно контекста исполнения т.е. null->this, имена полей контекста:
1.1 "id" - символьный идентификатор сущности
1.2 "->" - субъект
1.3 "()" - отношение
1.4 "<-" - объект
1.5 "=" - возвращаемое значение (результат исполнения отношения)
1.6 "#" - корень МО

2.  вычисление проекций до вызова отношения (как решить проблему итераторов, циклов и фильтров?):
value&	subview = EV.GetSubView(EV._sv);
value&	objview = EV.GetObjView(EV._ov);
3. переделать под новый прототип все функции:
value& __fastcall (*x86View)(value& subview, value& objview, EntView& Ent);
4. полная и корректная поддержка семантики ссылок "$"
5. самопроецирование сущностей
6. проблема имени сущности "."
7. явное разделение адресаций на две:
7.1 целочисленная адресация в МО
7.2 символьная иерархическая адресация в проекции МО

тут надо разобраться где какая адресация допустима к применению:
для субъекта, отношения и объекта допустима только адресация в МО
допустима ли адресация к МО из шаблонов проекций? зачем она? и можно ли её заменить?
возможно целочисленная адресация является полным аналогом символьной, т.е. является прямой ссылкой?
либо целочисленная указывает на сущности, а символьная на кэшируемые проекции?
таким образом целочисленную адресацию можно закэшировать на стадии исполнения, если она используется внутри проекций?

8. переписать все комментарии к коду в нормальную документацию
9. создать словарные сущности: "<-", "->", "()"
10. реализовать ссылки совместимые с DoJo:
http://dojotoolkit.org/api/?qs=1.10/dojox/json/ref
http://dojotoolkit.org/reference-guide/1.10/dojox/json/ref.html
https://www.sitepen.com/blog/2008/06/17/json-referencing-in-dojo/
11. возможно реализовать совмемстимость с:
https://www.w3.org/TR/rdf-json/
https://www.w3.org/TR/2013/NOTE-rdf-json-20131107/
или http://json-ld.org/

Функция _jsonView делает подстановку местоимений и значений сущностей, подобным образом как
функция eval в PHPRVM.
В PHPRVM работу функции eval() можно разделить на 2 фазы:
1. подстановка переменных окружения из контекста
2. собственно исполнение кода

Тогда по аналогии jsonRVM перед исполнением контроллера, должна:
1. найти полную проекцию контроллера, в т.ч. по его шаблону если он есть в _jsonView()
2. исполнить итоговую проекцию в jsonExec()

При чтении json необходимо:

1. заменить все сущности, символьные ссылки и значения сущностей в полях ->, <-, () на индексы сущностей МО
2. для сложных символных ссылок построить МО, и заменить на индексы этих МО

Частичная проекция сущности это шаблон проекции.
(Инстанцирование шаблонной проекции контроллера при исполнении - клонирование контроллера)

Местоимения при нахождении их проекции возвращают ссылку на элементы контекста,
проекции которых были найдены до вызова контроллера.

Вопросы:

1. проверить можно ли всегда делать множественный вызов контроллера для множественного объекта?
нет, контроллер сам определяет что делать с субъектом и объектом


Семантика json типов значений поля "=" :

1 Number  - индекс сущности в МО, проекцию которой необходимо подставить
2 Boolean - прямой возврат
3 String  - иерархическое имя сущности в МО, проекцию которой необходимо подставить
4 Object  - прямой возврат
5 Array   - прямой возврат
6 Null    - прямой возврат
7 Entity  - сущность, проекцию которой необходимо подставить

Семантика json типов значений полей сущности "->", "()", "<-":

1. Number  - индекс сущности в МО
2. Boolean - jsonView
3. String  - иерархическое имя сущности в МО
4. Object  - jsonView
5. Array   - jsonView
6. Null    - this
7. Entity  - другая сущность

Семантика типов элементов json проекции при чтении/записи (json как сегмент данных):

1. Number  - число
2. Boolean - булевый тип
3. String  - строка
4. Object  - карта свойств
5. Array   - последовательность
6. Null    - отсутствие значения
7. Entity  - сущность, проекция которой даст искомое значение элемента сегмента данных

null инициирует исполнение relview

Семантика типов элементов проекции в json при исполнении  (json как сегмент кода):

1. Number  - индекс машинной команды jsonRVM
2. Boolean - true/false -> копирование проекции obj->sub/очистка проекции subview = null
3. String  - исполнение проекции сущности по её имени
4. Object  - параллельное проецирование карты свойств сущности
5. Array   - исполнение подпрограммы
6. Null    - помещение результата исполнения полследнего элемента последовательности (флаг exec)
7. Entity  - сущность, проекция которой даст значение элемента сегмента кода, который необходимо исполнить (по идее не должна быть в сегменте кода)

Семантика типов элементов проекции в json в array при исполнении  (json как сегмент кода):

1. Number  - число либо индекс машинной команды jsonRVM
2. Boolean - булевое число либо true/false -> копирование проекции obj->sub/очистка проекции subview = null
3. String  - строка либо символьная адресация в контексте исполнения
4. Object  - объект либо параллельное проецирование карты свойств сущности,
5. Array   - последовательность элементов которые необходимо исполнить в одном контексте что бы получить проекцию сущности
6. Null    - помещение результата исполнения последнего элемента последовательности (флаг exec)
7. Entity  - сущность, проекция которой даст значение элемента сегмента кода, который необходимо исполнить


Организация памяти:

используется индексированное хранилище jsonView для сущностей
при разборе корневого json поля sub, rel, obj заменяются на индексы сущностей в МО
сущности из массивов, объектов и свойств "=" выносятся в МО и заменяются на индексы сущностей в МО


Программирование на МО

Проекция сущности на json (её тело-шаблон jsonView) может быть получена в результате
исполнения (или нахождения)  rel в контексте sub и obj. Сущность может использовать
контекстно зависимые местоимения в своей модели, таким образом её проекция тоже может
включать контекстно зависимые части и быть шаблонной проекцией.

*/
#pragma once
#include <string>
#include <vector>
#include <map>
#include <cpprest\json.h>

using namespace std;
using namespace web;
using namespace web::json;
using namespace utility;

struct EntView;
typedef value& (*x86View)(EntView &Ctx);

bool	InitRVMVoc(EntView &EV);
value&	_jsonView(EntView &EV);


__forceinline value	EntRef(int EntId)
{
	value	ref = value::object();
	ref[L"$ref"] = value(EntId);
	return ref;
}


template<class _T>
vector<_T> split(const _T& str, const _T& delim)
{
	vector<_T> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == _T::npos) pos = str.length();
		_T token = str.substr(prev, pos - prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}


struct EntView
{
	value&		RM;
	map<wstring, value*> EM;
	EntView*	parent;		//	родительский контекст
	value&		ent;		//	ссылка на сущность
	value&		sub;		//	ссылка на субъект
	//value&	rel;		//	ссылка на отношение
	value&		obj;		//	ссылка на объект
	value*		subview;	//	указатель на проекцию субъекта в контекст
	value*		objview;	//	указатель на проекцию объекта в контекст
	value*		jsonView;	//	указатель на проекцию - результат
	value		_sv, _rv, _ov;

	EntView(EntView* pparent, value& ent_ref, value& sub_ref, value& rel_ref, value& obj_ref, value& result_ref)
		: RM(pparent->RM), parent(pparent), ent(ent_ref), sub(sub_ref)/*, rel(rel_ref)*/, obj(obj_ref), subview(NULL), objview(NULL), jsonView(&result_ref) {}

	EntView(value& rm_ref, value& ent_ref, value& sub_ref, value& rel_ref, value& obj_ref, value& result_ref)
		: RM(rm_ref), parent(this), ent(ent_ref), sub(sub_ref)/*, rel(rel_ref)*/, obj(obj_ref), subview(NULL), objview(NULL), jsonView(&result_ref) {}

	/*
	функция должна построить:
	1. базовый словарь
	2. МО по переданному значению json
	3. вернуть идентификатор построенной сущности
	*/
	value& InitRoot(value &jsonValue)
	{
		int	RootId;

		// создаём корневую сущность
		if (jsonValue.is_object())	//	объект
		{
			if (jsonValue.has_field(L"()"))	//	сущность
			{
				jsonValue[L"id"] = value(L"#");
				RootId = AddEntity(jsonValue);
			}
			else	//	создаём новую сущность и возвращаем её адрес
			{
				RootId = CreateEntity(L"#"s, jsonValue);
			}
		}
		else
		{	//	создаём новую сущность и возвращаем её адрес
			RootId = CreateEntity(L"#"s, jsonValue);
		}

		return RM[RootId];
	}


	//	создание сущности по её частичной проекции
	int	AddEntity(value& Ent)
	{
		int	EntId = RM.size();
		RM[EntId] = value();
		if (!Ent.has_field(L"id")) Ent[L"id"] = value(L"Ent" + to_wstring(EntId));
		else if (!Ent[L"id"].is_string()) Ent[L"id"] = value(L"Ent" + to_wstring(EntId));

		if (!Ent.has_field(L"<-")) Ent[L"<-"] = EntRef(EntId);
		if (!Ent.has_field(L"->")) Ent[L"->"] = EntRef(EntId);

		for (auto& it : Ent.as_object())
		{
			if (it.first == L"<-" || it.first == L"->" || it.first == L"()")
			{
				it.second = objectFieldToRM(it.second);	//	подставляем ссылки на сущности вместо них самих
			}
			else
			{
				jsonViewToRM(it.second);	//	выносим значения элементов сущности в отдельные сущности
			}
		}

		RM[EntId] = Ent;
		RM[EntId][L"sub"] = value();	//	кэш проекции
		RM[EntId][L"obj"] = value();	//	кэш проекции
		EM[Ent[L"id"].as_string()] = &RM[EntId];	//	создаём карту имя -> ссылка на элемент
		return EntId;
	}

	value	objectFieldToRM(value &jsonValue)
	{
		if (jsonValue.is_null())		//	ссылка на себя
		{
			return jsonValue;
		}
		else if (jsonValue.is_object())	//	объект
		{
			if (jsonValue.has_field(L"$ref"))	//	ссылка
				return jsonValue;	//	возвращаем как есть
			if (jsonValue.has_field(L"()"))	//	выносим сущность в МО
				return EntRef(AddEntity(jsonValue));
		}

		//	создаём новую сущность по имеющемуся шаблону проекции
		return EntRef(CreateEntity(wstring(), jsonValue));
	}

	value& jsonViewToRM(value &jsonValue)
	{
		if (jsonValue.is_array())	//	масссив значений
		{
			for (auto& it : jsonValue.as_array())
				jsonViewToRM(it);
		}
		else if (jsonValue.is_object())	//	объект
		{
			if (jsonValue.has_field(L"()"))	//	сущность
			{
				value	EntId = value(AddEntity(jsonValue));
				jsonValue = value::object();
				jsonValue[L"$ref"] = EntId;
			}
			else for (auto& it : jsonValue.as_object())
			{	// сканируем все поля на предмет того что это сущность
				jsonViewToRM(it.second);
			}
		}

		return jsonValue;
	}

	//	создание сущности с закэшированной json проекцией
	int	CreateEntity(const wstring& Name, value& jsonView)
	{
		int	EntId = RM.size();
		RM[EntId] = value::object();
		RM[EntId][L"sub"] = value();	//	кэш проекции
		RM[EntId][L"obj"] = value();	//	кэш проекции
		 
		if (Name.length()) RM[EntId][L"id"] = value(Name);
		else               RM[EntId][L"id"] = value(L"Ent" + to_wstring(EntId));

		RM[EntId][L"->"] = EntRef(EntId);
		RM[EntId][L"<-"] = EntRef(EntId);
		RM[EntId][L"()"] = EntRef(0);	//	jsonView
		RM[EntId][L"jsonView"] = jsonViewToRM(jsonView);
		EM[RM[EntId][L"id"].as_string()] = &RM[EntId];	//	создаём карту имя -> ссылка на элемент
		return EntId;
	}

	//	создание сущности с закэшированной x86 проекцией
	int	Addx86Entity(const wstring& Name, x86View x86)
	{
		int	EntId = RM.size();
		RM[EntId] = value::object();
		RM[EntId][L"sub"] = value();	//	кэш проекции
		RM[EntId][L"obj"] = value();	//	кэш проекции

		if (Name.length()) RM[EntId][L"id"] = value(Name);
		else               RM[EntId][L"id"] = value(L"Ent" + to_wstring(EntId));

		RM[EntId][L"->"] = EntRef(EntId);
		RM[EntId][L"<-"] = EntRef(EntId);
		RM[EntId][L"()"] = EntRef(0);	//	jsonView
		RM[EntId][L"@"] = value((uint64_t)x86);
		EM[RM[EntId][L"id"].as_string()] = &RM[EntId];	//	создаём карту имя -> ссылка на элемент
		return EntId;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	Использование ссылок:
	ссылки в json представлении это не тип значения, а искуственный механизм для проброса ссылок на элемент
	в дереве json проекции или проекцию сущности МО
	После загрузки json в память, ссылки должны быть заменены на настоящие ссылки в область памяти
	*/
	//	специальная функция контекстной подстановки для свойства с именем "="
	//	находим и подставляем проекцию СУЩНОСТИ по имени или местоимению если она не задана непосредственно
	//	функция должна вернуть именно ссылку на проекцию
	//	ссылки могут быть только простых типов: string, number
	//	фактически функция исполняет строку или число (но можно и сложные объекты попробовать)
	value&	jsonViewRef(value &Ref, value &jsonView_ref)
	{
		switch (Ref.type())
		{
		case value::value_type::Number:		//	индекс комманды виртуальной машины
		{
			unsigned	EntId = Ref.as_number().to_uint32();
			if (EntId < RM.size())	return RM[EntId];
			else return jsonView_ref = value(L"Error: Entity with id " + to_wstring(EntId) + L" does not exist!");
		}

		case value::value_type::String:		//	символьная ссылка
		{
			//	#		root object (default)
			//	ent		entity model	= _this.Ent
			//	ent.<-	object model	= _this.Ent.obj
			//	ent.()	relation model	= _this.Ent.rel
			//	ent.->	subject model	= _this.Ent.sub
			//	.		entity view		= jsonView
			//	<-		object view		= _this.obj
			//	()		relation view	= _this.rel
			//	->		subject view	= _this.sub
			const wstring& String = Ref.as_string();
			value	*res = &jsonView_ref;	//	this by default

			if (String == L"."s)
			{
				if (EM.find(String) != EM.cend())
					return *EM[String];
				else if (this != parent)
					return parent->jsonViewRef(Ref, jsonView_ref);
				else
					return *res;
			}

			auto path = split(String, L"."s);


			for (auto it : path)
			{
				if (it == path[0])
				{
					if (it == L"ent")
						res = &ent;
					else if (it == L"this")
						res = jsonView;
					else if (it == L"<-")
						res = &GetObjView(ent[L"obj"]);	// jsonView);
					else if (it == L"->")
						res = &GetSubView(ent[L"sub"]);	// jsonView);
					else if (EM.find(it) != EM.cend())
						res = EM[it];
					else if (this != parent)
						return parent->jsonViewRef(Ref, jsonView_ref);
				}
				else try
				{
					if (res->is_array())
					{
						size_t index = _wtoi(it.c_str());
						res = &(*res)[index];
					}
					else if (res->is_object())
					{
						res = &(*res)[it];
					}
					else if (res->is_null())
					{	//	определяем тип индекса и создаём элемент
						*res = value::object();
						res = &(*res)[it];
					}
					else throw it;
				}
				catch (...)
				{
					return jsonView_ref = value(L"Error: property with name " + it + L" does not exist!");
				}
			}

			return *res;
		}

		default:	//	недопустимый тип значения ссылки
			return jsonView_ref = value(L"Error: incorrect reference value type = "s + to_wstring(Ref.type()));;
		}
	}

	//	отдельная функция для получения полной проекции сущности
	//	jsonView - это ссылка на значение для возврата результата если требуется выделить память под результат
	//	функция использует в качестве шиблонной проекции любой json переданный в Ent
	value&	jsonViewEnt(value &Ent, value &jsonView_ref)
	{
		if (Ent.is_object())
		{
			if (Ent.has_field(L"@"))	//	машинное слово
			{
				return x86Exec(Ent[L"@"]);
			}
			else if (Ent.has_field(L"$ref"))		//	это ссылка на сущность, её проекцию или её элемент
			{
				return jsonViewEnt(jsonViewRef(Ent[L"$ref"], jsonView_ref), jsonView_ref);
			}
			else if (Ent.has_field(L"()"))	//	это сущность
			{
				if (Ent.has_field(L"jsonView"))	//	у неё есть закэшированный шаблон проекции
					return jsonViewEnt(Ent[L"jsonView"], jsonView_ref);	//	рекурсия для обработки сущностей и ссылок
				else    //	надо вычислить проекцию
					return jsonExecEnt(Ent, jsonView_ref);
			}
		}

		return Ent;	//	собственное значение, закэшировано само по себе
	}

	value&	GetSubView(value &jsonView_ref)
	{	//	инстанцируем шаблонную проекцию субъекта в родительском контексте 
		if (!subview)
			subview = (&ent == &sub) ? jsonView : &parent->jsonViewEnt(sub, jsonView_ref);
		return *subview;
	}

	value&	GetObjView(value &jsonView_ref)
	{	//	инстанцируем шаблонную проекцию объекта в родительском контексте 
		if (!objview)
			objview = (&ent == &obj) ? jsonView : &parent->jsonViewEnt(obj, jsonView_ref);
		return *objview;
	}

	value&	x86Exec(value& x86Ptr)
	{
		switch (x86Ptr.type())
		{
		case value::value_type::Number:		//	function address
			return ((x86View)x86Ptr.as_number().to_uint64())(*this);

		default:
			return *this->jsonView = value(L"Wrong x86View json value type!"s);
		}
	}

	value&	GetEntRef(value &Ent)
	{
		if (Ent.is_object())
			if (Ent.has_field(L"$ref"))
				return jsonViewRef(Ent[L"$ref"], Ent);

		return Ent;	//	собственное значение
	}

	value&	jsonExecEnt(value &Ent, value &jsonView)
	{
		EntView	ctx(this,
					Ent,
					GetEntRef(Ent[L"->"]),
					Ent[L"()"]/*GetEntRef(Parent, Ent[L"()"])*/,
					GetEntRef(Ent[L"<-"]),
					jsonView);
		return ctx.jsonExec(Ent[L"()"]);// ctx.rel);
	}

	//	Исполнение шаблонной проекции, либо её вычисление в случае отсутствия
	//	Построение jsonValue через исполнение Rel в контексте EV 
	//	сама функция не кэширует вычисленное значение?
	//	имеет прототип отличный от других контроллеров и не является контроллером
	//	рекурсивно раскручивает структуру проекции контроллера доходя до простых json
	value&	jsonExec(value &relview)	//	reflector
	{	//	если вызвали exec то неважно было ли закэшировано значение в Ent или нет, требуется заново его пересчитать
		switch (relview.type())
		{
		case value::value_type::Object:
		{
			if (relview.has_field(L"@"))		//	машинное слово
			{
				return x86Exec(relview[L"@"]);
			}
			else if (relview.has_field(L"$ref"))		//	ссылка
			{	//	исполнение ссылки это возврат проекции на которую она указывает
				return jsonExec(jsonViewRef(relview[L"$ref"], *jsonView));	//	тут не обязательна полная проекция
			}
			else if (relview.has_field(L"()"))		//	сущность
			{
				if (relview.has_field(L"jsonView"))	//	есть шаблон проекции?
					return jsonExec(relview[L"jsonView"]);
				else								//	интерпретируем
					return jsonExecEnt(relview, *jsonView);
			}
			else   //	контроллер это лямбда структура, которая управляет параллельным проецированием свойств объекта в элементы субъекта
			{
				value&	_subview = GetSubView(_sv);
				value&	_objview = GetObjView(_ov);
				if (!_subview.is_object()) _subview = value::object();
				for (auto& it : relview.as_object())
				{
					jsonView = subview = &_subview[it.first];
					value& res = jsonExec(it.second);
					_subview[it.first] = res;// jsonExec(EV, it.second);
				}
				return _subview;
			}
		}

		case value::value_type::Array:	//	лямбда вектор, который управляет параллельным проецированием элементов объекта в элементы субъекта
		{
			value&	_subview = GetSubView(_sv);
			value&	objview = GetObjView(_ov);
			if (!_subview.is_array()) _subview = value::array();
			for (size_t i = 0; i < relview.size(); i++)
			{
				jsonView = subview = &_subview[i];
				_subview[i] = jsonExec(relview[i]);
			}
			return _subview;
		}

		case value::value_type::Number:		//	индекс сущности в МО
		case value::value_type::String:		//	иерархическая символьная ссылка в проекции сущности
			return jsonViewEnt(jsonViewRef(relview, *jsonView), *jsonView);

		case value::value_type::Boolean:	//	фильтрация атрибутов модели по маске, управление видимостью проекций
			return *jsonView = relview;	//	в будущем можно попробовать сделать if

		default:	//	в будущем можно попробовать сделать проверку на null ?
			return *jsonView = value();
		}
	}
};

