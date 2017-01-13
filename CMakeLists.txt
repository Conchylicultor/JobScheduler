#### Global options ####

cmake_minimum_required(VERSION 2.8.8)
project(job_scheduler)

set (CMAKE_CXX_FLAGS "-g -Wall -Wextra -fopenmp -fPIC -std=c++11 -O2")

#### Dependencies ####

include_directories(include/)

# #### Library definition ####
#
# set(LIB_SOURCES
#     include/workerbase.hpp
#     include/workerfactory.hpp
#     include/queuethread.hpp
#     include/queuescheduler.hpp
#
#     include/job_scheduler.hpp  # Wrapper around all headers
#
#     # Template based so no .cpp files
# )
#
# set(JS_LIB_DIR "${CMAKE_SOURCE_DIR}/lib/")
# message(STATUS "Generating job_scheduler library in: ${JS_LIB_DIR}")
#
# add_library(job_scheduler SHARED ${LIB_SOURCES})
# set_target_properties(job_scheduler PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${JS_LIB_DIR})

#### Sample program definition ####

set(MAIN_SOURCES
    main_utils.hpp

    main.cpp
)

link_directories(${JS_LIB_DIR})

add_executable (a.out ${MAIN_SOURCES})
#add_dependencies(a.out job_scheduler)
#target_link_libraries (a.out -ljob_scheduler)
