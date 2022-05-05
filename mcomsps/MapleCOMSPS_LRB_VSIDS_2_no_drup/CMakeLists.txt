cmake_minimum_required(VERSION 3.0)

# set the name of project and supporting languages 
project(maple_comsps CXX)

# for debugging
set(CMAKE_VERBOSE_MAKEFILE TRUE)

# specify the source files to build glucose library
set(MAPLE_COMSPS_LIB_SOURCES
    MapleCOMSPS_LRB_VSIDS_2/utils/Options.cc
    MapleCOMSPS_LRB_VSIDS_2/utils/System.cc
    MapleCOMSPS_LRB_VSIDS_2/core/Solver.cc
    MapleCOMSPS_LRB_VSIDS_2/simp/SimpSolver.cc)

# specify build targets
add_library(maple-comsps-lib-static STATIC ${MAPLE_COMSPS_LIB_SOURCES} m4ri-20200125/.libs/libm4ri.a)
add_library(maple-comsps-lib-shared SHARED ${MAPLE_COMSPS_LIB_SOURCES} m4ri-20200125/.libs/libm4ri.so)

set(MAPLE_COMSPS_VERSION 2017)
set_target_properties(maple-comsps-lib-static PROPERTIES OUTPUT_NAME "maple-comsps")
set_target_properties(maple-comsps-lib-shared PROPERTIES OUTPUT_NAME "maple-comsps" VERSION ${MAPLE_COMSPS_VERSION})

# need m4ri library
target_include_directories(maple-comsps-lib-static PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/m4ri-20200125)
target_include_directories(maple-comsps-lib-shared PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/m4ri-20200125)
add_custom_command(
  OUTPUT m4ri-20200125/.libs/libm4ri.a m4ri-20200125/.libs/libm4ri.so
  COMMAND tar xvzf ${CMAKE_CURRENT_SOURCE_DIR}/m4ri-20200125.tar.gz && cd m4ri-20200125 && ./configure --enable-thread-safe && make
)

# specify C++11/14/17
#target_compile_features(${DPS_NAME} PUBLIC cxx_std_11)  # avairable from cmake-3.8
set_property(TARGET maple-comsps-lib-static PROPERTY CXX_STANDARD 11)
set_property(TARGET maple-comsps-lib-shared PROPERTY CXX_STANDARD 11)
# compiling glucose requires following options
set(CMAKE_CXX_FLAGS "-Wall -Wno-parentheses  -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS")

# specify compile options for each build type
target_compile_options(maple-comsps-lib-static PUBLIC
  $<$<CONFIG:Release>:-O3>             # Release        (ex: cmake -DCMAKE_BUILD_TYPE=Release ..)
  $<$<CONFIG:Debug>:-O0 -g>            # Debug          (ex: cmake -DCMAKE_BUILD_TYPE=Debug ..)
  $<$<CONFIG:RelWithDebgInfo>:-O3 -g>  # RelWithDebInfo (ex: cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..)
  )
target_compile_options(maple-comsps-lib-shared PUBLIC
  $<$<CONFIG:Release>:-O3>             # Release        (ex: cmake -DCMAKE_BUILD_TYPE=Release ..)
  $<$<CONFIG:Debug>:-O0 -g>            # Debug          (ex: cmake -DCMAKE_BUILD_TYPE=Debug ..)
  $<$<CONFIG:RelWithDebgInfo>:-O3 -g>  # RelWithDebInfo (ex: cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..)
  )
