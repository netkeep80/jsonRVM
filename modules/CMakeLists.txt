# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

#set( CMAKE_VERBOSE_MAKEFILE ON )

#list( APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../buildtools/cmake/" )
#list( REMOVE_DUPLICATES CMAKE_MODULE_PATH )

#find_package( SVNRevision REQUIRED )

# Project name
project( modules VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES CXX )

# Build for C++17 if we can
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

# Where any include files are
include_directories(common/include)
include_directories(common/include/nlohmann)

if( WIN32 )
#    set( CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static" )
endif()

set( COMMON_INCLUDE "${CMAKE_SOURCE_DIR}/modules/common/include" )

#subdirectories
option( ENABLE_CONSOLE "Enable relationship model console virtual machine target build" ON )
option( ENABLE_VOCABULARY "Enable base vocabulary target build" ON )

if( ENABLE_CONSOLE )
    add_subdirectory( console )
endif()

if( ENABLE_VOCABULARY )
    add_subdirectory( vocabulary )
endif()

