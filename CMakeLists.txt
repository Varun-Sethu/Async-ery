cmake_minimum_required(VERSION 3.5)

project(async_lib)
add_subdirectory(src)

add_library(${PROJECT_NAME}
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

set(CMAKE_BUILD_TYPE Debug)
set(EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_CLANG_TIDY 
        clang-tidy;
            -header-filter=.*;
            -checks=*,-abseil-*,-altera-*,-android-*,-fuchsia-*,-google-*,-llvm*,readability-*,clang-analyzer-*,-clang-analyzer-optin.performance.Padding,-misc-no-recursion;
            -warnings-as-errors=*)

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


target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src/${PROJECT_NAME})


# ==== For testing ====
add_executable(io_test tests/io_test.cpp)
add_executable(timing_test tests/timing_test.cpp)
add_executable(main tests/main.cpp)
add_executable(stress_test tests/stress.cpp)

set_property(TARGET io_test PROPERTY CXX_STANDARD 20)
set_property(TARGET timing_test PROPERTY CXX_STANDARD 20)
set_property(TARGET main PROPERTY CXX_STANDARD 20)
set_property(TARGET stress_test PROPERTY CXX_STANDARD 20)



target_link_libraries(io_test PRIVATE async_lib)
target_link_libraries(timing_test PRIVATE timing_structures)
target_link_libraries(main PRIVATE async_lib)
target_link_libraries(stress_test PRIVATE async_lib)
