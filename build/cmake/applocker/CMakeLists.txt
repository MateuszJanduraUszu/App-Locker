# CMakeLists.txt

# Copyright (c) Mateusz Jandura. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)
project(applocker
    VERSION 1.0.3
    DESCRIPTION "App Locker Service"
    LANGUAGES CXX
)

set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

# translate x64/Win32 into x64/x86
if(CMAKE_GENERATOR_PLATFORM STREQUAL x64)
    set(APPLOCKER_PLATFORM_ARCH x64)
elseif(CMAKE_GENERATOR_PLATFORM STREQUAL Win32)
    set(APPLOCKER_PLATFORM_ARCH x86)
else()
    set(APPLOCKER_PLATFORM_ARCH Invalid)
    message(FATAL_ERROR "Requires either x64 or Win32 platform architecture.")
endif()

set(CMAKE_SUPPRESS_REGENERATION TRUE)
if(MSVC)
    set(VS_SOURCE_GROUPS src)
endif()

set(APPLOCKER_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../src")
set(APPLOCKER_SOURCES
    "${APPLOCKER_SRC_DIR}/applocker/directory_watcher.cpp"
    "${APPLOCKER_SRC_DIR}/applocker/directory_watcher.hpp"
    "${APPLOCKER_SRC_DIR}/applocker/event_sink.cpp"
    "${APPLOCKER_SRC_DIR}/applocker/event_sink.hpp"
    "${APPLOCKER_SRC_DIR}/applocker/main.cpp"
    "${APPLOCKER_SRC_DIR}/applocker/process.cpp"
    "${APPLOCKER_SRC_DIR}/applocker/process.hpp"
    "${APPLOCKER_SRC_DIR}/applocker/service.cpp"
    "${APPLOCKER_SRC_DIR}/applocker/service.hpp"
    "${APPLOCKER_SRC_DIR}/applocker/service_caches.cpp"
    "${APPLOCKER_SRC_DIR}/applocker/service_caches.hpp"
    "${APPLOCKER_SRC_DIR}/applocker/sync.cpp"
    "${APPLOCKER_SRC_DIR}/applocker/sync.hpp"
    "${APPLOCKER_SRC_DIR}/applocker/wmi.cpp"
    "${APPLOCKER_SRC_DIR}/applocker/wmi.hpp"
)
set(DBMGR_SOURCES
    "${APPLOCKER_SRC_DIR}/dbmgr/checksum.cpp"
    "${APPLOCKER_SRC_DIR}/dbmgr/checksum.hpp"
    "${APPLOCKER_SRC_DIR}/dbmgr/database.cpp"
    "${APPLOCKER_SRC_DIR}/dbmgr/database.hpp"
    "${APPLOCKER_SRC_DIR}/dbmgr/tinywin.hpp"
)

# put all source files in "src" directory
source_group("src" FILES ${APPLOCKER_SOURCES} ${DBMGR_SOURCES})

# put the compiled executable in either the "bin\Debug" or "bin\Release" directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")

add_executable(applocker ${APPLOCKER_SOURCES} ${DBMGR_SOURCES})

target_compile_features(applocker PRIVATE cxx_std_17)
target_include_directories(applocker PRIVATE
    "${APPLOCKER_SRC_DIR}"
    "${APPLOCKER_SRC_DIR}/thirdparty/MJFS/inc"
    "${APPLOCKER_SRC_DIR}/thirdparty/MJMEM/inc"
    "${APPLOCKER_SRC_DIR}/thirdparty/MJSTR/inc"
    "${APPLOCKER_SRC_DIR}/thirdparty/MJSYNC/inc"
)
target_link_libraries(applocker PRIVATE
    # link MJFS module
    $<$<CONFIG:Debug>:${APPLOCKER_SRC_DIR}/thirdparty/MJFS/bin/${APPLOCKER_PLATFORM_ARCH}/Debug/mjfs.lib>
    $<$<CONFIG:Release>:${APPLOCKER_SRC_DIR}/thirdparty/MJFS/bin/${APPLOCKER_PLATFORM_ARCH}/Release/mjfs.lib>

    # link MJMEM module
    $<$<CONFIG:Debug>:${APPLOCKER_SRC_DIR}/thirdparty/MJMEM/bin/${APPLOCKER_PLATFORM_ARCH}/Debug/mjmem.lib>
    $<$<CONFIG:Release>:${APPLOCKER_SRC_DIR}/thirdparty/MJMEM/bin/${APPLOCKER_PLATFORM_ARCH}/Release/mjmem.lib>

    # link MJSTR module
    $<$<CONFIG:Debug>:${APPLOCKER_SRC_DIR}/thirdparty/MJSTR/bin/${APPLOCKER_PLATFORM_ARCH}/Debug/mjstr.lib>
    $<$<CONFIG:Release>:${APPLOCKER_SRC_DIR}/thirdparty/MJSTR/bin/${APPLOCKER_PLATFORM_ARCH}/Release/mjstr.lib>

    # link MJSYNC module
    $<$<CONFIG:Debug>:${APPLOCKER_SRC_DIR}/thirdparty/MJSYNC/bin/${APPLOCKER_PLATFORM_ARCH}/Debug/mjsync.lib>
    $<$<CONFIG:Release>:${APPLOCKER_SRC_DIR}/thirdparty/MJSYNC/bin/${APPLOCKER_PLATFORM_ARCH}/Release/mjsync.lib>

    # link WMI library
    wbemuuid.lib
)