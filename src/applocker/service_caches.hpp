// service_caches.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _APPLOCKER_SERVICE_CACHES_HPP_
#define _APPLOCKER_SERVICE_CACHES_HPP_
#include <applocker/process.hpp>
#include <applocker/sync.hpp>
#include <dbmgr/database.hpp>
#include <dbmgr/tinywin.hpp>
#include <mjsync/waitable_event.hpp>
#include <vector>
#include <winsvc.h>

namespace mjx {
    enum class _Service_state : unsigned char {
        _Terminated,
        _Waiting,
        _Working
    };

    class _Service_cache { // service's internal cache
    public:
        SERVICE_STATUS_HANDLE _Handle;
        SERVICE_STATUS _Status;
        waitable_event _State_event;
        ::std::atomic<_Service_state> _State;

        _Service_cache() noexcept;
        ~_Service_cache() noexcept;

        // returns the current state
        _Service_state _Get_state() const noexcept;

        // changes the current state
        void _Set_state(const _Service_state _New_state) noexcept;
        
        // submits a new state
        void _Submit() noexcept;
    };

    class _Service_shared_cache { // service's shared cache
    public:
        _Locked_resource<::std::vector<database_entry>> _Locked_apps;
        _Locked_resource<_Process_list> _New_procs;
        waitable_event _Task_event;

        ~_Service_shared_cache() noexcept;

        // returns an instance of this class
        static _Service_shared_cache& _Get() noexcept;

    private:
        _Service_shared_cache();
    };
} // namespace mjx

#endif // _APPLOCKER_SERVICE_CACHES_HPP_