// process.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _APPLOCKER_PROCESS_HPP_
#define _APPLOCKER_PROCESS_HPP_
#include <cstdint>
#include <dbmgr/checksum.hpp>
#include <vector>

namespace applocker {
    class _Toolhelp_snapshot {
    public:
        void* _Handle;

        _Toolhelp_snapshot() noexcept;
        ~_Toolhelp_snapshot() noexcept;

        // checks if the snapshot handle is valid
        bool _Valid() const noexcept;

    private:
        // creates toolhelp snapshot
        [[nodiscard]] static void* _Create() noexcept;
    };

    struct _Process_traits {
        struct _Basic_data {
            uint32_t _Id; // process ID (PID)
            ::dbmgr::checksum_t _Module_checksum; // process image file checksum
        };

        using _Process_list = ::std::vector<_Basic_data>;

        // returns basic data of all running processes
        static _Process_list _Get_process_list();

        // terminates the specified process
        static void _Terminate(const uint32_t _Id) noexcept;
    };

    using _Process_list = _Process_traits::_Process_list;
} // namespace applocker

#endif // _APPLOCKER_PROCESS_HPP_