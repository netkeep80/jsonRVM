# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
message( "Configuring: ${CMAKE_CURRENT_SOURCE_DIR}")

# Project name
project( vocabulary VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES CXX )

#--------------------  process subdirectories  --------------------
# FS module uses Windows-specific APIs (FindFirstFile, WIN32_FIND_DATA, etc.)
# HTTP module has MSVC-specific template code that requires updates for GCC
# Disable both by default on non-Windows platforms
if( WIN32 )
    option( ENABLE_FS "Enable file system base vocabulary target build" ON )
    option( ENABLE_HTTP "Enable http base vocabulary target build" ON )
else()
    option( ENABLE_FS "Enable file system base vocabulary target build" OFF )
    option( ENABLE_HTTP "Enable http base vocabulary target build" OFF )
endif()
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

