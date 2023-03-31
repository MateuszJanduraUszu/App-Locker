// process.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <applocker/process.hpp>
#include <dbmgr/checksum.hpp>
#include <Windows.h>
#include <TlHelp32.h>
#include <type_traits>

namespace applocker {
    vector<uint32_t> _Find_process_ids(
        const vector<::dbmgr::database::entry_type>& _Targets) noexcept {
        void* _Snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
        if (!_Snapshot) {
            return vector<uint32_t>{};
        }
        
        PROCESSENTRY32W _Entry = {0};
        _Entry.dwSize          = sizeof(PROCESSENTRY32W);
        bool _Next             = ::Process32FirstW(_Snapshot, ::std::addressof(_Entry));
        vector<uint32_t> _Result;
        while (_Next) {
            const uint32_t _Checksum = ::dbmgr::compute_checksum(_Entry.szExeFile);
            for (const auto& _Target : _Targets) {
                if (::memcmp(&_Checksum, _Target.data(), _Target.size()) == 0) {
                    _Result.push_back(_Entry.th32ProcessID);
                    break; // checksums are unique so we cannot find another similar to that one
                }
            }

            _Next = ::Process32NextW(_Snapshot, ::std::addressof(_Entry));
        }

        ::CloseHandle(_Snapshot);
        return _Result;
    }

    void _Terminate_process(const uint32_t _Id) noexcept {
        void* _Handle = ::OpenProcess(PROCESS_TERMINATE, false, _Id);
        if (_Handle) {
            ::TerminateProcess(_Handle, 0);
            ::CloseHandle(_Handle);
            _Handle = nullptr;
        }
    }
} // namespace applocker