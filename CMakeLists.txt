cmake_minimum_required(VERSION 3.25)
project(raycaster LANGUAGES C)

add_executable(raycaster main.c)

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)

target_link_libraries(raycaster PRIVATE SDL2::SDL2 m)
