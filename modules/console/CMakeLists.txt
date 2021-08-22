# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
message( "Configuring: ${CMAKE_CURRENT_SOURCE_DIR}")

# Project name
project( console
         VERSION ${CMAKE_PROJECT_VERSION}
         DESCRIPTION "Console relations model virtual machine"
         LANGUAGES CXX )

# name template
set( TARGET_NAME rmvm )

# Name of executable
add_executable( ${TARGET_NAME} )

# Adding build-requirements
target_include_directories( ${TARGET_NAME} PRIVATE
    "${DOC_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${COMMON_SOURCE_DIR}/include"
    "${COMMON_SOURCE_DIR}/include/nlohmann" )

target_sources( ${TARGET_NAME} PRIVATE
    "${DOC_SOURCE_DIR}/Introduction.md"
    "${COMMON_SOURCE_DIR}/include/nlohmann/json.hpp"
    "${COMMON_SOURCE_DIR}/include/vm.rm.h"
    "${COMMON_SOURCE_DIR}/include/interface.h"
    "${COMMON_SOURCE_DIR}/include/database_api.h"
    "${COMMON_SOURCE_DIR}/include/string_utils.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/base.rm.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/file_database.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/dll.rm.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp" )

if( WIN32 )
    add_definitions(-DUNICODE -D_UNICODE)
endif()

#--------------------  process subdirectories  --------------------
set( CONSOLE_SOURCE_DIR "${MODULES_SOURCE_DIR}/console" )

option( ENABLE_RVM_TEST "Enable test target build" ON )

if( ENABLE_RVM_TEST )
    add_subdirectory( test )
endif()
