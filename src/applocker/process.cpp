// process.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <applocker/process.hpp>
#include <dbmgr/tinywin.hpp>
#include <TlHelp32.h>

namespace mjx {
    _Toolhelp_snapshot::_Toolhelp_snapshot() noexcept : _Handle(_Create()) {}

    _Toolhelp_snapshot::~_Toolhelp_snapshot() noexcept {
        if (_Valid()) {
            ::CloseHandle(_Handle);
            _Handle = nullptr;
        }
    }

    [[nodiscard]] void* _Toolhelp_snapshot::_Create() noexcept {
        return ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    }

    bool _Toolhelp_snapshot::_Valid() const noexcept {
        return _Handle != nullptr && _Handle != INVALID_HANDLE_VALUE;
    }

    _Process_traits::_Process_list _Process_traits::_Get_process_list() {
        _Toolhelp_snapshot _Snapshot;
        if (!_Snapshot._Valid()) {
            return _Process_list{};
        }

        PROCESSENTRY32W _Entry = {0};
        _Entry.dwSize          = sizeof(PROCESSENTRY32W);
        bool _Next             = ::Process32FirstW(_Snapshot._Handle, &_Entry);
        _Process_list _List;
        _Basic_data _Data;
        while (_Next) {
            _Data._Id              = _Entry.th32ProcessID;
            _Data._Module_checksum = compute_checksum(_Entry.szExeFile);
            _List.push_back(_Data);
            _Next = ::Process32NextW(_Snapshot._Handle, &_Entry);
        }

        return _List;
    }

    void _Process_traits::_Terminate(const uint32_t _Id) noexcept {
        void* const _Handle = ::OpenProcess(PROCESS_TERMINATE, false, _Id);
        if (_Handle) { // valid process, try to terminate
            ::TerminateProcess(_Handle, 0);
            ::CloseHandle(_Handle);
        }
    }
} // namespace mjx