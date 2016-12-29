#### Global options ####

cmake_minimum_required(VERSION 2.8.8)
project(job_scheduler)

set (CMAKE_CXX_FLAGS "-g -Wall -Wextra -fopenmp -fPIC -std=c++11 -O2")

#### Dependencies ####

include_directories(include/)

#### Library definition ####

set(LIB_SOURCES
    include/workerfactory.hpp
    include/feeder.hpp
    include/queuescheduler.hpp
    include/job_scheduler.hpp  # TODO: Delete

    src/job_scheduler.cpp
)

set(JS_LIB_DIR "${CMAKE_SOURCE_DIR}/lib/")
message(STATUS "Generating js_conch library in: ${JS_LIB_DIR}")

add_library(js_conch SHARED ${LIB_SOURCES})
set_target_properties(js_conch PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${JS_LIB_DIR})

#### Sample program definition ####

set(MAIN_SOURCES
    utils.hpp

    main.cpp
)

link_directories(${JS_LIB_DIR})

add_executable (a.out ${MAIN_SOURCES})
add_dependencies(a.out js_conch)
target_link_libraries (a.out -ljs_conch)
