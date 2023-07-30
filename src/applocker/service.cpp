// service.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <applocker/directory_watcher.hpp>
#include <applocker/service.hpp>
#include <applocker/wmi.hpp>

namespace applocker {
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
                if (!_Watcher.is_watching()) { // watcher inactive, break
                    return 0;
                }

                while (!_Local_cache->_Flag.is_set()) {
                    switch (_Watcher.wait_for_changes()) {
                    case directory_watcher::stop_watching: // stop watching the database
                        _Local_cache->_Flag.set();
                        break;
                    case directory_watcher::update_required: // reload the database
                    {
                        ::dbmgr::database& _Db = ::dbmgr::database::current();
                        _Db.reload();
                        _Shared_cache._Locked_apps.assign(_Db.get_entries());
                        _Shared_cache._Task_event.notify(); // notify task's thread about the database changes
                        break;
                    }
                    default:
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
            _Mycache._Flag.set();
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
                _Cache->_State_event.notify(); // notify waiting thread
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
            _Cache->_State_event.notify();
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
        _Wmi_session _Session;
        if (!_Session._Connect()) {
            return;
        }

        _Database_modification_handler _Handler;
        bool _Terminated              = false;
        _Service_shared_cache& _Cache = _Service_shared_cache::_Get();
        while (!_Terminated) {
            switch (_Mycache._Get_state()) {
            case _Service_state::_Terminated:
                _Terminated = true;
                _Handler._Terminate();
                _Session._Terminate();
                break;
            case _Service_state::_Waiting:
                _Mycache._State_event.wait(true);
                break;
            case _Service_state::_Working:
            {
                _Cache._Task_event.wait(true);
                const auto& _Apps = *_Cache._Locked_apps.get();
                if (!_Apps.empty()) {
                    const auto& _Procs = *_Cache._New_procs.get();
                    for (const auto& _Proc : _Procs) {
                        for (const auto& _App : _Apps) {
                            if (_App.to_integer() == _Proc._Module_checksum) {
                                _Process_traits::_Terminate(_Proc._Id);
                                break;
                            }
                        }
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