# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
message( "Process: ${CMAKE_CURRENT_SOURCE_DIR}")

# Project name
project( rmvm.test
         VERSION ${CMAKE_PROJECT_VERSION}
         DESCRIPTION "Relations model virtual machine Unit tests"
         LANGUAGES CXX )

# name template
set( TARGET_NAME ${PROJECT_NAME} )

# Name of executable
add_executable( ${TARGET_NAME} )

set(MULTIPLY_TIMEOUT 10)
set(CTEST_OUTPUT_ON_FAILURE 1)

# Adding build-requirements
target_include_directories( ${TARGET_NAME} PRIVATE
	"${THIRD_PARTY_SOURCE_DIR}/doctest"
	"${CONSOLE_SOURCE_DIR}/include"
    "${COMMON_SOURCE_DIR}/include"
    "${COMMON_SOURCE_DIR}/include/nlohmann" )
#target_compile_definitions( ${TARGET_NAME} PRIVATE <macro-definitions>... )
#target_compile_options( ${TARGET_NAME} PRIVATE <compiler-option>... )
#target_compile_features( ${TARGET_NAME} PRIVATE <feature>... )
target_sources( ${TARGET_NAME} PRIVATE
    "${COMMON_SOURCE_DIR}/include/nlohmann/json.hpp"
    "${COMMON_SOURCE_DIR}/include/jsonRVM.h"
    "${COMMON_SOURCE_DIR}/include/interface.h"
    "${COMMON_SOURCE_DIR}/include/database_api.h"
    "${CONSOLE_SOURCE_DIR}/include/base.rm.h"
    "${CONSOLE_SOURCE_DIR}/include/file_database.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp" )
#target_link_libraries( ${TARGET_NAME} PRIVATE <dependency>... )
#target_link_options( ${TARGET_NAME} PRIVATE <linker-option>... )
#target_link_directories( ${TARGET_NAME} PRIVATE <linker-search-dir>... )

# Adding usage-requirements
#target_include_directories( ${TARGET_NAME} INTERFACE <include-search-dir>... )
#target_compile_definitions( ${TARGET_NAME} INTERFACE <macro-definitions>... )
#target_compile_options( ${TARGET_NAME} INTERFACE <compiler-option>... )
#target_compile_features( ${TARGET_NAME} INTERFACE <feature>... )
#target_sources( ${TARGET_NAME} INTERFACE <source-file>... )
#target_link_libraries( ${TARGET_NAME} INTERFACE <dependency>... )
#target_link_options( ${TARGET_NAME} INTERFACE <linker-option>... )
#target_link_directories( ${TARGET_NAME} INTERFACE <linker-search-dir>... )

# Adding build- and usage-requirements
#target_include_directories( ${TARGET_NAME} PUBLIC "${COMMON_INCLUDE}" )
#target_compile_definitions( ${TARGET_NAME} PUBLIC <macro-definitions>... )
#target_compile_options( ${TARGET_NAME} PUBLIC <compiler-option>... )
#target_compile_features( ${TARGET_NAME} PUBLIC <feature>... )
#target_sources( ${TARGET_NAME} PUBLIC "${COMMON_INCLUDE}/jsonRVM.h" )
#target_link_libraries( ${TARGET_NAME} PUBLIC <dependency>... )
#target_link_options( ${TARGET_NAME} PUBLIC <linker-option>... )
#target_link_directories( ${TARGET_NAME} PUBLIC <linker-search-dir>... )

if( WIN32 )
    add_definitions(-DUNICODE -D_UNICODE)
endif()

# Добавить тесты:
#	COMMAND test_runner $<TARGET_FILE_NAME:${NAME}>

add_test( NAME FirstTest COMMAND ${PROJECT_NAME}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

#add_test(SecondTest ${PROJECT_NAME} -54 -785)

# Установить поведение тестов:
set_tests_properties(FirstTest #SecondTest
	PROPERTIES TIMEOUT ${MULTIPLY_TIMEOUT})

#set_tests_properties(FirstTest PROPERTIES
#FAIL_REGULAR_EXPRESSION 0)
#PASS_REGULAR_EXPRESSION 0)

#set_tests_properties(SecondTest PROPERTIES
#PASS_REGULAR_EXPRESSION "The result is: 42390"
#FAIL_REGULAR_EXPRESSION "Error!")
