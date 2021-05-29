# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../buildtools/cmake/")
#list(REMOVE_DUPLICATES CMAKE_MODULE_PATH)

# Project name
project( console VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES CXX )

set( TARGET_NAME rmvm )

file( GLOB_RECURSE ${TARGET_NAME}_source_files
    "./src/**.cpp"
    "./src/**.c"
    )

# Name of executable
add_executable( ${TARGET_NAME} )


# Link
#target_link_libraries(${PROJECT_NAME} ${LIBRARY_NAME})

include_directories(include)

#target_include_directories( ${TARGET_NAME} PRIVATE
#    "include"
#    )

string( TIMESTAMP YEAR "%Y" )

target_sources( ${TARGET_NAME} PRIVATE "${${TARGET_NAME}_source_files}" )

#target_link_libraries( ${TARGET_NAME}.exe
#    lib1
#    lib2
#	${LIBRARY_NAME}
#    )

#install( FILES "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}"
#    DESTINATION ${CMAKE_PROJECT_VERSION}
#    )

option( ENABLE_RVM_TEST "Enable test target build" ON )

if( ENABLE_RVM_TEST )
    add_subdirectory( test )
endif()
