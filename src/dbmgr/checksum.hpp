// checksum.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _DBMGR_CHECKSUM_HPP_
#define _DBMGR_CHECKSUM_HPP_
#include <cstdint>
#include <string_view>

namespace dbmgr {
    struct _Crc32c_traits {
        // checks if SSE4.2 SIMD extension can be used
        static bool _Use_sse42() noexcept;
    
        // computes CRC-32C checksum with SSE4.2 SIMD extension support
        static uint32_t _Compute_sse42(const void* _First, const void* const _Last) noexcept;
    
        // computes CRC-32C checksum without SSE4.2 SIMD extension support
        static uint32_t _Compute_normal(const void* _First, const void* const _Last) noexcept;
    };

    uint32_t compute_checksum(const ::std::wstring_view _Str) noexcept;
} // namespace dbmgr

#endif // _DBMGR_CHECKSUM_HPP_