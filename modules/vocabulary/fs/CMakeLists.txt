# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
message( "Configuring: ${CMAKE_CURRENT_SOURCE_DIR}")

# Project name
project( fs
         VERSION 0.1.0
         DESCRIPTION "File system base vocabulary"
         LANGUAGES CXX )

# target name template
set( TARGET_NAME ${PROJECT_NAME}.rm )

# Name of library file
add_library( ${TARGET_NAME} SHARED )

# Adding build-requirements
target_include_directories( ${TARGET_NAME} PRIVATE
    "${DOC_SOURCE_DIR}"
    "${COMMON_SOURCE_DIR}/include/nlohmann"
    "${CMAKE_CURRENT_SOURCE_DIR}/include" )

target_sources( ${TARGET_NAME} PRIVATE
    "${COMMON_SOURCE_DIR}/include/nlohmann/json.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/fs.rm.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/fs.rm.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dllmain.cpp" )

# Adding build- and usage-requirements
target_include_directories( ${TARGET_NAME} PUBLIC
    "${COMMON_SOURCE_DIR}/include"
    "${COMMON_SOURCE_DIR}/include/nlohmann" )

target_sources( ${TARGET_NAME} PUBLIC
    "${COMMON_SOURCE_DIR}/include/vm.rm.h"
    "${COMMON_SOURCE_DIR}/include/nlohmann/json.hpp" )

add_definitions(-DFSRM_EXPORTS)

if( WIN32 )
    add_definitions(-DUNICODE -D_UNICODE)
endif()

#--------------------  process subdirectories  --------------------
option( ENABLE_FS_TEST "Enable test target build" OFF )

if( ENABLE_FS_TEST )
    add_subdirectory( test )
endif()
