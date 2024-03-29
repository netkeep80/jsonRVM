# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
message( "Configuring: ${CMAKE_CURRENT_SOURCE_DIR}")

# Project name
project( http.rm.test
         VERSION ${CMAKE_PROJECT_VERSION}
         DESCRIPTION "http base vocabulary Unit tests"
         LANGUAGES CXX )

# name template
set( TARGET_NAME ${PROJECT_NAME} )

# Name of executable
add_executable( ${TARGET_NAME} )

set(MULTIPLY_TIMEOUT 10)
set(CTEST_OUTPUT_ON_FAILURE 1)

# Adding build-requirements
target_include_directories( ${TARGET_NAME} PRIVATE
    "${DOC_SOURCE_DIR}"
    "${THIRD_PARTY_SOURCE_DIR}/xml2json/include"
    "${THIRD_PARTY_SOURCE_DIR}/cpp-httplib"
    "${THIRD_PARTY_SOURCE_DIR}/doctest"
	"${HTTP_SOURCE_DIR}/include"
    "${COMMON_SOURCE_DIR}/include" )

target_sources( ${TARGET_NAME} PRIVATE
    "${COMMON_SOURCE_DIR}/include/nlohmann/json.hpp"
    "${COMMON_SOURCE_DIR}/include/str_switch/str_switch.h"
    "${COMMON_SOURCE_DIR}/include/vm.rm.h"
    "${COMMON_SOURCE_DIR}/include/interface.h"
    "${COMMON_SOURCE_DIR}/include/database_api.h"
    "${THIRD_PARTY_SOURCE_DIR}/cpp-httplib/httplib.h"
    "${HTTP_SOURCE_DIR}/include/http.rm.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp" )

if( WIN32 )
    add_definitions(-DUNICODE -D_UNICODE)
endif()

# Добавить тесты:
#	COMMAND test_runner $<TARGET_FILE_NAME:${NAME}>

add_test( NAME HttpTest COMMAND ${PROJECT_NAME}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

#add_test(SecondTest ${PROJECT_NAME} -54 -785)

# Установить поведение тестов:
set_tests_properties(HttpTest #SecondTest
	PROPERTIES TIMEOUT ${MULTIPLY_TIMEOUT})

#set_tests_properties(FirstTest PROPERTIES
#FAIL_REGULAR_EXPRESSION 0)
#PASS_REGULAR_EXPRESSION 0)

#set_tests_properties(SecondTest PROPERTIES
#PASS_REGULAR_EXPRESSION "The result is: 42390"
#FAIL_REGULAR_EXPRESSION "Error!")
