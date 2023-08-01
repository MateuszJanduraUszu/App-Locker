// process.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <applocker/process.hpp>
#include <Windows.h>
#include <TlHelp32.h>
#include <type_traits>

namespace applocker {
    _Toolhelp_snapshot::_Toolhelp_snapshot() noexcept : _Handle(_Create()) {}

    _Toolhelp_snapshot::~_Toolhelp_snapshot() noexcept {
        if (_Handle) {
            ::CloseHandle(_Handle);
            _Handle = nullptr;
        }
    }

    [[nodiscard]] void* _Toolhelp_snapshot::_Create() noexcept {
        return ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    }

    _Process_traits::_Process_list _Process_traits::_Get_process_list() {
        _Toolhelp_snapshot _Snapshot;
        if (!_Snapshot._Handle) {
            return _Process_list{};
        }

        PROCESSENTRY32W _Entry = {0};
        _Entry.dwSize          = sizeof(PROCESSENTRY32W);
        bool _Next             = ::Process32FirstW(_Snapshot._Handle, ::std::addressof(_Entry));
        _Process_list _Result;
        _Basic_data _Data;
        while (_Next) {
            _Data._Id              = _Entry.th32ProcessID;
            _Data._Module_checksum = ::dbmgr::compute_checksum(_Entry.szExeFile);
            _Result.push_back(_Data);
            _Next = ::Process32NextW(_Snapshot._Handle, ::std::addressof(_Entry));
        }

        return _Result;
    }

    void _Process_traits::_Terminate(const uint32_t _Id) noexcept {
        void* const _Handle = ::OpenProcess(PROCESS_TERMINATE, false, _Id);
        if (_Handle) { // valid process, try to terminate
            ::TerminateProcess(_Handle, 0);
            ::CloseHandle(_Handle);
        }
    }
} // namespace applocker