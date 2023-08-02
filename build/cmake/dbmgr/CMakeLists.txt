# CMakeLists.txt

# Copyright (c) Mateusz Jandura. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)
project(dbmgr
    VERSION 1.0.1
    DESCRIPTION "App Locker Database Manager"
    LANGUAGES CXX
)

set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

if(NOT(CMAKE_GENERATOR_PLATFORM STREQUAL x64 OR CMAKE_GENERATOR_PLATFORM STREQUAL Win32))
    message(FATAL_ERROR "Requires either x64 or Win32 platform architecture.")
endif()

set(CMAKE_SUPPRESS_REGENERATION TRUE)
if(MSVC)
    set(VS_SOURCE_GROUPS src)
endif()

set(src_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../../src")
set(dbmgr_sources
    "${src_dir}/dbmgr/checksum.cpp"
    "${src_dir}/dbmgr/checksum.hpp"
    "${src_dir}/dbmgr/database.cpp"
    "${src_dir}/dbmgr/database.hpp"
    "${src_dir}/dbmgr/main.cpp"
    "${src_dir}/dbmgr/task.cpp"
    "${src_dir}/dbmgr/task.hpp"
)

# put all source files in "src" directory
source_group("src" FILES ${dbmgr_sources})

# put the compiled executable in either the "bin\Debug" or "bin\Release" directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")

add_executable(dbmgr
    ${dbmgr_sources}
)

target_compile_features(dbmgr PRIVATE cxx_std_17)
target_include_directories(dbmgr PRIVATE "${src_dir}")