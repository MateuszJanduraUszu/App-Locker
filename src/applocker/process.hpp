// process.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _APPLOCKER_PROCESS_HPP_
#define _APPLOCKER_PROCESS_HPP_
#include <cstdint>
#include <dbmgr/checksum.hpp>

namespace applocker {
    struct _Process_traits {
        struct _Basic_data {
            uint32_t _Id; // process ID (PID)
            ::dbmgr::checksum_t _Module_checksum; // process image file checksum
        };

        // terminates the specified process
        static void _Terminate(const uint32_t _Id) noexcept;
    };
} // namespace applocker

#endif // _APPLOCKER_PROCESS_HPP_