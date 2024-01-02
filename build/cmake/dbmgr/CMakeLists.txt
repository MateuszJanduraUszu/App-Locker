# CMakeLists.txt

# Copyright (c) Mateusz Jandura. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)
project(dbmgr
    VERSION 1.0.3
    DESCRIPTION "App Locker Database Manager"
    LANGUAGES CXX
)

set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

# translate x64/Win32 into x64/x86
if(CMAKE_GENERATOR_PLATFORM STREQUAL x64)
    set(DBMGR_PLATFORM_ARCH x64)
elseif(CMAKE_GENERATOR_PLATFORM STREQUAL Win32)
    set(DBMGR_PLATFORM_ARCH x86)
else()
    set(DBMGR_PLATFORM_ARCH Invalid)
    message(FATAL_ERROR "Requires either x64 or Win32 platform architecture.")
endif()

set(CMAKE_SUPPRESS_REGENERATION TRUE)
if(MSVC)
    set(VS_SOURCE_GROUPS src)
endif()

set(DBMGR_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../src")
set(DBMGR_SOURCES
    "${DBMGR_SRC_DIR}/dbmgr/checksum.cpp"
    "${DBMGR_SRC_DIR}/dbmgr/checksum.hpp"
    "${DBMGR_SRC_DIR}/dbmgr/database.cpp"
    "${DBMGR_SRC_DIR}/dbmgr/database.hpp"
    "${DBMGR_SRC_DIR}/dbmgr/main.cpp"
    "${DBMGR_SRC_DIR}/dbmgr/task.cpp"
    "${DBMGR_SRC_DIR}/dbmgr/task.hpp"
    "${DBMGR_SRC_DIR}/dbmgr/tinywin.hpp"
)

# put all source files in "src" directory
source_group("src" FILES ${DBMGR_SOURCES})

# put the compiled executable in either the "bin\Debug" or "bin\Release" directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")

add_executable(dbmgr ${DBMGR_SOURCES})

target_compile_features(dbmgr PRIVATE cxx_std_17)
target_include_directories(dbmgr PRIVATE
    "${DBMGR_SRC_DIR}"
    "${DBMGR_SRC_DIR}/thirdparty/MJFS/inc"
    "${DBMGR_SRC_DIR}/thirdparty/MJMEM/inc"
    "${DBMGR_SRC_DIR}/thirdparty/MJSTR/inc"
    "${DBMGR_SRC_DIR}/thirdparty/MJSYNC/inc"
)
target_link_libraries(dbmgr PRIVATE
    # link MJFS module
    $<$<CONFIG:Debug>:${DBMGR_SRC_DIR}/thirdparty/MJFS/bin/${DBMGR_PLATFORM_ARCH}/Debug/mjfs.lib>
    $<$<CONFIG:Release>:${DBMGR_SRC_DIR}/thirdparty/MJFS/bin/${DBMGR_PLATFORM_ARCH}/Release/mjfs.lib>

    # link MJMEM module
    $<$<CONFIG:Debug>:${DBMGR_SRC_DIR}/thirdparty/MJMEM/bin/${DBMGR_PLATFORM_ARCH}/Debug/mjmem.lib>
    $<$<CONFIG:Release>:${DBMGR_SRC_DIR}/thirdparty/MJMEM/bin/${DBMGR_PLATFORM_ARCH}/Release/mjmem.lib>

    # link MJSTR module
    $<$<CONFIG:Debug>:${DBMGR_SRC_DIR}/thirdparty/MJSTR/bin/${DBMGR_PLATFORM_ARCH}/Debug/mjstr.lib>
    $<$<CONFIG:Release>:${DBMGR_SRC_DIR}/thirdparty/MJSTR/bin/${DBMGR_PLATFORM_ARCH}/Release/mjstr.lib>

    # link MJSYNC module
    $<$<CONFIG:Debug>:${DBMGR_SRC_DIR}/thirdparty/MJSYNC/bin/${DBMGR_PLATFORM_ARCH}/Debug/mjsync.lib>
    $<$<CONFIG:Release>:${DBMGR_SRC_DIR}/thirdparty/MJSYNC/bin/${DBMGR_PLATFORM_ARCH}/Release/mjsync.lib>
)