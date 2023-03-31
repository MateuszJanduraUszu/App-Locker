// waitable_event.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <applocker/waitable_event.hpp>
#include <Windows.h>

namespace applocker {
    void* _Create_waitable_event() noexcept {
        return ::CreateEventW(nullptr, true, false, nullptr);
    }

    waitable_event::waitable_event() noexcept : _Myimpl(_Create_waitable_event()) {}

    waitable_event::~waitable_event() noexcept {
        if (_Myimpl) {
            ::CloseHandle(_Myimpl);
        }
    }

    bool waitable_event::good() const noexcept {
        return _Myimpl != nullptr;
    }

    const waitable_event::native_handle_type waitable_event::native_handle() const noexcept {
        return _Myimpl;
    }

    void waitable_event::wait() noexcept {
        if (_Myimpl) {
            ::WaitForSingleObject(_Myimpl, 0xFFFF'FFFF); // infinite timeout
        }
    }

    void waitable_event::notify() noexcept {
        if (_Myimpl) {
            ::SetEvent(_Myimpl);
        }
    }

    void waitable_event::reset() noexcept {
        if (_Myimpl) {
            ::ResetEvent(_Myimpl);
        }
    }
} // namespace applocker