// directory_watcher.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _APPLOCKER_DIRECTORY_WATCHER_HPP_
#define _APPLOCKER_DIRECTORY_WATCHER_HPP_
#include <applocker/waitable_event.hpp>
#include <dbmgr/database.hpp>
#include <Windows.h>

namespace applocker {
    template <class _Elem, size_t _Size>
    constexpr size_t _Str_size(const _Elem(&)[_Size]) noexcept {
        return _Size;
    }

    class directory_watcher { // class for database file observation
    public:
        directory_watcher(waitable_event& _Event) noexcept;
        ~directory_watcher() noexcept;

        directory_watcher()                                    = delete;
        directory_watcher(const directory_watcher&)            = delete;
        directory_watcher& operator=(const directory_watcher&) = delete;

        enum wait_result : unsigned char {
            error,
            stop_watching,
            continue_wait,
            update_required
        };

        // checks if the watcher is watching
        bool is_watching() const noexcept;

        // waits for some directory changes
        wait_result wait_for_changes() noexcept;

    private:
        struct _Notification_events {
            explicit _Notification_events(waitable_event& _Event) noexcept;

            waitable_event& _Thread_event;
            waitable_event _Dir_event;
        };

        // Note: The max buffer size used by ReadDirectoryChangesW() can be calculated using this
        //       equation: sizeof(FILE_NOTIFY_INFORMATION) + (strlen(filename) * sizeof(wchar_t)).
        //       It is correct since we are only interested in one file (apps.db).
#ifdef _M_X64
        static constexpr unsigned long _Max_buffer_size = static_cast<unsigned long>(
            sizeof(FILE_NOTIFY_INFORMATION) + (_Str_size("apps.db") * sizeof(wchar_t)));
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86 vvv
        static constexpr unsigned long _Max_buffer_size = sizeof(FILE_NOTIFY_INFORMATION)
            + (_Str_size("apps.db") * sizeof(wchar_t));
#endif // _M_X64

        // opens the watched directory
        [[nodiscard]] static void* _Open_watched_directory() noexcept;

        // checks if the watcher should notify waiting thread
        static bool _Should_notify(FILE_NOTIFY_INFORMATION* const _Info) noexcept;

        void* _Mydir;
        unsigned char _Mybuf[_Max_buffer_size];
        _Notification_events _Myevents;
        OVERLAPPED _Myovl;
    };
} // namespace applocker

#endif // _APPLOCKER_DIRECTORY_WATCHER_HPP_