# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
message( "Configuring: ${CMAKE_CURRENT_SOURCE_DIR}")

# Project name
project(rm)

# Включить поддержку тестирования:
enable_testing()

set(CMAKE_PROJECT_VERSION 3.0.0 )

message( "------------------------------------------------" )
message( "CMAKE_SYSTEM: ${CMAKE_SYSTEM}" )
message( "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}" )
message( "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}" )
message( "Current revision is ${CMAKE_PROJECT_VERSION}" )
message( "CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}" )
message( "CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}" )
message( "------------------------------------------------" )

#--------------------  process subdirectories  --------------------
set( MODULES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules" )
set( THIRD_PARTY_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party" )
set( DOC_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/doc" )

add_subdirectory(modules)

option( ENABLE_TEST "Enable integration test target build" OFF )

if( ENABLE_TESTS )
    add_subdirectory( tests )
endif()
