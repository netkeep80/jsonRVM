# Require CMAKE 3.20 or higher
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# Project name
project(trmvm.test VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES CXX)

#find_package(doctest REQUIRED)

# If you create new headers/cpp files, add them to these list!
#set(HEADER_FILES
#	doctest.h
#)

set(SOURCE_FILES	
	main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

#target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
#target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARY_NAME})
#target_link_libraries(test ${LIBRARY_NAME})

set(MULTIPLY_TIMEOUT 10)
set(CTEST_OUTPUT_ON_FAILURE 1)
#message(${CMAKE_CTEST_COMMAND})
#message(${CMAKE_COMMAND})

# Добавить тесты:
#	COMMAND test_runner $<TARGET_FILE_NAME:${NAME}>

#add_test( FirstTest ${PROJECT_NAME} )

add_test( NAME FirstTest COMMAND ${PROJECT_NAME}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

#add_test(SecondTest ${PROJECT_NAME} -54 -785)

# Установить поведение тестов:
set_tests_properties(FirstTest #SecondTest
	PROPERTIES TIMEOUT ${MULTIPLY_TIMEOUT})

#set_tests_properties(FirstTest PROPERTIES
#FAIL_REGULAR_EXPRESSION 0)
#PASS_REGULAR_EXPRESSION 0)

#set_tests_properties(SecondTest PROPERTIES
#PASS_REGULAR_EXPRESSION "The result is: 42390"
#FAIL_REGULAR_EXPRESSION "Error!")
