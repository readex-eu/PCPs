project(uncore_freq_plugin)

cmake_minimum_required(VERSION 3.5)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../scorep_plugin_common/;${CMAKE_MODULE_PATH}")

file(GLOB SUBMODULE_FILES "${CMAKE_SOURCE_DIR}/../scorep_plugin_common/*")
list(LENGTH SUBMODULE_FILES COUNT_SUBMODULE_FILES)

if(${COUNT_SUBMODULE_FILES} EQUAL 0)
    message(STATUS "Initializing git submodule")
    execute_process(COMMAND "git" "submodule" "init" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    execute_process(COMMAND "git" "submodule" "update" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()

find_path(TUNING_SUBSTRATE_PLUGIN_INC scorep/rrl_tuning_plugins.h ENV RRL_INC)

execute_process(
  COMMAND git rev-parse HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_REV
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE error
  ERROR_VARIABLE error_msg
)
if (NOT ${error} EQUAL 0)
    message(STATUS "can't retrive git hash, set to 0")
    set(GIT_REV "0")
endif()


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../extern/libfreqgen ${CMAKE_BINARY_DIR}/libfreqgen)

add_library(uncore_freq_plugin SHARED uncore_freq_plugin.c)
target_compile_definitions(uncore_freq_plugin PRIVATE GIT_REV="${GIT_REV}")
target_link_libraries(uncore_freq_plugin PRIVATE freqgen)
target_include_directories(uncore_freq_plugin PRIVATE ${TUNING_SUBSTRATE_PLUGIN_INC} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../extern/libfreqgen/include)
target_compile_features(uncore_freq_plugin PUBLIC c_std_11)
target_compile_options(uncore_freq_plugin PRIVATE $<$<CONFIG:Debug>:-Wall -pedantic -Wextra -O3 -fno-omit-frame-pointer>)

install(TARGETS uncore_freq_plugin LIBRARY DESTINATION lib)








