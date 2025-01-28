#-------------------------------------------------------------------------------
# Zephyr Example Application
#
# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

get_filename_component(LOCAL_CONF "${CMAKE_CURRENT_SOURCE_DIR}/local.conf" ABSOLUTE)
if(EXISTS ${LOCAL_CONF})
    set(OVERLAY_CONFIG ${LOCAL_CONF})
    message(STATUS "Using LoRaWAN credentials from: ${LOCAL_CONF}")
else()
    message(WARNING "local.conf not found at: ${LOCAL_CONF}. Using default LoRaWAN credentials.")
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(buzznode)

list(APPEND CMAKE_MODULE_PATH ${ZEPHYR_BASE}/modules/nanopb)
include(nanopb)

# Find all proto files
file(GLOB_RECURSE proto_files protobufs/*.proto)
foreach(proto_file ${proto_files})
    zephyr_nanopb_sources(app ${proto_file})
endforeach()

file(GLOB_RECURSE app_sources src/*.cpp)
target_sources(app PRIVATE ${app_sources})

# Set C++17 using compiler flags instead of target_compile_features
set_target_properties(app PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_link_libraries(app PRIVATE etl::etl)