cmake_minimum_required(VERSION 3.0)

# set the name of project and supporting languages 
project(kissat C)

# for debugging
set(CMAKE_VERBOSE_MAKEFILE TRUE)

# Include External Library
include(ExternalProject)
ExternalProject_Add(
    kissat-sc2021-lib-static         # Name for custom target
    SOURCE_DIR     
        ${CMAKE_CURRENT_SOURCE_DIR}/kissat-sc2021
    BINARY_DIR     
        ${CMAKE_CURRENT_SOURCE_DIR}/kissat-sc2021
    CONFIGURE_COMMAND 
        ./configure --no-proofs --no-checks #-g
    BUILD_COMMAND
        cd ./build && make libkissat.a
    INSTALL_COMMAND 
        cp ./build/libkissat.a ${CMAKE_CURRENT_BINARY_DIR}/libkissat-sc2021.a
    BUILD_BYPRODUCTS 
        ${CMAKE_CURRENT_BINARY_DIR}/libkissat-sc2021.a       
)

# file(GLOB KISSAT_LIB_SOURCES RELATIVE "${CMAKE_SOURCE_DIR}" "src/*.c")
# list(REMOVE_ITEM KISSAT_LIB_SOURCES src/main.c)
# message("${KISSAT_LIB_SOURCES}")

# add_custom_target(kissat-sc2021-lib-static
#     WORKING_DIRECTORY 
#         ${CMAKE_CURRENT_SOURCE_DIR}/kissat-sc2021
#     COMMAND ./configure
#     COMMAND make
#     COMMAND cp build/libkissat.a ${CMAKE_CURRENT_BINARY_DIR}/libkissat-sc2021.a
#     SOURCES ${KISSAT_LIB_SOURCES}
# )
