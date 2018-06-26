project(mpit_plugin)

cmake_minimum_required(VERSION 2.8)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../scorep_plugin_common/;${CMAKE_MODULE_PATH}")

find_path(TUNING_SUBSTRATE_PLUGIN_INC scorep/rrl_tuning_plugins.h ENV RRL_INC)
#find_path(MPI_INCLUDE mpi.h HINTS /usr/include/mpi/ ENV MPI_INC CMAKE_INCLUDE_PATH CPATH)
find_package(MPI)
include_directories(${MPI_CXX_INCLUDE_PATH} ${MPI_C_INCLUDE_PATH})
#include_directories(${MPI_INCLUDE})

include_directories(${TUNING_SUBSTRATE_PLUGIN_INC})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILE_FLAGS} -std=c++11 -Wall -pedantic -g -O3 -fno-omit-frame-pointer")


include_directories("${PROJECT_SOURCE_DIR}/include")

set(SRC
    ${CMAKE_SOURCE_DIR}/mpit_plugin.cpp
    ${CMAKE_SOURCE_DIR}/mpit_interface.cpp
    )

add_library(mpit_plugin SHARED ${SRC})

install(TARGETS mpit_plugin LIBRARY DESTINATION lib)
