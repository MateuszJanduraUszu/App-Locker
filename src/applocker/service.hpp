// service.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _APPLOCKER_SERVICE_HPP_
#define _APPLOCKER_SERVICE_HPP_
#include <applocker/service_caches.hpp>
#include <applocker/sync.hpp>

namespace mjx {
    class _Database_modification_handler { // handles database modification at runtime
    public:
        _Database_modification_handler() noexcept;
        ~_Database_modification_handler() noexcept;

        // terminates database modification handler thread
        void _Terminate() noexcept;

    private:
        struct _Thread_cache {
            waitable_event _Event;
            _Sync_flag _Flag;
        };

        // creates database modification handler thread
        static void* _Create_thread(_Thread_cache* const _Cache) noexcept;
    
        _Thread_cache _Mycache;
        void* _Mythread;
    };

    class service_launcher {
    public:
        service_launcher() noexcept;
        ~service_launcher() noexcept;

        static constexpr wchar_t service_name[] = L"App Locker";

        // checks if launch is possible
        bool is_launch_possible() const noexcept;
        
        // launches the service
        void launch();

    private:
        // handles the service control signals
        static unsigned long __stdcall _Control_handler(
            unsigned long _Code, unsigned long, void*, void* _Ctx) noexcept;

        // initializes the service
        void _Init() noexcept;

        // registers the service control handler
        [[nodiscard]] bool _Register_control_handler() noexcept;
        
        // changes the service state
        void _Set_state(const unsigned long _New_state) noexcept;
        
        // performs the service task
        void _Perform_task();

        _Service_cache _Mycache;
    };

    void __stdcall service_entry(unsigned long, wchar_t**);
} // namespace mjx

#endif // _APPLOCKER_SERVICE_HPP_