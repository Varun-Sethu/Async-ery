project(io)

add_library(${PROJECT_NAME}
    include/${PROJECT_NAME}/aio.h
    include/${PROJECT_NAME}/io_poll_source.h
    include/${PROJECT_NAME}/io_request.h
    src/aio.cpp
    src/io_poll_source.cpp
    src/io_request.cpp
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    concurrency
    scheduler
    pthread
    rt
)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "bin"
    LIBRARY_OUTPUT_DIRECTORY "lib"
    RUNTIME_OUTPUT_DIRECTORY "bin"
)