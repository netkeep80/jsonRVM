#pragma once
#include "interface.h"
#include "nlohmann/json.hpp"

namespace rm
{
	using namespace std;
	using namespace nlohmann;

	/**
	 * @class database_api
	 * @brief Абстрактный интерфейс для работы с сущностями базы данных через JSON
	 *
	 * Класс реализует паттерн Curiously Recurring Template Pattern (CRTP) через наследование
	 * от interface_t<database_api>, обеспечивая статический полиморфизм. Требует обязательной
	 * реализации методов в производных классах.
	 *
	 * Основные возможности:
	 * - Работа с сущностями в формате JSON
	 * - Получение/добавление сущностей по ID
	 * - Поиск сущностей с помощью JSON-запросов
	 *
	 * @method get_entity   Получение сущности из базы по идентификатору
	 * @method add_entity   Добавление новой сущности в базу с генерацией ID
	 * @method query_entity Поиск сущностей по сложному запросу
	 *
	 * @note Все методы передают JSON-объекты по ссылке для избежания копирования
	 * @warning Попытка вызова методов без реализации приведет к исключению runtime_error
	 *
	 * Пример использования:
	 * @code
	 * class db_implementation : public database_api {
	 *     // ... реализация методов ...
	 * };
	 *
	 * db_implementation db;
	 * json entity;
	 * db.get_entity(entity, "user_123");
	 * @endcode
	 */
	class database_api : public interface_t<database_api>
	{
	public:
		virtual void get_entity(json &ent, const string &ent_id) { this->impl().get_entity(ent, ent_id); }
		virtual void add_entity(const json &ent, string &ent_id) { this->impl().add_entity(ent, ent_id); }
		virtual void query_entity(json &ent, const json &query) { this->impl().query_entity(ent, query); }
	};
}
