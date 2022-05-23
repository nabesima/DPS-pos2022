cmake_minimum_required(VERSION 3.13)

# set the name of project and supporting languages 
project(DPS CXX)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)

# specify version
set(DPS_MAJOR   1)
set(DPS_MINER   4)
set(DPS_PATCH   0)
# make binary name
set(DPS_NAME DPS-${DPS_MAJOR}.${DPS_MINER}.${DPS_PATCH})
add_definitions(-DDPS_MAJOR=${DPS_MAJOR})
add_definitions(-DDPS_MINER=${DPS_MINER})
add_definitions(-DDPS_PATCH=${DPS_PATCH})
add_definitions(-DDPS_BUILD_TYPE=\"${CMAKE_BUILD_TYPE}\")

# specify the source files to build DPS
set(DPS_SOURCES
    DPS-src/Main.cpp
    DPS-src/period/PrdClausesQueueMgr.cpp
    DPS-src/period/PrdClausesQueue.cpp
    DPS-src/period/PrdClauses.cpp
    DPS-src/period/ClauseBuffer.cpp
    DPS-src/sat/Clause.cpp
    DPS-src/sat/Instance.cpp
    DPS-src/sat/Model.cpp
    DPS-src/solvers/AbstDetSeqSolver.cpp
    DPS-src/solvers/ThreadLocalVars.cpp
    DPS-src/solvers/DPS_C_API.cpp
    DPS-src/solvers/SolverFactory.cpp
    DPS-src/solvers/MiniSatWrapper.cpp
    DPS-src/solvers/GlucoseWrapper.cpp
    DPS-src/solvers/MapleCOMSPSWrapper.cpp
    DPS-src/solvers/KissatWrapper.cpp
    DPS-src/utils/System.cpp
    DPS-src/utils/p2.cc
    DPS-src/utils/Chronometer.cpp    
    DPS-src/parallel/Sharer.cpp
    DPS-src/parallel/Options.cpp
    DPS-src/parallel/Version.cpp
    DPS-src/parallel/DetParallelSolver.cpp
)

# STDC_FORMAT_MACROS is required for minisat headers
set(CMAKE_CXX_FLAGS "-Wall -Wno-parentheses -D __STDC_FORMAT_MACROS")

# specify sub directories
add_subdirectory(minisat/minisat-2.2.0)
add_subdirectory(glucose/glucose-3.0)
add_subdirectory(mcomsps/MapleCOMSPS_LRB_VSIDS_2_no_drup)
add_subdirectory(kissat)

add_executable(${DPS_NAME} ${DPS_SOURCES})

# specify link libraries
target_link_libraries(${DPS_NAME} pthread)
target_link_libraries(${DPS_NAME} z)
target_link_libraries(${DPS_NAME} minisat-lib-static)
target_link_libraries(${DPS_NAME} glucose-lib-static)
target_link_libraries(${DPS_NAME} maple-comsps-lib-static)
target_link_directories(${DPS_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/mcomsps/MapleCOMSPS_LRB_VSIDS_2_no_drup/m4ri-20200125/.libs)
target_link_libraries(${DPS_NAME} m4ri)
add_dependencies(${DPS_NAME} kissat-sc2021-lib-static)
target_link_libraries(${DPS_NAME} ${CMAKE_CURRENT_BINARY_DIR}/kissat/libkissat-sc2021.a)

# specify C++11/14/17
#target_compile_features(${DPS_NAME} PUBLIC cxx_std_11)  # avairable from cmake-3.8
set_property(TARGET ${DPS_NAME} PROPERTY CXX_STANDARD 11)
# specify compile options for each build type
target_compile_options(${DPS_NAME} PUBLIC
  $<$<CONFIG:Release>:-O3>             # Release        (ex: cmake -DCMAKE_BUILD_TYPE=Release ..)
  $<$<CONFIG:Debug>:-O0 -g>            # Debug          (ex: cmake -DCMAKE_BUILD_TYPE=Debug ..)
  $<$<CONFIG:RelWithDebgInfo>:-O3 -g>  # RelWithDebInfo (ex: cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..)
  )
  
