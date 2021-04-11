cmake_minimum_required(VERSION 3.0)
project(MyProgram)
add_executable(Multiply Multiply.c)

set(MULTIPLY_TIMEOUT 1)

# Включить поддержку тестирования:
enable_testing()

# Добавить тесты:
add_test(FirstTest Multiply 15 207)
add_test(SecondTest Multiply -54 -785)
add_test(ThirdTest Multiply 85234)

# Установить поведение тестов:
set_tests_properties(FirstTest SecondTest ThirdTest
PROPERTIES TIMEOUT ${MULTIPLY_TIMEOUT})

set_tests_properties(FirstTest PROPERTIES
PASS_REGULAR_EXPRESSION "The result is: 3105"
FAIL_REGULAR_EXPRESSION "Error!")

set_tests_properties(SecondTest PROPERTIES
PASS_REGULAR_EXPRESSION "The result is: 42390"
FAIL_REGULAR_EXPRESSION "Error!")

set_tests_properties(ThirdTest PROPERTIES
PASS_REGULAR_EXPRESSION "Error!")