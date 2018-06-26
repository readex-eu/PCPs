project(cpu_freq_plugin)

cmake_minimum_required(VERSION 3.5)
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../scorep_plugin_common/;${CMAKE_MODULE_PATH}")
find_path(TUNING_SUBSTRATE_PLUGIN_INC scorep/rrl_tuning_plugins.h ENV RRL_INC)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../extern/libfreqgen ${CMAKE_BINARY_DIR}/libfreqgen)

add_library(cpu_freq_plugin SHARED cpu_freq_plugin.c)

target_link_libraries(cpu_freq_plugin PRIVATE freqgen)
target_include_directories(cpu_freq_plugin PRIVATE ${TUNING_SUBSTRATE_PLUGIN_INC} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../extern/libfreqgen/include)
target_compile_features(cpu_freq_plugin PUBLIC c_std_11)
target_compile_options(cpu_freq_plugin PRIVATE $<$<CONFIG:Debug>:-Wall -pedantic -Wextra -O3 -fno-omit-frame-pointer>)

install(TARGETS cpu_freq_plugin LIBRARY DESTINATION lib)
