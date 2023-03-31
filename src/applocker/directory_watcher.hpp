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
    extern [[nodiscard]] void* _Open_watched_directory() noexcept;

    template <class _Elem, size_t _Size>
    consteval size_t _Str_size(const _Elem(&)[_Size]) noexcept {
        return _Size;
    }

    class directory_watcher { // class for database file observation
    public:
        directory_watcher(waitable_event& _Event) noexcept;
        ~directory_watcher() noexcept;

        directory_watcher() = delete;
        directory_watcher(const directory_watcher&) = delete;
        directory_watcher& operator=(const directory_watcher&) = delete;

        bool is_watching() const noexcept;
        bool wait_for_changes() noexcept;

    private:
        struct _Notification_events {
            explicit _Notification_events(waitable_event& _Event) noexcept;

            waitable_event& _Thread_event;
            waitable_event _Dir_event;
        };

        // Note: The max buffer size used by ReadDirectoryChangesW() can be calculated using this
        //       equation: sizeof(FILE_NOTIFY_INFORMATION) + (strlen(filename) * sizeof(wchar_t)).
        //       It is correct since we are only interested in one file (apps.db).
        static constexpr unsigned long _Max_buffer_size = sizeof(FILE_NOTIFY_INFORMATION)
            + (_Str_size("apps.db") * sizeof(wchar_t));

        static bool _Should_notify(FILE_NOTIFY_INFORMATION* const _Info) noexcept;

        void* _Mydir;
        unsigned char _Mybuf[_Max_buffer_size];
        _Notification_events _Myevents;
        OVERLAPPED _Myovl;
    };
} // namespace applocker

#endif // _APPLOCKER_DIRECTORY_WATCHER_HPP_