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
      [ "А", "md_doc__dictionary.html#autotoc_md8", null ],
      [ "Ассоциативная Модель Данных", "md_doc__dictionary.html#amd", null ],
      [ "Б", "md_doc__dictionary.html#autotoc_md9", null ],
      [ "В", "md_doc__dictionary.html#autotoc_md10", null ],
      [ "Внешняя структура сущности", "md_doc__dictionary.html#outter_ent_struct", null ],
      [ "Внутренняя структура сущности", "md_doc__dictionary.html#inner_ent_struct", null ],
      [ "Вышестоящий контекст исполнения", "md_doc__dictionary.html#upper_ctx", null ],
      [ "Г", "md_doc__dictionary.html#autotoc_md11", null ],
      [ "Д", "md_doc__dictionary.html#autotoc_md12", null ],
      [ "Дерево контекстов исполнения", "md_doc__dictionary.html#ctx_tree", null ],
      [ "Дочерний контекст исполнения", "md_doc__dictionary.html#child_ctx", null ],
      [ "Е", "md_doc__dictionary.html#autotoc_md13", null ],
      [ "Ё", "md_doc__dictionary.html#autotoc_md14", null ],
      [ "Ж", "md_doc__dictionary.html#autotoc_md15", null ],
      [ "З", "md_doc__dictionary.html#autotoc_md16", null ],
      [ "Значение отношения", "md_doc__dictionary.html#rel_val", null ],
      [ "И", "md_doc__dictionary.html#autotoc_md17", null ],
      [ "Исполнение сущности", "md_doc__dictionary.html#exec_ent", null ],
      [ "Й", "md_doc__dictionary.html#autotoc_md18", null ],
      [ "К", "md_doc__dictionary.html#autotoc_md19", null ],
      [ "Контекст исполнения", "md_doc__dictionary.html#exec_ctx", null ],
      [ "Контекстный объект", "md_doc__dictionary.html#ctx_obj", null ],
      [ "Контекстное отношение", "md_doc__dictionary.html#ctx_rel", null ],
      [ "Контекстная связь", "md_doc__dictionary.html#ctx_ref", null ],
      [ "Контекстный субъект", "md_doc__dictionary.html#ctx_sub", null ],
      [ "Контекстная сущность", "md_doc__dictionary.html#ctx_ent", null ],
      [ "Корневой контекст исполнения", "md_doc__dictionary.html#root_ctx", null ],
      [ "Корень модели отношений", "md_doc__dictionary.html#rm_root", null ],
      [ "Корневая сущность модели отношений", "md_doc__dictionary.html#root_ent", null ],
      [ "Л", "md_doc__dictionary.html#autotoc_md20", null ],
      [ "Лямбда вектор {#}", "md_doc__dictionary.html#autotoc_md21", null ],
      [ "Лямбда cтруктура {#}", "md_doc__dictionary.html#autotoc_md22", null ],
      [ "М", "md_doc__dictionary.html#autotoc_md23", null ],
      [ "Местоимение", "md_doc__dictionary.html#pronoun", null ],
      [ "Модель Отношений", "md_doc__dictionary.html#rm", null ],
      [ "Н", "md_doc__dictionary.html#autotoc_md24", null ],
      [ "О", "md_doc__dictionary.html#autotoc_md25", null ],
      [ "Объект", "md_doc__dictionary.html#obj", null ],
      [ "Определение отношения", "md_doc__dictionary.html#calc_rel", null ],
      [ "Отношение", "md_doc__dictionary.html#rel", null ],
      [ "П", "md_doc__dictionary.html#autotoc_md26", null ],
      [ "Проецирование сущности", "md_doc__dictionary.html#view", null ],
      [ "Проекция сущности", "md_doc__dictionary.html#ent_view", null ],
      [ "Переменные контекста исполнения", "md_doc__dictionary.html#ctx_vars", null ],
      [ "Р", "md_doc__dictionary.html#autotoc_md27", null ],
      [ "Родительский контекст исполнения", "md_doc__dictionary.html#parent_ctx", null ],
      [ "С", "md_doc__dictionary.html#autotoc_md28", null ],
      [ "Связь с предком", "md_doc__dictionary.html#ancestor_link", null ],
      [ "Структура сущности", "md_doc__dictionary.html#ent_struct", null ],
      [ "Субъект", "md_doc__dictionary.html#sub", null ],
      [ "Сущность", "md_doc__dictionary.html#ent", null ],
      [ "Сущность объект", "md_doc__dictionary.html#ent_obj", null ],
      [ "Сущность отношение", "md_doc__dictionary.html#ent_rel", null ],
      [ "Сущность субъект", "md_doc__dictionary.html#ent_sub", null ],
      [ "Т", "md_doc__dictionary.html#autotoc_md29", null ],
      [ "Текущий контекст исполнения", "md_doc__dictionary.html#ctx", null ],
      [ "Триплет", "md_doc__dictionary.html#triplet", null ],
      [ "У", "md_doc__dictionary.html#autotoc_md30", null ],
      [ "Уровень контекста исполнения", "md_doc__dictionary.html#ctx_level", null ],
      [ "Ф", "md_doc__dictionary.html#autotoc_md31", null ],
      [ "Х", "md_doc__dictionary.html#autotoc_md32", null ],
      [ "Ц", "md_doc__dictionary.html#autotoc_md33", null ],
      [ "Ч", "md_doc__dictionary.html#autotoc_md34", null ],
      [ "Ш", "md_doc__dictionary.html#autotoc_md35", null ],
      [ "Щ", "md_doc__dictionary.html#autotoc_md36", null ],
      [ "Ъ", "md_doc__dictionary.html#autotoc_md37", null ],
      [ "Ы", "md_doc__dictionary.html#autotoc_md38", null ],
      [ "Ь", "md_doc__dictionary.html#autotoc_md39", null ],
      [ "Э", "md_doc__dictionary.html#autotoc_md40", null ],
      [ "Экземпляр отношения", "md_doc__dictionary.html#rel_instance", null ],
      [ "Ю", "md_doc__dictionary.html#autotoc_md41", null ],
      [ "Я", "md_doc__dictionary.html#autotoc_md42", null ]
    ] ],
    [ "Введение", "md_doc__introduction.html", null ],
    [ "LICENSE", "md_doc__l_i_c_e_n_s_e.html", null ],
    [ "Математическая модель", "md_doc__mathematical_definition.html", null ],
    [ "Модель Отношений", "md_doc__r_m.html", [
      [ "Виды топологий сущностей МО", "md_doc__r_m.html#autotoc_md47", null ],
      [ "json проекция в МО", "md_doc__r_m.html#autotoc_md48", null ]
    ] ],
    [ "Модель Отношений (старый текст)", "md_doc_rmodel.html", null ],
    [ "Реляционная виртуальная машина", "md_doc__r_v_m.html", [
      [ "Инициализация", "md_doc__r_v_m.html#autotoc_md51", null ],
      [ "Исполнение МО", "md_doc__r_v_m.html#autotoc_md52", null ],
      [ "Правила json скрипта (проекции МО в json):", "md_doc__r_v_m.html#autotoc_md53", null ],
      [ "Варианты взаимоотношений узлов json:", "md_doc__r_v_m.html#autotoc_md54", null ],
      [ "О свойствах (атрибутах) и элементах сущности:", "md_doc__r_v_m.html#autotoc_md55", null ],
      [ "Семантика значений разных типов в полях $sub, $rel, $obj json объекта описывающего сущность", "md_doc__r_v_m.html#autotoc_md56", null ],
      [ "Семантика json значений при исполнении  (json как байткод RVM):", "md_doc__r_v_m.html#autotoc_md57", null ],
      [ "Программирование на МО", "md_doc__r_v_m.html#autotoc_md58", null ],
      [ "Контекст исполнения сущности", "md_doc__r_v_m.html#autotoc_md59", null ],
      [ "Стэк", "md_doc__r_v_m.html#autotoc_md60", null ],
      [ "Аналогия исполнения", "md_doc__r_v_m.html#autotoc_md61", null ]
    ] ],
    [ "Словарь общепринятых терминов", "md_doc_wiki_concepts.html", null ],
    [ "README", "md__r_e_a_d_m_e.html", [
      [ "json Relations (Model) Virtual Machine", "md__r_e_a_d_m_e.html#autotoc_md63", [
        [ "Модель Отношений - это язык метапрограммирования.", "md_doc__introduction.html#autotoc_md44", null ],
        [ "jsonRVM Documentation", "md__r_e_a_d_m_e.html#autotoc_md64", null ],
        [ "License", "md__r_e_a_d_m_e.html#autotoc_md65", null ],
        [ "Used third-party tools", "md__r_e_a_d_m_e.html#autotoc_md66", null ]
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
"namespacerm.html#a1c7d0997ec99952a5d349c066811b0d1"
];

var SYNCONMSG = 'нажмите на выключить для синхронизации панелей';
var SYNCOFFMSG = 'нажмите на включить для синхронизации панелей';