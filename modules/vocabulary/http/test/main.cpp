#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include "http.rm.h"

using namespace std;
using namespace rm;

#include <thread>
#include <chrono>
#include <cstdio>
#include <httplib.h>

#define SERVER_CERT_FILE "./cert.pem"
#define SERVER_PRIVATE_KEY_FILE "./key.pem"

using namespace httplib;

std::string dump_headers(const Headers& headers) {
	std::string s;
	char buf[BUFSIZ];

	for (auto it = headers.begin(); it != headers.end(); ++it) {
		const auto& x = *it;
		snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
		s += buf;
	}

	return s;
}

std::string log(const Request& req, const Response& res) {
	std::string s;
	char buf[BUFSIZ];

	s += "================================\n";

	snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
		req.version.c_str(), req.path.c_str());
	s += buf;

	std::string query;
	for (auto it = req.params.begin(); it != req.params.end(); ++it) {
		const auto& x = *it;
		snprintf(buf, sizeof(buf), "%c%s=%s",
			(it == req.params.begin()) ? '?' : '&', x.first.c_str(),
			x.second.c_str());
		query += buf;
	}
	snprintf(buf, sizeof(buf), "%s\n", query.c_str());
	s += buf;

	s += dump_headers(req.headers);

	s += "--------------------------------\n";

	snprintf(buf, sizeof(buf), "%d %s\n", res.status, res.version.c_str());
	s += buf;
	s += dump_headers(res.headers);
	s += "\n";

	if (!res.body.empty()) { s += res.body; }

	s += "\n";

	return s;
}


void    server(void) {
	try {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
		SSLServer svr(SERVER_CERT_FILE, SERVER_PRIVATE_KEY_FILE);
#else
		Server svr;
#endif

		if (!svr.is_valid()) {
			printf("server has an error...\n");
			return;
		}

		svr.Get("/", [=](const Request& /*req*/, Response& res) {
			res.set_redirect("/hi");
			});

		svr.Get("/hi", [](const Request& /*req*/, Response& res) {
			res.set_content("Hello World!\n", "text/plain");
			});

		svr.Get("/slow", [](const Request& /*req*/, Response& res) {
			std::this_thread::sleep_for(std::chrono::seconds(2));
			res.set_content("Slow...\n", "text/plain");
			});

		svr.Get("/dump", [](const Request& req, Response& res) {
			res.set_content(dump_headers(req.headers), "text/plain");
			});

		svr.Get("/stop",
			[&](const Request& /*req*/, Response& /*res*/) { svr.stop(); });

		svr.set_error_handler([](const Request& /*req*/, Response& res) {
			const char* fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
			char buf[BUFSIZ];
			snprintf(buf, sizeof(buf), fmt, res.status);
			res.set_content(buf, "text/html");
			});

		svr.set_logger([](const Request& req, const Response& res) {
			printf("%s", log(req, res).c_str());
			});

		svr.listen("localhost", 8080);

	}
	catch (exception& e) {
		cout << e.what();
	}
	catch (...) {
	}
}


DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

////////////////////////////////////////////////////////////////////////////////


TEST_CASE("Test http://localhost:8080/hi") {
	MESSAGE("result:");

	thread _thread(server);
	_thread.detach();

	jsonRVM root;
	//	base vocabulary
	ImportRelationsModel(root);

	root[""] = json::parse(R"(
{
   "$obj" : {
      "URI" : "http://localhost:8080/hi"
   },
   "$rel" : "http/GET/urlencoded"
}
)");

	json    res;
	EntContext $(res, root[""]);
	root.JSONExec($, root[""]);
	cout << res.dump(2) << endl;

	CHECK(res["body"].get_ref<string&>() == "Hello World!\n"s);
}


////////////////////////////////////////////////////////////////////////////////

/*
int factorial(const int number) {
	return number < 1 ? 1 : number <= 1 ? number : factorial(number - 1) * number;
}

TEST_CASE("testing the factorial function") {
	CHECK(factorial(0) == 1);
	CHECK(factorial(1) == 1);
	CHECK(factorial(2) == 2);
	CHECK(factorial(3) == 6);
	CHECK(factorial(10) == 3628800);
}


TEST_CASE("lots of nested subcases") {
	cout << endl << "root" << endl;
	SUBCASE("") {
		cout << "1" << endl;
		SUBCASE("") { cout << "1.1" << endl; }
	}
	SUBCASE("") {
		cout << "2" << endl;
		SUBCASE("") { cout << "2.1" << endl; }
		SUBCASE("") {
			// whops! all the subcases below shouldn't be discovered and executed!
//            FAIL("");

			cout << "2.2" << endl;
			SUBCASE("") {
				cout << "2.2.1" << endl;
				SUBCASE("") { cout << "2.2.1.1" << endl; }
				SUBCASE("") { cout << "2.2.1.2" << endl; }
			}
		}
		SUBCASE("") { cout << "2.3" << endl; }
		SUBCASE("") { cout << "2.4" << endl; }
	}
}

static void call_func() {
	SUBCASE("from function...") {
		MESSAGE("print me twice");
		SUBCASE("sc1") {
			MESSAGE("hello! from sc1");
		}
		SUBCASE("sc2") {
			MESSAGE("hello! from sc2");
		}
	}
}

TEST_CASE("subcases can be used in a separate function as well") {
	call_func();
	MESSAGE("lala");
}

SCENARIO("vectors can be sized and resized") {
	GIVEN("A vector with some items") {
		std::vector<int> v(5);

		REQUIRE(v.size() == 5);
		REQUIRE(v.capacity() >= 5);

		WHEN("the size is increased") {
			v.resize(10);

			THEN("the size and capacity change") {
				CHECK(v.size() == 20);
				CHECK(v.capacity() >= 10);
			}
		}
		WHEN("the size is reduced") {
			v.resize(0);

			THEN("the size changes but not capacity") {
				CHECK(v.size() == 0);
				CHECK(v.capacity() >= 5);
			}
		}
		WHEN("more capacity is reserved") {
			v.reserve(10);

			THEN("the capacity changes but not the size") {
				CHECK(v.size() == 5);
				CHECK(v.capacity() >= 10);
			}
		}
		WHEN("less capacity is reserved") {
			v.reserve(0);

			THEN("neither size nor capacity are changed") {
				CHECK(v.size() == 10);
				CHECK(v.capacity() >= 5);
			}
		}
	}
}

TEST_CASE("test case should fail even though the last subcase passes") {
	SUBCASE("one") {
		CHECK(false);
	}
	SUBCASE("two") {
		CHECK(true);
	}
}

TEST_CASE("fails from an exception but gets re-entered to traverse all subcases") {
	SUBCASE("level zero") {
		SUBCASE("one") {
			CHECK(false);
		}
		SUBCASE("two") {
			CHECK(false);
		}

		//throw_if(true, "failure... but the show must go on!");
	}
}

static void checks(int data)
{
	DOCTEST_SUBCASE("check data 1") { REQUIRE(data % 2 == 0); }
	DOCTEST_SUBCASE("check data 2") { REQUIRE(data % 4 == 0); }
}

TEST_CASE("Nested - related to https://github.com/onqtam/doctest/issues/282")
{
	DOCTEST_SUBCASE("generate data variant 1")
	{
		int data(44);

		// checks
		checks(data);
	}
	DOCTEST_SUBCASE("generate data variant 1")
	{
		int data(80);

		// checks (identical in both variants)
		checks(data);
	}
}

DOCTEST_MSVC_SUPPRESS_WARNING(5045) // Spectre mitigation stuff
DOCTEST_GCC_SUPPRESS_WARNING("-Wuseless-cast") // for the std::string() cast
#undef SUBCASE
#define SUBCASE(...) DOCTEST_SUBCASE(std::string(__VA_ARGS__).c_str())

TEST_CASE("subcases with changing names") {
	for (int i = 0; i < 2; ++i) {
		SUBCASE("outer " + std::to_string(i)) {
			for (int k = 0; k < 2; ++k) {
				SUBCASE("inner " + std::to_string(k)) {
					MESSAGE("msg!");
				}
			}
		}
	}
	SUBCASE("separate") {
		MESSAGE("separate msg!");
	}
}
*/
