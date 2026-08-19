cmake_minimum_required(VERSION 3.16)

project(Lab01 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(lab01
    src/main.cpp
)

target_include_directories(lab01 PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(lab01 PRIVATE ${OpenCV_LIBS})