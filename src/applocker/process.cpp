// process.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <applocker/process.hpp>
#include <Windows.h>

namespace applocker {
    void _Process_traits::_Terminate(const uint32_t _Id) noexcept {
        void* const _Handle = ::OpenProcess(PROCESS_TERMINATE, false, _Id);
        if (_Handle) { // valid process, try to terminate
            ::TerminateProcess(_Handle, 0);
            ::CloseHandle(_Handle);
        }
    }
} // namespace applocker