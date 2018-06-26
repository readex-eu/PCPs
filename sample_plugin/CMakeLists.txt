project(example)

cmake_minimum_required(VERSION 3.5)

find_path(TUNING_SUBSTRATE_PLUGIN_INC scorep/rrl_tuning_plugins.h ENV RRL_INC)

add_library(example SHARED example.c)
target_include_directories(example PRIVATE ${TUNING_SUBSTRATE_PLUGIN_INC})
target_compile_features(example PUBLIC c_std_11)
target_compile_options(example PRIVATE $<$<CONFIG:Debug>:-Wall -pedantic -Wextra -O3 -fno-omit-frame-pointer>)

install(TARGETS example LIBRARY DESTINATION lib)