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

# Nanopb setup
list(APPEND CMAKE_MODULE_PATH ${ZEPHYR_BASE}/modules/nanopb)
include(nanopb)

# Set proto path for imports
set(PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/protobufs)

# Generate protobuf files
file(GLOB_RECURSE proto_files protobufs/buzzverse/*.proto)
foreach(proto_file ${proto_files})
    get_filename_component(proto_name ${proto_file} NAME_WE)
    nanopb_generate_cpp(PROTO_SRCS PROTO_HDRS ${proto_file}
        RELPATH ${PROTO_PATH}
    )
    list(APPEND proto_sources ${PROTO_SRCS})
endforeach()

# Create a separate library for proto files
zephyr_library_named(proto)
target_sources(proto PRIVATE ${proto_sources})
target_include_directories(proto PUBLIC 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/protobufs
)

# Add application sources
file(GLOB_RECURSE app_sources src/*.cpp)
target_sources(app PRIVATE ${app_sources})

# Set C++17 using compiler flags instead of target_compile_features
set_target_properties(app PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_link_libraries(app PRIVATE proto etl::etl)