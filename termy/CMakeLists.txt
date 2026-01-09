cmake_minimum_required(VERSION 3.5)

project(termy)

add_executable(termy
    main.cpp
    src/frame.cpp
    src/menu.cpp
    src/keyboard_poll_source.cpp
    src/window.cpp
)

target_link_libraries(termy PRIVATE async_lib fmt::fmt)

target_include_directories(termy PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_property(TARGET termy PROPERTY CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
    fmt
    URL https://github.com/fmtlib/fmt/archive/refs/tags/10.2.1.zip
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

target_compile_options(gtest PRIVATE -Wno-error)
target_compile_options(gtest_main PRIVATE -Wno-error)

enable_testing()

add_executable(termy_tests
    test/test_main.cpp
    test/menu_test.cpp
    test/window_test.cpp
    src/frame.cpp
    src/menu.cpp
    src/window.cpp
)

target_include_directories(termy_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/test
)

target_link_libraries(termy_tests PRIVATE
    async_lib
    fmt::fmt
    GTest::gtest_main
)

set_property(TARGET termy_tests PROPERTY CXX_STANDARD 20)

include(GoogleTest)
gtest_discover_tests(termy_tests)
