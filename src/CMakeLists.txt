# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

#set( CMAKE_VERBOSE_MAKEFILE ON )

#list( APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../buildtools/cmake/" )
#list( REMOVE_DUPLICATES CMAKE_MODULE_PATH )

#find_package( SVNRevision REQUIRED )

# Project name
project( LKInode VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES CXX )

message( "--------" )
message( "CMAKE_SYSTEM: ${CMAKE_SYSTEM}" )
message( "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}" )
message( "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}" )
message( "Current revision is ${CMAKE_PROJECT_VERSION}" )
message( "CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}" )
message( "--------" )


# Build for C++17 if we can
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

# Where any include files are
include_directories(3p/nlohmann)
include_directories(3p/doctest)

if( WIN32 )
#    set( CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static" )
endif()

option( ENABLE_TRMVM "Enable transactional relationship model virtual machine target build" ON )
option( ENABLE_TEST "Enable integration test target build" ON )

if( ENABLE_TRMVM )
    add_subdirectory( trmvm )
endif()

if( ENABLE_TEST )
#    add_subdirectory( test )
endif()
