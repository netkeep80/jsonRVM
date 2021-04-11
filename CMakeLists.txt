# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# Project name
project(CoopLKI)

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

#set(LIBRARY_NAME cpplib)

set(CMAKE_PROJECT_VERSION 0.1.0 )

#include(FetchContent)

# Downloading abseil package
#FetchContent_Declare(
#  abseil
#  URL https://github.com/abseil/abseil-cpp/archive/master.zip)
#FetchContent_Populate(abseil)
#message(STATUS "abseil is available in " ${abseil_SOURCE_DIR})


# Subdirectories to build
add_subdirectory(src)

#ExternalProject_Add( )
