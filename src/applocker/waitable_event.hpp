// waitable_event.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _APPLOCKER_WAITABLE_EVENT_HPP_
#define _APPLOCKER_WAITABLE_EVENT_HPP_

namespace applocker {
    class waitable_event { // waiting-based event
    public:
        using native_handle_type = void*;

        waitable_event() noexcept;
        ~waitable_event() noexcept;

        waitable_event(const waitable_event&) = delete;
        waitable_event& operator=(const waitable_event&) = delete;

        // checks if the event is ready to use
        bool good() const noexcept;

        // returns a handle to the native implementation
        const native_handle_type native_handle() const noexcept;
        
        // waits for the event notification
        void wait(const bool _Reset = false) noexcept;
        
        // notifies the event
        void notify() noexcept;
        
        // resets the event
        void reset() noexcept;

    private:
        // creates a new event
        static void* _Create() noexcept;

        void* _Myimpl;
    };
} // namespace applocker

#endif // _APPLOCKER_WAITABLE_EVENT_HPP_