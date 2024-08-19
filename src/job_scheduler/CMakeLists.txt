project(job_scheduler)

# Interface target
add_library(${PROJECT_NAME}_intf
    include/${PROJECT_NAME}/interface/job_scheduler_intf.h
    include/${PROJECT_NAME}/interface/poll_source.h
    include/${PROJECT_NAME}/interface/scheduling_context.h
    include/${PROJECT_NAME}/interface/job.h
)

set_property(TARGET ${PROJECT_NAME}_intf PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME}_intf PROPERTY LINKER_LANGUAGE CXX)

target_include_directories(${PROJECT_NAME}_intf INTERFACE include)


# Actual target
add_library(${PROJECT_NAME}
    include/${PROJECT_NAME}/circular_queue.h
    include/${PROJECT_NAME}/job_scheduler.h
    src/job_scheduler.cpp
    src/circular_queue.cpp
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_link_libraries(${PROJECT_NAME} PUBLIC concurrency)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    timing_structures
    concurrency
    pthread
    ${PROJECT_NAME}_intf
)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "bin"
    LIBRARY_OUTPUT_DIRECTORY "lib"
    RUNTIME_OUTPUT_DIRECTORY "bin"
)


