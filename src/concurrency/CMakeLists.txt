project(concurrency)

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
                    -Wzero-as-null-pointer-constant)

add_library(${PROJECT_NAME}
    include/${PROJECT_NAME}/spinlock.h
    src/spinlock.cpp
)


set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "bin"
    LIBRARY_OUTPUT_DIRECTORY "lib"
    RUNTIME_OUTPUT_DIRECTORY "bin"
)