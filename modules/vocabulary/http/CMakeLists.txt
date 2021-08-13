# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
message( "Configuring: ${CMAKE_CURRENT_SOURCE_DIR}")

# Project name
project( http
         VERSION 0.1.0
         DESCRIPTION "http base vocabulary"
         LANGUAGES CXX )

# target name template
set( TARGET_NAME ${PROJECT_NAME}.rm )

# Name of library file
add_library( ${TARGET_NAME} SHARED )

# Adding build-requirements
target_include_directories( ${TARGET_NAME} PRIVATE
    "${DOC_SOURCE_DIR}"
    "${THIRD_PARTY_SOURCE_DIR}/xml2json/include"
    "${THIRD_PARTY_SOURCE_DIR}/cpp-httplib"
    "${COMMON_SOURCE_DIR}/include/nlohmann"
    "${CMAKE_CURRENT_SOURCE_DIR}/include" )

target_sources( ${TARGET_NAME} PRIVATE
    "${COMMON_SOURCE_DIR}/include/nlohmann/json.hpp"
    "${THIRD_PARTY_SOURCE_DIR}/cpp-httplib/httplib.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/http.rm.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dllmain.cpp" )

# Adding build- and usage-requirements
target_include_directories( ${TARGET_NAME} PUBLIC
    "${COMMON_SOURCE_DIR}/include"
    "${COMMON_SOURCE_DIR}/include/nlohmann" )

target_sources( ${TARGET_NAME} PUBLIC
    "${COMMON_SOURCE_DIR}/include/vm.rm.h"
    "${COMMON_SOURCE_DIR}/include/nlohmann/json.hpp" )

if( WIN32 )
    add_definitions(-DUNICODE -D_UNICODE)
endif()

#--------------------  process subdirectories  --------------------
set( HTTP_SOURCE_DIR "${VOCABULARY_SOURCE_DIR}/http" )

option( ENABLE_HTTP_TEST "Enable test target build" ON )

if( ENABLE_HTTP_TEST )
    add_subdirectory( test )
endif()
