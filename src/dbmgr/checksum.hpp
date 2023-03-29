// checksum.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _DBMGR_CHECKSUM_HPP_
#define _DBMGR_CHECKSUM_HPP_
#include <cstdint>
#include <string_view>

namespace dbmgr {
    using ::std::wstring_view;

    extern bool _Is_sse42_present() noexcept;
    extern uint32_t _Compute_crc32c_simd(const void* _First, const void* const _Last) noexcept;
    extern uint32_t _Compute_crc32c_non_simd(const void* _First, const void* const _Last) noexcept;

    uint32_t compute_checksum(const wstring_view _Str) noexcept;
} // namespace dbmgr

#endif // _DBMGR_CHECKSUM_HPP_