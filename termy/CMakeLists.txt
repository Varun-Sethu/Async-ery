cmake_minimum_required(VERSION 3.5)

project(termy)

add_executable(termy main.cpp)

target_link_libraries(termy PRIVATE async_lib)

target_include_directories(termy PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

set_property(TARGET termy PROPERTY CXX_STANDARD 20)
