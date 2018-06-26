project(epb_plugin)

cmake_minimum_required(VERSION 2.8)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../scorep_plugin_common/;${CMAKE_MODULE_PATH}")

file(GLOB SUBMODULE_FILES "${CMAKE_SOURCE_DIR}/../scorep_plugin_common/*")
list(LENGTH SUBMODULE_FILES COUNT_SUBMODULE_FILES)

if(${COUNT_SUBMODULE_FILES} EQUAL 0)
    message(STATUS "Initializing git submodule")
    execute_process(COMMAND "git" "submodule" "init" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    execute_process(COMMAND "git" "submodule" "update" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()

find_package(X86Adapt REQUIRED)

find_path(TUNING_SUBSTRATE_PLUGIN_INC scorep/rrl_tuning_plugins.h ENV RRL_INC)

add_library(epb_plugin SHARED epb_plugin.c)
target_link_libraries(epb_plugin PRIVATE ${X86_ADAPT_LIBRARIES})
target_include_directories(epb_plugin PRIVATE ${X86_ADAPT_INCLUDE_DIRS} PRIVATE ${TUNING_SUBSTRATE_PLUGIN_INC})
target_compile_features(epb_plugin PUBLIC c_std_11)
target_compile_options(epb_plugin PRIVATE $<$<CONFIG:Debug>:-Wall -pedantic -Wextra -O3 -fno-omit-frame-pointer>)

install(TARGETS epb_plugin LIBRARY DESTINATION lib)
