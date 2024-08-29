cmake_minimum_required(VERSION 3.5)

project(async_lib)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
set(CMAKE_CXX_CLANG_TIDY clang-tidy-17 "-config-file=${CMAKE_SOURCE_DIR}/.clang-tidy")
add_compile_options(-Wall 
                    -Wextra
                    -pedantic-errors
                    -Wnon-virtual-dtor
                    -Werror
                    -Wconversion
                    -Wdouble-promotion
                    -Wnull-dereference
                    -Wcast-align
                    -Wuninitialized
                    -Wsuggest-override
                    -Wzero-as-null-pointer-constant
                    -fstack-protector-strong)
                    
add_subdirectory(src)
add_library(${PROJECT_NAME}
    include/${PROJECT_NAME}/async_result.h
    include/${PROJECT_NAME}/task_factory.h
    include/${PROJECT_NAME}/task_io_source.h
    include/${PROJECT_NAME}/task_timer_source.h
    include/${PROJECT_NAME}/task_value_source.h
    include/${PROJECT_NAME}/task.h
    include/${PROJECT_NAME}/types.h
    src/${PROJECT_NAME}/task_timer_source.cpp
    src/${PROJECT_NAME}/task_io_source.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC scheduler_intf cell io timing timing_structures concurrency)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY LINKER_LANGUAGE CXX)


target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src/${PROJECT_NAME})


# ==== For testing ====
add_executable(io_example examples/io_example.cpp)
add_executable(main_example examples/main_example.cpp)
add_executable(concurrency_example examples/concurrency.cpp)
add_executable(error_example examples/error_example.cpp)

set_property(TARGET io_example PROPERTY CXX_STANDARD 23)
set_property(TARGET main_example PROPERTY CXX_STANDARD 23)
set_property(TARGET concurrency_example PROPERTY CXX_STANDARD 23)
set_property(TARGET error_example PROPERTY CXX_STANDARD 23)


target_link_libraries(io_example PRIVATE async_lib)
target_link_libraries(main_example PRIVATE async_lib)
target_link_libraries(concurrency_example PRIVATE async_lib)
target_link_libraries(error_example PRIVATE async_lib)
