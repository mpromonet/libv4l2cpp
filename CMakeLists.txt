cmake_minimum_required(VERSION 3.0)

# set project name from current directory
get_filename_component(BASENAME ${CMAKE_CURRENT_LIST_DIR} NAME)
project(${BASENAME})

set (CMAKE_CXX_STANDARD 11)

aux_source_directory(src SRC_FILES)
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/inc")

add_executable (${PROJECT_NAME}test main.cpp)
target_link_libraries (${PROJECT_NAME}test ${PROJECT_NAME}) 
