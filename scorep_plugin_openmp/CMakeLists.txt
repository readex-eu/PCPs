project(OpenMPTP)

cmake_minimum_required(VERSION 3.5)

find_path(TUNING_SUBSTRATE_PLUGIN_INC scorep/rrl_tuning_plugins.h ENV RRL_INC)

link_directories(${CMAKE_SOURCE_DIR})
add_library(OpenMPTP SHARED OpenMPTP.c)
target_include_directories(OpenMPTP PRIVATE ${TUNING_SUBSTRATE_PLUGIN_INC})
target_compile_features(OpenMPTP PUBLIC c_std_11)
target_compile_options(OpenMPTP PRIVATE $<$<CONFIG:Debug>:-Wall -O3 -fno-omit-frame-pointer>)

install(TARGETS OpenMPTP LIBRARY DESTINATION lib/)

