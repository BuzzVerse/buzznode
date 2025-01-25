#-------------------------------------------------------------------------------
# Zephyr Example Application
#
# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES CXX)
file(GLOB_RECURSE app_sources src/*.cpp)
target_sources(app PRIVATE ${app_sources})

# Set C++17 using compiler flags instead of target_compile_features
set_target_properties(app PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_link_libraries(app PRIVATE etl::etl)