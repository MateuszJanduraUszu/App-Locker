// service.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <applocker/service.hpp>
#include <applocker/directory_watcher.hpp>
#include <applocker/process.hpp>
#include <type_traits>

namespace applocker {
    _Service_cache::_Service_cache() noexcept
        : _Handle(nullptr), _Status(), _Event(), _State(_Service_state::_Working) {}

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

    _Service_shared_cache::_Service_shared_cache() noexcept : _Mylock(SRWLOCK_INIT), _Myentries() {
        _Set_entries(::dbmgr::database::current().get_entries());
    }

    _Service_shared_cache::~_Service_shared_cache() noexcept {}

    _Service_shared_cache::_Lock_guard::_Lock_guard(SRWLOCK& _Lock, const bool _Shared) noexcept
        : _Lock(::std::addressof(_Lock)), _Shared(_Shared) {
        if (this->_Shared) {
            ::AcquireSRWLockShared(this->_Lock);
        } else {
            ::AcquireSRWLockExclusive(this->_Lock);
        }
    }

    _Service_shared_cache::_Lock_guard::~_Lock_guard() noexcept {
        if (_Shared) {
            ::ReleaseSRWLockShared(_Lock);
        } else {
            ::ReleaseSRWLockExclusive(_Lock);
        }
    }

    _Service_shared_cache& _Service_shared_cache::_Get() noexcept {
        static _Service_shared_cache _Cache;
        return _Cache;
    }

    vector<::dbmgr::database::entry_type> _Service_shared_cache::_Get_entries() const noexcept {
        _Lock_guard _Guard(_Mylock, true);
        return _Myentries;
    }

    void _Service_shared_cache::_Set_entries(
        const vector<::dbmgr::database::entry_type>& _New_entries) noexcept {
        _Lock_guard _Guard(_Mylock, false);
        _Myentries = _New_entries;
    }

    _Database_modification_handler::_Database_modification_handler() noexcept
        : _Mycache(), _Mythread(_Create_thread(::std::addressof(_Mycache))) {}

    _Database_modification_handler::~_Database_modification_handler() noexcept {
        _Terminate();
    }

    void* _Database_modification_handler::_Create_thread(_Thread_cache* const _Cache) noexcept {
        return ::CreateThread(nullptr, 0,
            [](void* _Data) -> unsigned long {
                _Thread_cache* _Local_cache          = static_cast<_Thread_cache*>(_Data);
                _Service_shared_cache& _Shared_cache = _Service_shared_cache::_Get();
                directory_watcher _Watcher(_Local_cache->_Event);
                while (!_Local_cache->_Flag.test()) {
                    // Note: The directory_watcher::wait_for_changes() returns true if any modifications
                    //       has been made to the directory being monitored since the last call to this
                    //       method. Otherwise it returns false, which means that we have been notified
                    //       to end monitoring the directory.
                    if (_Watcher.wait_for_changes()) { // refresh the database
                        ::dbmgr::database& _Db = ::dbmgr::database::current();
                        _Db.reload();
                        _Shared_cache._Set_entries(_Db.get_entries());
                    } else { // end monitoring the database
                        break;
                    }
                }

                return 0;
            },
            _Cache, 0, nullptr
        );
    }

    void _Database_modification_handler::_Terminate() noexcept {
        if (_Mythread) {
            _Mycache._Flag.test_and_set();
            _Mycache._Event.notify();
            ::WaitForSingleObject(_Mythread, 0xFFFF'FFFF); // wait for the thread termination
            ::CloseHandle(_Mythread);
            _Mythread = nullptr;
        }
    }

    service_launcher::service_launcher() noexcept : _Mycache() {
        _Init();
        if (!_Register_control_handler()) {
            _Mycache._Set_state(_Service_state::_Terminated);
        }
    }

    service_launcher::~service_launcher() noexcept {}

    unsigned long __stdcall service_launcher::_Control_handler(
        unsigned long _Code, unsigned long, void*, void* _Ctx) noexcept {
        _Service_cache* const _Cache = static_cast<_Service_cache*>(_Ctx);
        switch (_Code) {
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
        case SERVICE_CONTROL_PRESHUTDOWN:
        {
            _Cache->_Status.dwCurrentState  = SERVICE_STOPPED;
            const _Service_state _Old_state = _Cache->_Get_state();
            _Cache->_Set_state(_Service_state::_Terminated);
            if (_Old_state == _Service_state::_Waiting) {
                _Cache->_Event.notify(); // notify waiting thread
            }

            _Cache->_Submit();
            break;
        }
        case SERVICE_CONTROL_PAUSE:
            _Cache->_Status.dwCurrentState = SERVICE_PAUSED;
            _Cache->_Set_state(_Service_state::_Waiting);
            _Cache->_Submit();
            break;
        case SERVICE_CONTROL_CONTINUE:
            _Cache->_Status.dwCurrentState = SERVICE_RUNNING;
            _Cache->_Set_state(_Service_state::_Working);
            _Cache->_Event.notify();
            _Cache->_Submit();
            break;
        default:
            break;
        }

        return 0; // no error
    }

    void service_launcher::_Init() noexcept {
        SERVICE_STATUS& _Status           = _Mycache._Status;
        _Status.dwCheckPoint              = 0;
        _Status.dwControlsAccepted        = SERVICE_CONTROL_PRESHUTDOWN
            | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
        _Status.dwCurrentState            = SERVICE_STOPPED;
        _Status.dwServiceSpecificExitCode = 0;
        _Status.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
        _Status.dwWaitHint                = 0;
        _Status.dwWin32ExitCode           = NO_ERROR;
        _Mycache._Submit();
    }

    [[nodiscard]] bool service_launcher::_Register_control_handler() noexcept {
        _Mycache._Handle = ::RegisterServiceCtrlHandlerExW(
            service_name, &_Control_handler, ::std::addressof(_Mycache));
        return _Mycache._Handle != nullptr;
    }

    void service_launcher::_Set_state(const unsigned long _New_state) noexcept {
        _Mycache._Status.dwCurrentState = _New_state;
        _Mycache._Submit();
    }

    void service_launcher::_Perform_task() noexcept {
        _Database_modification_handler _Handler;
        bool _Terminated              = false;
        _Service_shared_cache& _Cache = _Service_shared_cache::_Get();
        while (!_Terminated) {
            switch (_Mycache._Get_state()) {
            case _Service_state::_Terminated:
                _Terminated = true;
                _Handler._Terminate();
                break;
            case _Service_state::_Waiting:
                _Mycache._Event.wait();
                _Mycache._Event.reset();
                break;
            case _Service_state::_Working:
            {
                ::Sleep(3000); // avoid high CPU usage
                const auto& _Entries = _Cache._Get_entries();
                if (!_Entries.empty()) {
                    for (const uint32_t _Id : _Find_process_ids(_Entries)) {
                        _Terminate_process(_Id);
                    }
                }

                break;
            }
            default:
                break;
            }
        }
    }

    bool service_launcher::is_launch_possible() const noexcept {
        return _Mycache._Get_state() != _Service_state::_Terminated;
    }

    void service_launcher::launch() noexcept {
        _Set_state(SERVICE_RUNNING);
        _Perform_task();
        _Set_state(SERVICE_STOPPED);
    }

    void __stdcall service_entry(unsigned long, wchar_t**) noexcept {
        service_launcher _Launcher;
        if (_Launcher.is_launch_possible()) {
            _Launcher.launch();
        }
    }
} // namespace applocker