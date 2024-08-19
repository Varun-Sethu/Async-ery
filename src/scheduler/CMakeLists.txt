project(scheduler)

# Actual target
add_library(${PROJECT_NAME}
    include/${PROJECT_NAME}/job_queue.h
    include/${PROJECT_NAME}/scheduler.h
    include/${PROJECT_NAME}/worker.h
    include/${PROJECT_NAME}/worker_pool.h
    src/scheduler.cpp
    src/job_queue.cpp
    src/worker.cpp
    src/worker_pool.cpp
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_link_libraries(${PROJECT_NAME} PUBLIC concurrency)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    timing_structures
    concurrency
    pthread
)

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PUBLIC include/interface)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "bin"
    LIBRARY_OUTPUT_DIRECTORY "lib"
    RUNTIME_OUTPUT_DIRECTORY "bin"
)



# Interface target
add_library(${PROJECT_NAME}_intf
    include/interface/${PROJECT_NAME}/scheduler_intf.h
    include/interface/${PROJECT_NAME}/poll_source.h
    include/interface/${PROJECT_NAME}/scheduling_context.h
    include/interface/${PROJECT_NAME}/job.h
    include/interface/${PROJECT_NAME}/scheduler_factory.h
    src/scheduler_factory.cpp
)

set_property(TARGET ${PROJECT_NAME}_intf PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME}_intf PROPERTY LINKER_LANGUAGE CXX)
target_link_libraries(${PROJECT_NAME}_intf PRIVATE ${PROJECT_NAME})


target_include_directories(${PROJECT_NAME}_intf INTERFACE include/interface)
target_include_directories(${PROJECT_NAME}_intf PRIVATE include)