[![Gitpod ready-to-code](https://img.shields.io/badge/Gitpod-ready--to--code-blue?logo=gitpod)](https://gitpod.io/#https://github.com/netkeep80/jsonrvm)
[![GitHub Workflow Status](https://img.shields.io/github/workflow/status/netkeep80/jsonRVM/publish?label=Publish%20Doxygen%20to%20gh-pages)](https://github.com/netkeep80/jsonRVM/actions/workflows/publish.yml)

[![GitHub Workflow Status](https://img.shields.io/github/workflow/status/netkeep80/jsonRVM/publish)](https://github.com/netkeep80/jsonRVM/actions/workflows/publish.yml)
![GitHub last commit](https://img.shields.io/github/last-commit/netkeep80/jsonRVM)
[![GitHub](https://img.shields.io/github/license/netkeep80/jsonRVM)](https://github.com/netkeep80/jsonRVM/blob/master/LICENSE)

<p align="center"><img src="rm_view.jpg"></p>

# json Relations (Model) Virtual Machine
```
            R
         S__|__O
       O   _|_   S     Fractal
    R__|__/_|_\__|__R  Triune
       |  \_|_/  |     Entity
       S    |    O     
          __|__
         /  |  \
        /___|___\
```

### Язык метапрограммирования "Модель Отношений" (JSON Relations Model)

#### Обзор
Язык "Модель Отношений" (Relations Model, RM) — это декларативный DSL (Domain-Specific Language), ориентированный на описание сложных систем через иерархию сущностей и их отношений. Программы представляются в виде JSON-структур, которые исполняются виртуальной машиной RMVM. Ключевые особенности:

1. **Сущности (Entities)**  
   Базовые элементы системы, описываемые JSON-объектами. Могут быть:
   - Примитивами: `числа`, `строки`, `булевы значения`.
   - Составными типами: `массивы`, `объекты`.
   - Ссылками: `$ref` на другие сущности.
   - Исполняемыми конструкциями: `$rel`, `$obj`, `$sub`.

2. **Отношения (Relations)**  
   Определяют правила взаимодействия между сущностями. Примеры:
   - **Создание сущности**: `add_entity`.
   - **Загрузка словарей**: `load/dll`.
   - **Параллельные вычисления**.

3. **Контексты (Contexts)**  
   Каждое отношение исполняется в контексте, который включает:
   - `$ent` — текущая сущность.
   - `$rel` — исполняемое отношение.
   - `$obj` — объект отношения (входные данные).
   - `$sub` — субъект отношения (результат).

---

### Синтаксис и Конструкции

#### 1. Ссылки (`$ref`)
Позволяют обращаться к другим сущностям по JSON-пути:
```json
{
  "action": { "$ref": "/entities/user" }
}
```

#### 2. Исполняемые блоки
- **Последовательное выполнение** (массивы):
  ```json
  [
    { "$rel": "step1" },
    { "$rel": "step2" }
  ]
  ```
- **Параллельное выполнение** (объекты):
  ```json
  {
    "task1": { "$rel": "process_data" },
    "task2": { "$rel": "validate_data" }
  }
  ```

#### 3. Отношения (`$rel`, `$obj`, `$sub`)
```json
{
  "$rel": "add_entity",
  "$obj": {
    "type": "user",
    "name": "Alice"
  },
  "$sub": "$ent/new_user_id"
}
```
- `$rel` — имя отношения.
- `$obj` — входные данные.
- `$sub` — куда сохранить результат.

#### 4. Импорт словарей
Загрузка внешних модулей через DLL:
```json
{
  "$rel": "rmvm/load/dll",
  "$obj": {
    "PathFolder": "libs/",
    "FileName": "math_operations.dll"
  }
}
```

---

### Семантика

#### 1. Исполнение отношений
- **Рекурсивная обработка**: VM рекурсивно обходит JSON-структуру, применяя отношения.
- **Автоматическое распараллеливание**: Объекты исполняют свои ключи параллельно.
- **Обработка ошибок**: Исключения включают контекст (сущность, отношение, путь).

#### 2. Работа с данными
- **Динамическое разрешение ссылок**: `$ref` вычисляются в рантайме.
- **Потокобезопасность**: Мьютексы защищают доступ к общим данным.
- **Сериализация**: Сущности сохраняются в JSON-файлы через `file_database_t`.

#### 3. Расширяемость
- **Внешние библиотеки**: DLL с функциями вида `import_relations_model_to`.
- **Базовые сущности**: Встроенные отношения (`version`, `add_entity`).

---

### Пример программы
```json
{
  "schema": {
    "$rel": "add_entity",
    "$obj": {
      "name": "Пример системы",
      "components": [
        { "$ref": "/entities/logger" },
        { "$ref": "/entities/database" }
      ]
    },
    "$sub": "$ent/system_id"
  },
  "init": [
    {
      "$rel": "rmvm/load/dll",
      "$obj": {
        "PathFolder": "modules/",
        "FileName": "network.dll"
      }
    },
    {
      "$rel": "network/connect",
      "$obj": { "url": "https://api.example.com" }
    }
  ]
}
```

---

### Особенности реализации
- **Компиляция строк**: Макросы `SWITCH`/`CASE` используют хеширование для эффективного сравнения строк.
- **Кодировки**: Поддержка CP1251 и UTF-8 через функции преобразования.
- **Многопоточность**: `std::mutex` и `std::lock_guard` для синхронизации.

---

### Типичные сценарии использования
1. **ORM-системы**: Динамическое создание сущностей БД.
2. **Микросервисы**: Параллельное выполнение задач.
3. **Конфигурация систем**: Иерархическое описание компонентов.
4. **Плагины**: Загрузка внешних модулей через DLL.

Этот язык позволяет описывать сложные системы в декларативном стиле, абстрагируясь от низкоуровневых деталей, что делает его удобным для моделирования бизнес-логики и распределённых систем.

## jsonRVM Documentation

[By Doxygen on russian.](https://netkeep80.github.io/jsonRVM/)

## License

<img align="right" src="https://opensource.org/trademarks/opensource/OSI-Approved-License-100x137.png">

The class is licensed under the [MIT License](https://opensource.org/licenses/MIT):

Copyright &copy; 2016-2021 [Vertushkin Roman Pavlovich](https://vk.com/earthbirthbook)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

## Thanks

I deeply appreciate the help of the following people.

- [Vladimir Muravyev](https://github.com/vmuravyev) helped to develop the terminological apparatus of the Relations Model.

## Used third-party tools

jsonRVM using the following third-party tools. Thanks a lot!

- [**JSON for Modern C++**](https://github.com/nlohmann/json) for load/exec/unload json view of Relations Model
- [**The fastest feature-rich C++ single-header testing framework**](https://github.com/onqtam/doctest) for unit testing of Relations (Model) Virtual Machine
- [**A C++11 single-file header-only cross platform HTTP/HTTPS library**](https://github.com/yhirose/cpp-httplib) for writing http base vocabulary for Relations (Model) Virtual Machine
- [**xml2json is a header-only C++ library**](https://github.com/Cheedoong/xml2json) for writing http base vocabulary for Relations (Model) Virtual Machine
- [**Switch for strings in C++11**](https://github.com/Efrit/str_switch) for writing fast switch for Relations (Model) Virtual Machine
