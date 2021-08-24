/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "jsonRVM", "index.html", [
    [ "Сокращения принятые в руководстве", "md_doc__abbreviations.html", [
      [ "EOP / СОП", "md_doc__abbreviations.html#autotoc_md1", null ],
      [ "RM / МО", "md_doc__abbreviations.html#autotoc_md2", null ],
      [ "RVM / РВМ", "md_doc__abbreviations.html#autotoc_md3", null ],
      [ "AMD / АМД", "md_doc__abbreviations.html#autotoc_md4", null ],
      [ "MHMVC", "md_doc__abbreviations.html#autotoc_md5", null ],
      [ "RDF", "md_doc__abbreviations.html#autotoc_md6", null ]
    ] ],
    [ "Терминологический аппарат", "md_doc__dictionary.html", [
      [ "Ассоциативная Модель Данных", "md_doc__dictionary.html#autotoc_md8", null ],
      [ "Триплет", "md_doc__dictionary.html#autotoc_md9", null ],
      [ "Сущность", "md_doc__dictionary.html#autotoc_md10", null ],
      [ "Контекстная сущность", "md_doc__dictionary.html#autotoc_md11", null ],
      [ "Модель отношений", "md_doc__dictionary.html#autotoc_md12", null ],
      [ "Экземпляр отношения - Структура сущности", "md_doc__dictionary.html#autotoc_md13", null ],
      [ "Значение сущности", "md_doc__dictionary.html#autotoc_md14", null ],
      [ "Исполнение модели отношений", "md_doc__dictionary.html#autotoc_md15", null ],
      [ "Сущность субъект", "md_doc__dictionary.html#autotoc_md16", null ],
      [ "Контекстный субъект", "md_doc__dictionary.html#autotoc_md17", null ],
      [ "Субъект", "md_doc__dictionary.html#autotoc_md18", null ],
      [ "Сущность отношение", "md_doc__dictionary.html#autotoc_md19", null ],
      [ "Контекстное отношение", "md_doc__dictionary.html#autotoc_md20", null ],
      [ "Отношение", "md_doc__dictionary.html#autotoc_md21", null ],
      [ "Сущность объект", "md_doc__dictionary.html#autotoc_md22", null ],
      [ "Контекстный объект", "md_doc__dictionary.html#autotoc_md23", null ],
      [ "Объект", "md_doc__dictionary.html#autotoc_md24", null ],
      [ "Экземпляр сущности - Экземпляр экземпляра отношения", "md_doc__dictionary.html#autotoc_md25", null ],
      [ "Инстанцирование сущности - Инстанциование экземпляра отношения", "md_doc__dictionary.html#autotoc_md26", null ],
      [ "Проекция сущности", "md_doc__dictionary.html#autotoc_md27", null ],
      [ "Исполнение сущности (в контексте исполнения)", "md_doc__dictionary.html#autotoc_md28", null ],
      [ "Вычисление отношения", "md_doc__dictionary.html#autotoc_md29", null ],
      [ "Значение сущности - Значение экземпляра отношения", "md_doc__dictionary.html#autotoc_md30", null ],
      [ "Контекст исполнения", "md_doc__dictionary.html#autotoc_md31", null ],
      [ "Местоимение", "md_doc__dictionary.html#autotoc_md32", null ]
    ] ],
    [ "Введение", "md_doc__introduction.html", null ],
    [ "LICENSE", "md_doc__l_i_c_e_n_s_e.html", null ],
    [ "Математическая модель", "md_doc__mathematical_definition.html", [
      [ "Приложения", "md_doc__mathematical_definition.html#autotoc_md36", [
        [ "Модель Отношений - это язык метапрограммирования.", "md_doc__introduction.html#autotoc_md34", null ],
        [ "1. ВСЕ СПЕЦ. СИМВОЛЫ ИСПОЛЬЗУЕМЫЕ ДЛЯ МАТЕМАТИКИ И ФИЗИКИ В UNICODE", "md_doc__mathematical_definition.html#autotoc_md37", null ]
      ] ]
    ] ],
    [ "Модель Отношений", "md_doc__r_m.html", [
      [ "Виды топологий сущностей МО", "md_doc__r_m.html#autotoc_md39", null ],
      [ "json проекция в МО", "md_doc__r_m.html#autotoc_md40", null ]
    ] ],
    [ "Реляционная виртуальная машина", "md_doc__r_v_m.html", [
      [ "Проекция сущности", "md_doc__r_v_m.html#autotoc_md42", null ],
      [ "Исполнение сущности", "md_doc__r_v_m.html#autotoc_md43", null ],
      [ "Вычисление отношения", "md_doc__r_v_m.html#autotoc_md44", null ],
      [ "Сущность", "md_doc__r_v_m.html#autotoc_md45", null ],
      [ "Экземпляр отношения", "md_doc__r_v_m.html#autotoc_md46", null ],
      [ "Экземпляр сущности", "md_doc__r_v_m.html#autotoc_md47", null ],
      [ "Инициализация", "md_doc__r_v_m.html#autotoc_md48", null ],
      [ "Исполнение МО", "md_doc__r_v_m.html#autotoc_md49", null ],
      [ "Правила json скрипта (проекции МО в json):", "md_doc__r_v_m.html#autotoc_md50", null ],
      [ "Варианты взаимоотношений узлов json:", "md_doc__r_v_m.html#autotoc_md51", null ],
      [ "О свойствах (атрибутах) и элементах сущности:", "md_doc__r_v_m.html#autotoc_md52", null ],
      [ "Семантика значений разных типов в полях $sub, $rel, $obj json объекта описывающего сущность", "md_doc__r_v_m.html#autotoc_md53", null ],
      [ "тип значения |                     сементика значения", "md_doc__r_v_m.html#autotoc_md55", null ],
      [ "Number      | адрес объекта типа json в памяти", "md_doc__r_v_m.html#autotoc_md56", null ],
      [ "Boolean     | непосредственное значение = true/false", "md_doc__r_v_m.html#autotoc_md57", null ],
      [ "| исполнения", "md_doc__r_v_m.html#autotoc_md58", null ],
      [ "Object      | json объект определяющий непосредственное значение сущности", "md_doc__r_v_m.html#autotoc_md59", null ],
      [ "Array       | json массив определяющий непосредственное значение сущности", "md_doc__r_v_m.html#autotoc_md60", null ],
      [ "| сущности в контексте исполнения EntView[\"\"]", "md_doc__r_v_m.html#autotoc_md61", null ],
      [ "Entity      | непосредственное описание другой сущности", "md_doc__r_v_m.html#autotoc_md62", null ],
      [ "Семантика json значений при исполнении  (json как байткод RVM):", "md_doc__r_v_m.html#autotoc_md63", null ],
      [ "тип значения |                     сементика значения", "md_doc__r_v_m.html#autotoc_md65", null ],
      [ "Number      | адрес скомпилированного тела сущности, которое необходимо исполнить RVM", "md_doc__r_v_m.html#autotoc_md66", null ],
      [ "| false : не проецировать", "md_doc__r_v_m.html#autotoc_md67", null ],
      [ "| которое необходимо исполнить RVM", "md_doc__r_v_m.html#autotoc_md68", null ],
      [ "| вычисление проекций множества пар может происходить параллельно, в многопоточном режиме", "md_doc__r_v_m.html#autotoc_md69", null ],
      [ "| как текущее значение проекции", "md_doc__r_v_m.html#autotoc_md70", null ],
      [ "| отсутствие действия над текущей проекцией в контексте исполнения", "md_doc__r_v_m.html#autotoc_md71", null ],
      [ "Entity      | сущность, для которой надо создать дочерний контекст и исполнить", "md_doc__r_v_m.html#autotoc_md72", null ],
      [ "Программирование на МО", "md_doc__r_v_m.html#autotoc_md73", null ],
      [ "Контекст исполнения сущности", "md_doc__r_v_m.html#autotoc_md74", null ],
      [ "Стэк", "md_doc__r_v_m.html#autotoc_md75", null ],
      [ "Аналогия исполнения", "md_doc__r_v_m.html#autotoc_md76", null ]
    ] ],
    [ "Словарь общепринятых терминов", "md_doc_wiki_concepts.html", null ],
    [ "README", "md__r_e_a_d_m_e.html", [
      [ "json Relations (Model) Virtual Machine", "md__r_e_a_d_m_e.html#autotoc_md78", [
        [ "jsonRVM Documentation", "md__r_e_a_d_m_e.html#autotoc_md79", null ],
        [ "Used third-party tools", "md__r_e_a_d_m_e.html#autotoc_md80", null ]
      ] ]
    ] ],
    [ "Пространства имен", "namespaces.html", [
      [ "Пространства имен", "namespaces.html", "namespaces_dup" ],
      [ "Члены пространств имен", "namespacemembers.html", [
        [ "Указатель", "namespacemembers.html", null ],
        [ "Функции", "namespacemembers_func.html", null ],
        [ "Переменные", "namespacemembers_vars.html", null ],
        [ "Определения типов", "namespacemembers_type.html", null ]
      ] ]
    ] ],
    [ "Классы", "annotated.html", [
      [ "Классы", "annotated.html", "annotated_dup" ],
      [ "Алфавитный указатель классов", "classes.html", null ],
      [ "Иерархия классов", "hierarchy.html", "hierarchy" ],
      [ "Члены классов", "functions.html", [
        [ "Указатель", "functions.html", null ],
        [ "Функции", "functions_func.html", null ],
        [ "Переменные", "functions_vars.html", null ],
        [ "Определения типов", "functions_type.html", null ]
      ] ]
    ] ],
    [ "Файлы", "files.html", [
      [ "Файлы", "files.html", "files_dup" ],
      [ "Список членов всех файлов", "globals.html", [
        [ "Указатель", "globals.html", null ],
        [ "Функции", "globals_func.html", null ],
        [ "Макросы", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"namespacerm.html#a610895365f144e89242ce3837b38bac2"
];

var SYNCONMSG = 'нажмите на выключить для синхронизации панелей';
var SYNCOFFMSG = 'нажмите на включить для синхронизации панелей';