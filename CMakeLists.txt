cmake_minimum_required(VERSION 3.6.0)

project(mg_ray)      

	include_directories(${CMAKE_SOURCE_DIR}/src)
    #adding the executable
    add_executable(${PROJECT_NAME} ../src/main.cpp)
