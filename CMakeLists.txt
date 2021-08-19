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

#--------------------  Doxygen  --------------------
# look for Doxygen package
find_package(Doxygen)

if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.out)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # Note: do not put "ALL" - this builds docs together with application EVERY TIME!
    add_custom_target( docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

#--------------------  process subdirectories  --------------------
set( MODULES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules" )
set( THIRD_PARTY_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party" )
set( DOC_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/doc" )

add_subdirectory(modules)

option( ENABLE_TEST "Enable integration test target build" OFF )

if( ENABLE_TESTS )
    add_subdirectory( tests )
endif()

