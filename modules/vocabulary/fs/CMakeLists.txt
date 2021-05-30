# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
message( "Process: ${CMAKE_CURRENT_SOURCE_DIR}")

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
    "${COMMON_SOURCE_DIR}/include/nlohmann"
    "${CMAKE_CURRENT_SOURCE_DIR}/include" )
#target_compile_definitions( ${TARGET_NAME} PRIVATE <macro-definitions>... )
#target_compile_options( ${TARGET_NAME} PRIVATE <compiler-option>... )
#target_compile_features( ${TARGET_NAME} PRIVATE <feature>... )
target_sources( ${TARGET_NAME} PRIVATE
    "${COMMON_SOURCE_DIR}/include/nlohmann/json.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/fs.rm.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/fs.rm.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dllmain.cpp" )
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
target_include_directories( ${TARGET_NAME} PUBLIC
    "${COMMON_SOURCE_DIR}/include"
    "${COMMON_SOURCE_DIR}/include/nlohmann" )
#target_compile_definitions( ${TARGET_NAME} PUBLIC <macro-definitions>... )
#target_compile_options( ${TARGET_NAME} PUBLIC <compiler-option>... )
#target_compile_features( ${TARGET_NAME} PUBLIC <feature>... )
target_sources( ${TARGET_NAME} PUBLIC
    "${COMMON_SOURCE_DIR}/include/jsonRVM.h"
    "${COMMON_SOURCE_DIR}/include/nlohmann/json.hpp" )
#target_link_libraries( ${TARGET_NAME} PUBLIC <dependency>... )
#target_link_options( ${TARGET_NAME} PUBLIC <linker-option>... )
#target_link_directories( ${TARGET_NAME} PUBLIC <linker-search-dir>... )

add_definitions(-DFSRM_EXPORTS)

if( WIN32 )
    add_definitions(-DUNICODE -D_UNICODE)
endif()

#--------------------  process subdirectories  --------------------
option( ENABLE_FS_TEST "Enable test target build" OFF )

if( ENABLE_FS_TEST )
    add_subdirectory( test )
endif()
