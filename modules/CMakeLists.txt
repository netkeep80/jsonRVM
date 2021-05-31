# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
message( "Configuring: ${CMAKE_CURRENT_SOURCE_DIR}")

# Project name
project( modules VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES CXX )

# Build for C++17 if we can
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

#--------------------  process subdirectories  --------------------
set( COMMON_SOURCE_DIR "${MODULES_SOURCE_DIR}/common" )
set( VOCABULARY_SOURCE_DIR "${MODULES_SOURCE_DIR}/vocabulary" )

option( ENABLE_CONSOLE "Enable relations model console virtual machine target build" ON )
option( ENABLE_VOCABULARY "Enable base vocabulary target build" ON )

if( ENABLE_CONSOLE )
    add_subdirectory( console )
endif()

if( ENABLE_VOCABULARY )
    add_subdirectory( vocabulary )
endif()

