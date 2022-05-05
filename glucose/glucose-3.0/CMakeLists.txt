cmake_minimum_required(VERSION 3.0)

# set the name of project and supporting languages 
project(glucose CXX)

# for debugging
set(CMAKE_VERBOSE_MAKEFILE TRUE)

# specify the source files to build glucose library
set(GLUCOSE_LIB_SOURCES
    utils/Options.cc
    utils/System.cc
    core/Solver.cc
    simp/SimpSolver.cc)

# specify build targets
add_library(glucose-lib-static STATIC ${GLUCOSE_LIB_SOURCES})
add_library(glucose-lib-shared SHARED ${GLUCOSE_LIB_SOURCES})
    
set(GLUCOSE_VERSION 3.0)
set_target_properties(glucose-lib-static PROPERTIES OUTPUT_NAME "glucose")
set_target_properties(glucose-lib-shared PROPERTIES OUTPUT_NAME "glucose" VERSION ${GLUCOSE_VERSION})

# specify C++11/14/17
#target_compile_features(${DPS_NAME} PUBLIC cxx_std_11)  # avairable from cmake-3.8
set_property(TARGET glucose-lib-static PROPERTY CXX_STANDARD 11)
set_property(TARGET glucose-lib-shared PROPERTY CXX_STANDARD 11)
# compiling glucose requires following options
set(CMAKE_CXX_FLAGS "-Wall -Wno-parentheses  -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS")

# specify compile options for each build type
target_compile_options(glucose-lib-static PUBLIC
  $<$<CONFIG:Release>:-O3>             # Release        (ex: cmake -DCMAKE_BUILD_TYPE=Release ..)
  $<$<CONFIG:Debug>:-O0 -g>            # Debug          (ex: cmake -DCMAKE_BUILD_TYPE=Debug ..)
  $<$<CONFIG:RelWithDebgInfo>:-O3 -g>  # RelWithDebInfo (ex: cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..)
  )
target_compile_options(glucose-lib-shared PUBLIC
  $<$<CONFIG:Release>:-O3>             # Release        (ex: cmake -DCMAKE_BUILD_TYPE=Release ..)
  $<$<CONFIG:Debug>:-O0 -g>            # Debug          (ex: cmake -DCMAKE_BUILD_TYPE=Debug ..)
  $<$<CONFIG:RelWithDebgInfo>:-O3 -g>  # RelWithDebInfo (ex: cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..)
  )
