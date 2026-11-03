cmake_minimum_required(VERSION 3.10.0)
project(project4 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/W3)
    add_compile_options(/EHsc)
    add_compile_options(/O2)
    add_compile_options(/utf-8)
else()
    add_compile_options(-Wall)
    add_compile_options(-O3)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

add_executable(generator generator.cpp)

add_executable(bench main.cpp)

add_executable(bench_alt main.cpp)
target_compile_definitions(bench_alt PRIVATE ALT_TWO_LOCKS)
