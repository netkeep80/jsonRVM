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
#include "JsonRVM.h"


//	функция отображает объект в значение проекции субъекта осуществляя подстановку в шаблонную проекцию объекта
//	Необходимо проверить нет ли закэшированного значения в View?
//	и если его нет то вычислить и вернуть.
value&	_jsonView(EntView &EV)
{	//	получаем ссылку на проекцию субъекта, что бы знать куда записывать проекцию объекта
	//	контекст EV относится к сущности внутри которой идёт проецирование объекта в субъект
	if (!EV.subview)
	{
		EV.subview = (&EV.ent == &EV.sub) ? (EV.jsonView) : &EV.parent->jsonViewEnt(EV.sub, *EV.jsonView);
	}
	if (!EV.objview)
	{
		EV.objview = (&EV.ent == &EV.obj) ? (EV.jsonView) : &EV.parent->jsonViewEnt(EV.obj, *EV.subview);
	}
	return *EV.subview = *EV.objview;
}


bool	InitRVMVoc(EntView &EV)
{
	EV.Addx86Entity(L"jsonView"s, _jsonView);
	EV.Addx86Entity(L"="s, _jsonView);
	return true;
}

