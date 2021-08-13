#pragma once

namespace rm
{
	template<class implementation_t>
	class interface_t
	{
		implementation_t* __link{ nullptr };
	protected:
		void	link(implementation_t* ptr) { __link = ptr; }
		auto&	impl() const
		{
			if (__link)	return *__link;
			else		throw runtime_error("Method interface not implemented or interface_t<"s + typeid(implementation_t).name() + "> not linked"s);
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
	void func1(void) { cout << __FUNCTION__ << endl; }
	void func2(void) { cout << __FUNCTION__ << endl; }
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