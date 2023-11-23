cmake_minimum_required(VERSION 3.5)

project(serpent_tables)

add_library(${PROJECT_NAME} STATIC
  src/serpent_tables.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
)
