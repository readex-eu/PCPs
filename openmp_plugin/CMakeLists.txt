project(OpenMPTP)

cmake_minimum_required(VERSION 3.5)

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


link_directories(${CMAKE_SOURCE_DIR})
add_library(OpenMPTP SHARED OpenMPTP.c)
target_compile_definitions(OpenMPTP PRIVATE GIT_REV="${GIT_REV}")
target_include_directories(OpenMPTP PRIVATE ${TUNING_SUBSTRATE_PLUGIN_INC})
target_compile_features(OpenMPTP PUBLIC c_std_11)
target_compile_options(OpenMPTP PRIVATE $<$<CONFIG:Debug>:-Wall -O3 -fno-omit-frame-pointer>)

install(TARGETS OpenMPTP LIBRARY DESTINATION lib/)

