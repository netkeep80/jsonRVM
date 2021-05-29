# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# Project name
project(rm)

# Включить поддержку тестирования:
enable_testing()

# Turn on any compiler-specific flags
#if (WIN32)
#	# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
#else()
#	# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
#	# if ("$ENV{SANITIZE}" STREQUAL "ON")
#		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#	# endif()
#	
#	if(APPLE)
#		set(CMAKE_XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Debug] "YES")
#		set(CMAKE_XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS[variant=MinSizeRel] "NO")
#		set(CMAKE_XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS[variant=RelWithDebInfo] "YES")
#		set(CMAKE_XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Release] "NO")
#	endif()
#endif()


set(CMAKE_PROJECT_VERSION 0.1.0 )

include_directories(third_party/doctest)

#include(FetchContent)

# Downloading abseil package
#FetchContent_Declare(
#  abseil
#  URL https://github.com/abseil/abseil-cpp/archive/master.zip)
#FetchContent_Populate(abseil)
#message(STATUS "abseil is available in " ${abseil_SOURCE_DIR})

message( "--------" )
message( "CMAKE_SYSTEM: ${CMAKE_SYSTEM}" )
message( "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}" )
message( "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}" )
message( "Current revision is ${CMAKE_PROJECT_VERSION}" )
message( "CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}" )
message( "CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}" )
message( "--------" )

# Subdirectories to build
add_subdirectory(modules)

option( ENABLE_TEST "Enable integration test target build" OFF )

if( ENABLE_TESTS )
    add_subdirectory( tests )
endif()

#ExternalProject_Add( )

# Modern template commands

# Adding build-requirements
#target_include_directories( ${TARGET_NAME} PRIVATE <include-search-dir>... )
#target_compile_definitions( ${TARGET_NAME} PRIVATE <macro-definitions>... )
#target_compile_options( ${TARGET_NAME} PRIVATE <compiler-option>... )
#target_compile_features( ${TARGET_NAME} PRIVATE <feature>... )
#target_sources( ${TARGET_NAME} PRIVATE <source-file>... )
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
#target_sources( ${TARGET_NAME} PUBLIC <source-file>... )
#target_link_libraries( ${TARGET_NAME} PUBLIC <dependency>... )
#target_link_options( ${TARGET_NAME} PUBLIC <linker-option>... )
#target_link_directories( ${TARGET_NAME} PUBLIC <linker-search-dir>... )