cmake_minimum_required(VERSION 2.8)

project(network_programming)

set(CMAKE_C_FLAGS "-g")

file(GLOB SRCs "*.c")
file(GLOB HDRs "*.h")

foreach(SRC ${SRCs})
    message(STATUS "Processing ${SRC} ...")
    get_filename_component(BIN ${SRC} NAME_WE)
    add_executable(${BIN} ${SRC} ${HDR})
endforeach()


