project(cell)

add_library(${PROJECT_NAME}
    include/${PROJECT_NAME}/cell.h
    include/${PROJECT_NAME}/tracking_once_cell.h
    include/${PROJECT_NAME}/when_all_cell.h
    include/${PROJECT_NAME}/when_any_cell.h
    include/${PROJECT_NAME}/write_once_cell.h
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY LINKER_LANGUAGE CXX)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PRIVATE scheduler)