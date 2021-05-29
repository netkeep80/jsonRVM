# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

#set( CMAKE_VERBOSE_MAKEFILE ON )

#list( APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../buildtools/cmake/" )
#list( REMOVE_DUPLICATES CMAKE_MODULE_PATH )

#find_package( SVNRevision REQUIRED )

# Project name
project( vocabulary VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES CXX )

# Build for C++17 if we can
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

# Where any include files are
#include_directories(${CMAKE_SOURCE_DIR}/modules/common/include)
include_directories(../common/include)
include_directories(../common/include/nlohmann)

if( WIN32 )
#    set( CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static" )
endif()

option( ENABLE_FS "Enable file system base vocabulary target build" ON )
option( ENABLE_HTTP "Enable http base vocabulary target build" OFF )
option( ENABLE_IMGUI "Enable ImGui base vocabulary target build" OFF )

if( ENABLE_FS )
    add_subdirectory( fs )
endif()

if( ENABLE_HTTP )
    add_subdirectory( http )
endif()

if( ENABLE_IMGUI )
    add_subdirectory( imgui )
endif()

