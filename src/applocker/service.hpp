// service.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _APPLOCKER_SERVICE_HPP_
#define _APPLOCKER_SERVICE_HPP_
#include <applocker/waitable_event.hpp>
#include <dbmgr/database.hpp>
#include <atomic>
#include <vector>
#include <windows.h>

namespace applocker {
    using ::std::atomic;
    using ::std::atomic_flag;
    using ::std::vector;

    enum class _Service_state : unsigned char {
        _Terminated,
        _Waiting,
        _Working
    };

    class _Service_cache {
    public:
        SERVICE_STATUS_HANDLE _Handle;
        SERVICE_STATUS _Status;
        waitable_event _Event;
        atomic<_Service_state> _State;

        _Service_cache() noexcept;
        ~_Service_cache() noexcept;

        _Service_state _Get_state() const noexcept;
        void _Set_state(const _Service_state _New_state) noexcept;
        void _Submit() noexcept;
    };

    class _Service_shared_cache {
    public:
        ~_Service_shared_cache() noexcept;

        static _Service_shared_cache& _Get() noexcept;
        vector<::dbmgr::database::entry_type> _Get_entries() const noexcept;
        void _Set_entries(const vector<::dbmgr::database::entry_type>& _New_entries) noexcept;

    private:
        _Service_shared_cache() noexcept;
        
        struct _Lock_guard {
            SRWLOCK* _Lock;
            bool _Shared;

            explicit _Lock_guard(SRWLOCK& _Lock, const bool _Shared) noexcept;
            ~_Lock_guard() noexcept;
        };

        mutable SRWLOCK _Mylock;
        vector<::dbmgr::database::entry_type> _Myentries;
    };

    class _Database_modification_handler { // handles database modification at runtime
    public:
        _Database_modification_handler() noexcept;
        ~_Database_modification_handler() noexcept;

        void _Terminate() noexcept;

    private:
        struct _Thread_cache {
            waitable_event _Event;
            atomic_flag _Flag;
        };

        static void* _Create_thread(_Thread_cache* const _Cache) noexcept;
    
        _Thread_cache _Mycache;
        void* _Mythread;
    };

    class service_launcher {
    public:
        service_launcher() noexcept;
        ~service_launcher() noexcept;

        bool is_launch_possible() const noexcept;
        void launch() noexcept;

        static constexpr wchar_t service_name[] = L"App Locker";

    private:
        static unsigned long __stdcall _Control_handler(
            unsigned long _Code, unsigned long, void*, void* _Ctx) noexcept;
        void _Init() noexcept;
        [[nodiscard]] bool _Register_control_handler() noexcept;
        void _Set_state(const unsigned long _New_state) noexcept;
        void _Perform_task() noexcept;

        _Service_cache _Mycache;
    };

    void __stdcall service_entry(unsigned long, wchar_t**) noexcept;
} // namespace applocker

#endif // _APPLOCKER_SERVICE_HPP_