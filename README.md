[![Gitpod ready-to-code](https://img.shields.io/badge/Gitpod-ready--to--code-blue?logo=gitpod)](https://gitpod.io/#https://github.com/netkeep80/jsonrvm)
[![GitHub Actions](https://github.com/netkeep80/jsonRVM/actions/workflows/publish.yml/badge.svg)](https://github.com/netkeep80/jsonRVM/actions/workflows/publish.yml)
![GitHub last commit](https://img.shields.io/github/last-commit/netkeep80/jsonRVM)
[![GitHub](https://img.shields.io/github/license/netkeep80/jsonRVM)](https://github.com/netkeep80/jsonRVM/blob/master/LICENSE.MIT)

<p align="center"><img src="rm_view.jpg"></p>

# jsonRVM — json Relations (Model) Virtual Machine

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

---

## Содержание / Contents

- [Русский](#русский)
- [English](#english)
- [Документация / Documentation](#документация--documentation)

---

# Русский

## Описание

**jsonRVM** — это виртуальная машина для исполнения программ, написанных на декларативном языке **"Модель Отношений"** (Relations Model, RM). Программы представляются в формате JSON и интерпретируются виртуальной машиной как байткод.

### Основные концепции

**Модель Отношений** — это язык метапрограммирования, где всё представляется как сущности и их отношения:

1. **Сущности (Entities)** — базовые элементы системы в виде JSON-объектов:
   - Примитивы: числа, строки, булевы значения
   - Составные типы: массивы, объекты
   - Ссылки: `$ref` на другие сущности
   - Исполняемые конструкции: `$rel`, `$obj`, `$sub`

2. **Отношения (Relations)** — определяют взаимодействие между сущностями:
   - `$rel` — отношение (контроллер)
   - `$obj` — объект (входные данные)
   - `$sub` — субъект (результат)

3. **Контексты (Contexts)** — иерархия исполнения с доступом к:
   - `$ent` — текущая сущность
   - `$rel` — текущее отношение
   - `$obj` — текущий объект
   - `$sub` — текущий субъект

### Ключевые особенности

- **Декларативный подход** — описание *что* делать, а не *как*
- **Автоматический параллелизм** — объекты исполняются параллельно, массивы — последовательно
- **Гомоиконичность** — код и данные имеют одинаковую JSON-структуру
- **Расширяемость** — поддержка плагинов через DLL-библиотеки
- **Рефлексия** — программы могут анализировать и модифицировать себя

### Пример программы

```json
{
  "$rel/result": {
    "$obj": 1,
    "$rel": "+",
    "$sub": 1
  }
}
```

Результат: `result = 2`

### Более сложный пример

```json
{
  "$rel": [
    {
      "$rel": "rmvm/load/dll",
      "$obj": {
        "PathFolder": "modules/",
        "FileName": "network.dll"
      }
    },
    {
      "$rel": "add_entity",
      "$obj": {
        "name": "Пример системы",
        "components": [
          { "$ref": "/entities/logger" },
          { "$ref": "/entities/database" }
        ]
      },
      "$sub": "$ent/system_id"
    }
  ]
}
```

---

# English

## Description

**jsonRVM** is a virtual machine for executing programs written in the declarative **"Relations Model"** (RM) language. Programs are represented in JSON format and interpreted by the virtual machine as bytecode.

### Core Concepts

**Relations Model** is a metaprogramming language where everything is represented as entities and their relations:

1. **Entities** — basic system elements as JSON objects:
   - Primitives: numbers, strings, booleans
   - Compound types: arrays, objects
   - References: `$ref` to other entities
   - Executable constructs: `$rel`, `$obj`, `$sub`

2. **Relations** — define interactions between entities:
   - `$rel` — relation (controller)
   - `$obj` — object (input data)
   - `$sub` — subject (result)

3. **Contexts** — execution hierarchy with access to:
   - `$ent` — current entity
   - `$rel` — current relation
   - `$obj` — current object
   - `$sub` — current subject

### Key Features

- **Declarative approach** — describe *what* to do, not *how*
- **Automatic parallelism** — objects execute in parallel, arrays — sequentially
- **Homoiconicity** — code and data share the same JSON structure
- **Extensibility** — plugin support via DLL libraries
- **Reflection** — programs can analyze and modify themselves

### Example Program

```json
{
  "$rel/result": {
    "$obj": 1,
    "$rel": "+",
    "$sub": 1
  }
}
```

Result: `result = 2`

### Advanced Example

```json
{
  "$rel": [
    {
      "$rel": "rmvm/load/dll",
      "$obj": {
        "PathFolder": "modules/",
        "FileName": "network.dll"
      }
    },
    {
      "$rel": "add_entity",
      "$obj": {
        "name": "Example system",
        "components": [
          { "$ref": "/entities/logger" },
          { "$ref": "/entities/database" }
        ]
      },
      "$sub": "$ent/system_id"
    }
  ]
}
```

---

## Документация / Documentation

### Анализ и планирование / Analysis and Planning

- [**analysis.md**](analysis.md) — Анализ текущего состояния проекта (сильные и слабые стороны) / Project analysis (strengths and weaknesses)
- [**plan.md**](plan.md) — План развития проекта / Development roadmap

### Техническая документация / Technical Documentation

- [**Doxygen документация**](https://netkeep80.github.io/jsonRVM/) — Автогенерируемая документация API
- [**doc/Introduction.md**](doc/Introduction.md) — Введение в Модель Отношений
- [**doc/RVM.md**](doc/RVM.md) — Описание виртуальной машины
- [**doc/RM.md**](doc/RM.md) — Формальное описание Модели Отношений
- [**doc/Dictionary.md**](doc/Dictionary.md) — Словарь базовых операций

### Примеры / Examples

- [**examples/**](examples/) — Примеры программ на Модели Отношений

---

## Сборка / Building

### Требования / Requirements

- CMake 3.20+
- C++17 compatible compiler
- Windows (для поддержки DLL / for DLL support)

### Команды / Commands

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Запуск тестов / Running Tests

```bash
cmake -DENABLE_TESTS=ON ..
cmake --build .
ctest
```

---

## Использование / Usage

```bash
# Показать справку / Show help
rmvm.exe

# Исполнить JSON-программу / Execute JSON program
rmvm.exe program.json

# Загрузить DLL-словарь / Load DLL vocabulary
rmvm.exe vocabulary.dll

# Показать базовый словарь / Show base vocabulary
rmvm.exe rmvm.exe
```

---

## Лицензия / License

<img align="right" src="https://opensource.org/trademarks/opensource/OSI-Approved-License-100x137.png">

Проект лицензирован под [MIT License](https://opensource.org/licenses/MIT):

Copyright &copy; 2016-2021 [Vertushkin Roman Pavlovich](https://vk.com/earthbirthbook)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

---

## Благодарности / Thanks

- [Vladimir Muravyev](https://github.com/vmuravyev) — помощь в разработке терминологического аппарата Модели Отношений

---

## Используемые библиотеки / Third-party Libraries

- [**JSON for Modern C++**](https://github.com/nlohmann/json) — работа с JSON
- [**doctest**](https://github.com/onqtam/doctest) — unit-тестирование
- [**cpp-httplib**](https://github.com/yhirose/cpp-httplib) — HTTP/HTTPS клиент и сервер
- [**xml2json**](https://github.com/Cheedoong/xml2json) — конвертация XML в JSON
- [**str_switch**](https://github.com/Efrit/str_switch) — switch для строк
