cmake_minimum_required(VERSION 3.12)

project(Simple-Scene)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE Aegix-Engine)
