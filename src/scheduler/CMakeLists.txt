project(scheduler)

add_library(${PROJECT_NAME}
    include/${PROJECT_NAME}/circular_queue.h
    include/${PROJECT_NAME}/scheduler.h
    include/${PROJECT_NAME}/types.h
    src/circular_queue.cpp
    src/scheduler.cpp
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_link_libraries(${PROJECT_NAME} PUBLIC concurrency)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    timing_structures
    concurrency
    pthread
)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)

set_target_properties(scheduler
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "bin"
    LIBRARY_OUTPUT_DIRECTORY "lib"
    RUNTIME_OUTPUT_DIRECTORY "bin"
)