// waitable_event.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _APPLOCKER_WAITABLE_EVENT_HPP_
#define _APPLOCKER_WAITABLE_EVENT_HPP_

namespace applocker {
    extern void* _Create_waitable_event() noexcept;

    class waitable_event { // waiting-based event
    public:
        using native_handle_type = void*;

        waitable_event() noexcept;
        ~waitable_event() noexcept;

        waitable_event(const waitable_event&) = delete;
        waitable_event& operator=(const waitable_event&) = delete;

        bool good() const noexcept;
        const native_handle_type native_handle() const noexcept;
        void wait() noexcept;
        void notify() noexcept;
        void reset() noexcept;

    private:
        void* _Myimpl;
    };
} // namespace applocker

#endif // _APPLOCKER_WAITABLE_EVENT_HPP_