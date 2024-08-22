project(timing)

add_library(timing
    include/${PROJECT_NAME}/timing_poll_source.h
    src/timing_poll_source.cpp
)

add_library(timing_structures
    include/${PROJECT_NAME}/structures/timing_wheel.h
    include/${PROJECT_NAME}/structures/timing_wheel_hierarchical.h
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET timing_structures PROPERTY CXX_STANDARD 20)
set_property(TARGET timing_structures PROPERTY LINKER_LANGUAGE CXX)

target_link_libraries(${PROJECT_NAME} PRIVATE scheduler_intf concurrency)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(timing_structures PUBLIC include)

target_include_directories(${PROJECT_NAME} PRIVATE src)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "bin"
    LIBRARY_OUTPUT_DIRECTORY "lib"
    RUNTIME_OUTPUT_DIRECTORY "bin"
)

set_target_properties(timing_structures
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "bin"
    LIBRARY_OUTPUT_DIRECTORY "lib"
    RUNTIME_OUTPUT_DIRECTORY "bin"
)