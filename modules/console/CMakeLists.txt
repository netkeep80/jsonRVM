# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
message( "Process: ${CMAKE_CURRENT_SOURCE_DIR}")

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
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
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
    "${CMAKE_CURRENT_SOURCE_DIR}/include/base.rm.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/file_database.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp" )
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
#target_include_directories( ${TARGET_NAME} PUBLIC <include-search-dir>... )
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

#--------------------  process subdirectories  --------------------
set( CONSOLE_SOURCE_DIR "${MODULES_SOURCE_DIR}/console" )

option( ENABLE_RVM_TEST "Enable test target build" ON )

if( ENABLE_RVM_TEST )
    add_subdirectory( test )
endif()
