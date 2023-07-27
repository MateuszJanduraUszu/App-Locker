// directory_watcher.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <applocker/directory_watcher.hpp>
#include <cwchar>

namespace applocker {
    directory_watcher::directory_watcher(waitable_event& _Event) noexcept
        : _Mydir(_Open_watched_directory()), _Mybuf(), _Myevents(_Event), _Myovl() {
        if (_Mydir) {
            _Myovl.hEvent = _Myevents._Dir_event.native_handle();
        }
    }

    directory_watcher::~directory_watcher() noexcept {
        if (_Mydir) {
            ::CloseHandle(_Mydir);
            _Mydir = nullptr;
        }
    }

    directory_watcher::_Notification_events::_Notification_events(waitable_event& _Event) noexcept
        : _Thread_event(_Event), _Dir_event() {}

    [[nodiscard]] void* directory_watcher::_Open_watched_directory() noexcept {
        return ::CreateFileW(::dbmgr::database_location::current().directory().c_str(),
            GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);
    }

    bool directory_watcher::_Should_notify(FILE_NOTIFY_INFORMATION* const _Info) noexcept {
        static constexpr size_t _Expected_length = _Str_size("apps.db") - 1; // exclute null-terminator
        _Info->FileNameLength                   /= sizeof(wchar_t);
        if (_Info->FileNameLength != _Expected_length) {
            return false;
        }

        return ::wcsncmp(_Info->FileName, L"apps.db", _Expected_length) == 0;
    }

    bool directory_watcher::is_watching() const noexcept {
        return _Mydir != nullptr;
    }

    directory_watcher::wait_result directory_watcher::wait_for_changes() noexcept {
        // Note: We are waiting for two events. The first one signals that changes have been
        //       made to the directory being monitored. The second event is used to interrupt
        //       the current thread's waiting state when waiting for directory changes.
        void* _Events[2] = {
            _Myevents._Dir_event.native_handle(), _Myevents._Thread_event.native_handle()};
        unsigned long _Bytes; // returned bytes (unused)
        if (::ReadDirectoryChangesW(_Mydir, _Mybuf, _Max_buffer_size, false,
            FILE_NOTIFY_CHANGE_LAST_WRITE, &_Bytes, ::std::addressof(_Myovl), nullptr) == 0) {
            return error;
        }

        switch (::WaitForMultipleObjects(2, _Events, false, 0xFFFF'FFFF)) {
        case 0: // notified by directory's event (WAIT_OBJECT_0)
            break;
        case 1: // notified by thread's event (WAIT_OBJECT_0 + 1)
            return stop_watching;
        default: // error occured
            return error;
        };

        return _Should_notify(
            reinterpret_cast<FILE_NOTIFY_INFORMATION*>(_Mybuf)) ? update_required : continue_wait;
    }
} // namespace applocker