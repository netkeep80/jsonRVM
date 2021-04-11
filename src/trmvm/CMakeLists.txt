# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../buildtools/cmake/")
#list(REMOVE_DUPLICATES CMAKE_MODULE_PATH)

# Project name
project( trmvm VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES CXX )

option( TRM_VOC_FS "Enable file system base vocabulary" OFF )
option( TRM_VOC_HTTP "Enable http base vocabulary" OFF )
option( TRM_VOC_IMGUI "Enable ImGui base vocabulary" OFF )

set( TARGET_NAME trmvm )

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
#set_target_properties( ${TARGET_NAME}.exe PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/BP2008.ld )
#set_target_properties( ${TARGET_NAME}.exe PROPERTIES LINK_FLAGS "-T\"${CMAKE_CURRENT_SOURCE_DIR}/src/BP2008.ld\" -Wl,-Map=${TARGET_NAME}.map")

if( TRM_VOC_FS )
    target_compile_definitions( ${TARGET_NAME} PRIVATE
        ENABLE_TRM_VOC_FS
        )
endif()

if( TRM_VOC_HTTP )
    target_compile_definitions( ${TARGET_NAME} PRIVATE
        ENABLE_TRM_VOC_FS
        )
endif()

if( TRM_VOC_IMGUI )
    target_compile_definitions( ${TARGET_NAME} PRIVATE
        ENABLE_TRM_VOC_FS
        )
endif()

target_sources( ${TARGET_NAME} PRIVATE "${${TARGET_NAME}_source_files}" )

#target_link_libraries( ${TARGET_NAME}.exe
#    monitorapi
#    bpmm
#	${LIBRARY_NAME}
#    )

#add_custom_target( ${TARGET_NAME}_NoCRC.hex
#    arm-none-eabi-objcopy -Oihex ${TARGET_NAME}.elf ${TARGET_NAME}_NoCRC.hex
#    DEPENDS ${TARGET_NAME}.elf
#    )

#install( FILES "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}"
#    DESTINATION ${CMAKE_PROJECT_VERSION}
#    )

option( ENABLE_TEST "Enable test target build" OFF )

if( ENABLE_TEST )
    add_subdirectory( test )
endif()
