cmake_minimum_required(VERSION 3.5)

project(serpent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

add_subdirectory(tables)
add_subdirectory(test/basic-usage)
add_subdirectory(test/debug)

add_library(${PROJECT_NAME} STATIC
  src/serpent.cpp
)
target_link_libraries(${PROJECT_NAME} serpent_tables)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
