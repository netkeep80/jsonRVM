#pragma once
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace rm
{
	using namespace std::literals;
	/**
	 * @class interface_t
	 * @brief Базовый шаблонный класс для реализации статического полиморфизма через CRTP
	 *
	 * @tparam implementation_t Тип конкретной реализации, должен быть наследником interface_t
	 *
	 * Реализует паттерн Curiously Recurring Template Pattern (CRTP), обеспечивая:
	 * - Механизм связывания интерфейса с конкретной реализацией
	 * - Безопасный доступ к методам реализации
	 * - Защиту от использования несвязанного интерфейса
	 *
	 * @method link       Связывает интерфейс с конкретной реализацией (protected)
	 * @method impl       Возвращает ссылку на реализацию с проверкой связывания (protected)
	 *
	 * @note Наследники должны:
	 * 1. Связать реализацию через link() в конструкторе
	 * 2. Реализовать чисто виртуальные методы интерфейса
	 * 3. Использовать impl() для доступа к методам реализации
	 *
	 * @warning Попытка использования методов интерфейса без предварительного связывания
	 * с реализацией приведёт к исключению runtime_error
	 *
	 * Пример использования:
	 * @code
	 * class MyInterface : public interface_t<MyInterface> {
	 * public:
	 *     void foo() { impl().foo_impl(); }
	 * };
	 *
	 * class MyImplementation : public MyInterface {
	 * public:
	 *     MyImplementation() { link(this); }
	 *     void foo_impl() {
	 * 		   // реализация
	 *     }
	 * };
	 * @endcode
	 */
	template <class implementation_t>
	class interface_t
	{
		implementation_t *__link{nullptr};

	protected:
		void link(implementation_t *ptr) { __link = ptr; }
		auto &impl() const
		{
			if (__link)
				return *__link;
			else
				throw std::runtime_error("Method interface not implemented or interface_t<"s + typeid(implementation_t).name() + "> not linked"s);
		}
	};
}

/* usage of interface_t


template<class database_impl_t>
class database_api : public interface_t<database_impl_t>
{
public:
	void func1(void) { this->impl().func1(); }
	void func2(void) { this->impl().func2(); }
	void func3(void) { this->impl().func3(); }
};


class file_database_t : public database_api<file_database_t>
{
public:
	void func1(void) { cout << __func__ << endl; }
	void func2(void) { cout << __func__ << endl; }
};


template<class database_impl_t>
class jsonRVM : protected database_api<database_impl_t>
{
	using db_interface = database_api<database_impl_t>;
	friend db_interface;

public:
	jsonRVM(database_impl_t* db) { db_interface::link(db); }

	void test1()
	{
		try
		{
			this->func1();
			this->func3();
			this->link(nullptr);
			this->func2();
		}
		catch (exception& e)
		{
			cout << e.what() << endl;
		}
	}
};


class example3
{
public:
	example3()
	{
		auto* db = new file_database_t();
		jsonRVM<file_database_t> rmvm(db);
		rmvm.test1();
		delete db;
	}
} static_example3;


*/