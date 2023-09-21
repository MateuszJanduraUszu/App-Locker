// service_caches.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <applocker/service_caches.hpp>

namespace applocker {
    _Service_cache::_Service_cache() noexcept
        : _Handle(nullptr), _Status(), _State_event(), _State(_Service_state::_Working) {}

    _Service_cache::~_Service_cache() noexcept {}

    _Service_state _Service_cache::_Get_state() const noexcept {
        return _State.load(::std::memory_order_relaxed);
    }

    void _Service_cache::_Set_state(const _Service_state _New_state) noexcept {
        _State.store(_New_state, ::std::memory_order_relaxed);
    }

    void _Service_cache::_Submit() noexcept {
        ++_Status.dwCheckPoint;
        ::SetServiceStatus(_Handle, ::std::addressof(_Status));
    }

    _Service_shared_cache::_Service_shared_cache() : _Locked_apps(), _New_procs(), _Task_event() {
        // Note: Immediate notification of the task thread is essential after the database is loaded.
        //       This is because some locked processes may still be running. At this stage, _New_procs
        //       doesn't yet contain any processes, so the task thread will scan existing processes to
        //       identify any that need further attention.
        _Locked_apps.assign(::dbmgr::database::current().get_entries());
        _Task_event.notify();
    }

    _Service_shared_cache::~_Service_shared_cache() noexcept {}

    _Service_shared_cache& _Service_shared_cache::_Get() noexcept {
        static _Service_shared_cache _Cache;
        return _Cache;
    }
} // namespace applocker