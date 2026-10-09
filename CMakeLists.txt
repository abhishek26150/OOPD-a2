# Name: Abhishek Kumar Singh
# Roll Number: MT26150

cmake_minimum_required(VERSION 3.12)
project(BookManagement CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_library(bookmgmt ${SOURCES})

add_executable(bookmgmt_tests tests/test_main.cpp)
target_link_libraries(bookmgmt_tests PRIVATE bookmgmt)

add_executable(demo examples/demo.cpp)
target_link_libraries(demo PRIVATE bookmgmt)