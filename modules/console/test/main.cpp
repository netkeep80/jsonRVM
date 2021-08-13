#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "jsonRVM.h"
#include "base.rm.h"
#include "file_database.h"

using namespace std;
using namespace rm;

namespace rm
{
    class test_database_t : public database_api
    {
        test_database_t* __this{ nullptr };
        string PathFolder;
        map<string, string> __rm;
        int count{ 0 };
    public:
        test_database_t(const string& path)
            : PathFolder(path)
        {
            __this = this;
            __rm["ent1"] = R"(
                {
                    "id" : "ent1"
                }
            )";
            count++;
            __rm["ent2"] = R"(
                {
                    "id" : "ent2"
                }
            )";
            count++;
        }

        virtual void	get_entity(json& ent, const string& ent_id) override
        {
            auto res = __rm.find(ent_id);
			if (res == __rm.end())
                throw runtime_error(__FUNCTION__ + ": Can't find entity "s + ent_id);

            ent = json::parse((*res).second);
        }

        virtual void	add_entity(const json& ent, string& ent_id) override
        {
            ent_id = "ent"s + to_string(reinterpret_cast<size_t>(&ent));
            __rm[ent_id] = ent.dump();
            count++;
        }

        virtual void	query_entity(json& ent, const json& query) override
        {
            cout << __FUNCTION__ << endl;
        }
    };
}

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

////////////////////////////////////////////////////////////////////////////////

TEST_CASE("absolute addressing in rmodel") {
    MESSAGE("result:");
    test_database_t	db(".\\");
    jsonRVM root(&db);
    //	base vocabulary
    ImportRelationsModel(root);
    json    res;
    char* fileNameInput = "absolute_addressing.json";
    std::ifstream in(fileNameInput);
    REQUIRE(in.good());
    in >> root[""];
    EntContext $(res, root[""]);
    root.JSONExec($, root[""]);
    cout << res.dump(2) << endl;
    
    CHECK(res["ent1"]["id"].get_ref<string&>() == "ent1"s);
    CHECK(res["ent2"]["id"].get_ref<string&>() == "ent2"s);
    CHECK(res["ent3"]["val"]["id"].get_ref<string&>() == "ent3"s);
}


TEST_CASE("relative addressing in rmodel") {
    MESSAGE("result:");
    test_database_t	db(".\\");
    jsonRVM root(&db);
    //	base vocabulary
    ImportRelationsModel(root);
    json    res;
    char* fileNameInput = "relative_addressing.json";
    std::ifstream in(fileNameInput);
    REQUIRE(in.good());
    in >> root[""];
    EntContext $(res, root[""]);
    root.JSONExec($, root[""]);
    cout << res.dump(2) << endl;
    
    CHECK(res["$up3ent"]["id"].get_ref<string&>() == "$up3ent/id"s);
    CHECK(res["$up3sub"]["id"].get_ref<string&>() == "$up3ent/$sub"s);
    CHECK(res["$up3obj"]["id"].get_ref<string&>() == "$up3ent/$obj"s);
    CHECK(res["$up3its"]["id"].get_ref<string&>() == "$up3its/id"s);

    CHECK(res["$up2ent"]["id"].get_ref<string&>() == "$up2ent/id"s);
    CHECK(res["$up2sub"]["id"].get_ref<string&>() == "$up2ent/$sub"s);
    CHECK(res["$up2obj"]["id"].get_ref<string&>() == "$up2ent/$obj"s);
    CHECK(res["$up2its"]["id"].get_ref<string&>() == "$up2its/id"s);

    CHECK(res["$up1ent"]["id"].get_ref<string&>() == "$up1ent/id"s);
    CHECK(res["$up1sub"]["id"].get_ref<string&>() == "$up1ent/$sub"s);
    CHECK(res["$up1obj"]["id"].get_ref<string&>() == "$up1ent/$obj"s);
    CHECK(res["$up1its"]["id"].get_ref<string&>() == "$up1its/id"s);

    CHECK(res["$ent"]["id"].get_ref<string&>() == "$ent/id"s);
    CHECK(res["$sub"]["id"].get_ref<string&>() == "$ent/$sub"s);
    CHECK(res["$obj"]["id"].get_ref<string&>() == "$ent/$obj"s);
    CHECK(res["$its"]["id"].get_ref<string&>() == "$its/id"s);
}

TEST_CASE("testing call version.json") {
    MESSAGE("result:");
    file_database_t	db(".\\");
    jsonRVM root(&db);
    //	base vocabulary
    ImportRelationsModel(root);
    json    res;
    char *fileNameInput = "version.json";

    try
    {
        std::ifstream in(fileNameInput);
        REQUIRE(in.good());
        in >> root[""];
        
        EntContext $(res, root[""]);
        root.JSONExec($, root[""]);
        CHECK(res["rmvm"]["version"].get_ref<string&>() == "3.0.0"s);
    }
    catch (json& j) { throw json({ { __FUNCTION__, j } }); }
    catch (json::exception& e) { throw json({ { __FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id) } }); }
    catch (std::exception& e) { throw json({ { __FUNCTION__, "std::exception: "s + e.what() } }); }
    catch (...) { throw json({ { __FUNCTION__, "unknown exception"s } }); }
}


TEST_CASE("testing base entity 'where'") {
    MESSAGE("result:");
    file_database_t	db(".\\");
    jsonRVM root(&db);
    //	base vocabulary
    ImportRelationsModel(root);
    json    res;
    char* fileNameInput = "where_test.json";

    try
    {
        std::ifstream in(fileNameInput);
        REQUIRE(in.good());
        in >> root[""];

        EntContext $(res, root[""]);
        root.JSONExec($, root[""]);
        cout << res.dump(2) << endl;
        CHECK(res[0].get_ref<string&>() == "4"s);
    }
    catch (json& j) { throw json({ { __FUNCTION__, j } }); }
    catch (json::exception& e) { throw json({ { __FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id) } }); }
    catch (std::exception& e) { throw json({ { __FUNCTION__, "std::exception: "s + e.what() } }); }
    catch (...) { throw json({ { __FUNCTION__, "unknown exception"s } }); }

    /*
    * надо ещё написать кейсы на невалидный $obj (когда его тип не array)
    * надо ещё написать кейсы на невалидный $sub
    */
}


TEST_CASE("performance test") {
    MESSAGE("result:");
    file_database_t	db(".\\");
    jsonRVM root(&db);
    //	base vocabulary
    ImportRelationsModel(root);
    json    res;
    cout << root.exec(res, json("performance")).dump(2);
    CHECK(res["report"][0].get_ref<string&>() == "Parameter;Average;StandardDeviation;Correlation;Successful;MeasCount"s);
    CHECK(res["report"][1].get_ref<string&>() == "param11;-0.565942;8.813454;0.879269;100.000000;688"s);
    CHECK(res["report"][2].get_ref<string&>() == "param6;0.145000;4.852494;0.911962;100.000000;688"s);
    CHECK(res["report"][3].get_ref<string&>() == "param1;-10.724466;22.198186;0.294701;99.562044;685"s);
    CHECK(res["report"][4].get_ref<string&>() == "param7;56.969527;49.532709;0.580730;99.640934;557"s);
    CHECK(res["report"][5].get_ref<string&>() == "param12;;;;0.000000;688"s);
    CHECK(res["report"][6].get_ref<string&>() == "param9;-7.835917;2.119798;0.470438;99.273256;688"s);
}

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
