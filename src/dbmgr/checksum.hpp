// checksum.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _DBMGR_CHECKSUM_HPP_
#define _DBMGR_CHECKSUM_HPP_
#include <cstdint>
#include <mjstr/string_view.hpp>

namespace mjx {
    using checksum_t = uint32_t; // 32-bit unsigned integer

    struct _Crc32c_traits {
        // checks if SSE4.2 SIMD extension can be used
        static bool _Use_sse42() noexcept;
    
        // computes CRC-32C checksum with SSE4.2 SIMD extension support
        static checksum_t _Compute_sse42(const void* _First, const void* const _Last) noexcept;
    
        // computes CRC-32C checksum without SSE4.2 SIMD extension support
        static checksum_t _Compute_software(const void* _First, const void* const _Last) noexcept;
    };

    checksum_t compute_checksum(const unicode_string_view _Str) noexcept;
} // namespace mjx

#endif // _DBMGR_CHECKSUM_HPP_