# CMakeLists.txt

# Copyright (c) Mateusz Jandura. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)
project(applocker
    VERSION 1.0.1
    DESCRIPTION "App Locker Service"
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
set(applocker_sources
    "${src_dir}/applocker/directory_watcher.cpp"
    "${src_dir}/applocker/directory_watcher.hpp"
    "${src_dir}/applocker/event_sink.cpp"
    "${src_dir}/applocker/event_sink.hpp"
    "${src_dir}/applocker/main.cpp"
    "${src_dir}/applocker/process.cpp"
    "${src_dir}/applocker/process.hpp"
    "${src_dir}/applocker/service.cpp"
    "${src_dir}/applocker/service.hpp"
    "${src_dir}/applocker/service_caches.cpp"
    "${src_dir}/applocker/service_caches.hpp"
    "${src_dir}/applocker/sync.cpp"
    "${src_dir}/applocker/sync.hpp"
    "${src_dir}/applocker/waitable_event.cpp"
    "${src_dir}/applocker/waitable_event.hpp"
    "${src_dir}/applocker/wmi.cpp"
    "${src_dir}/applocker/wmi.hpp"
)
set(dbmgr_sources
    "${src_dir}/dbmgr/checksum.cpp"
    "${src_dir}/dbmgr/checksum.hpp"
    "${src_dir}/dbmgr/database.cpp"
    "${src_dir}/dbmgr/database.hpp"
)

# put all source files in "src" directory
source_group("src" FILES ${applocker_sources} ${dbmgr_sources})

# put the compiled executable in either the "bin\Debug" or "bin\Release" directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")

add_executable(applocker
    ${applocker_sources}
    ${dbmgr_sources}
)

target_compile_features(applocker PRIVATE cxx_std_17)
target_include_directories(applocker PRIVATE "${src_dir}")
target_link_libraries(applocker PRIVATE wbemuuid.lib)