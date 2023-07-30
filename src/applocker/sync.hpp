// sync.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _APPLOCKER_SYNC_HPP_
#define _APPLOCKER_SYNC_HPP_
#include <atomic>
#include <type_traits>
#include <Windows.h>

namespace applocker {
    class shared_lock {
    public:
        shared_lock() noexcept;
        ~shared_lock() noexcept;

        // acquires exclusive lock
        void lock() noexcept;

        // acquires shared lock
        void lock_shared() noexcept;

        // releases exclusive lock
        void unlock() noexcept;

        // releases shared lock
        void unlock_shared() noexcept;

    private:
        SRWLOCK _Myimpl;
    };

    class lock_guard { // automatically acquires and releases exclusive lock
    public:
        explicit lock_guard(shared_lock& _Lock) noexcept;
        ~lock_guard() noexcept;

    private:
        shared_lock& _Mylock;
    };

    class shared_lock_guard { // automatically acquires and releases shared lock
    public:
        explicit shared_lock_guard(shared_lock& _Lock) noexcept;
        ~shared_lock_guard() noexcept;

    private:
        shared_lock& _Mylock;
    };

    template <class _Ty>
    class locked_resource { // manages access to the shared resource
    public:
        locked_resource() noexcept : _Myres() {}

        ~locked_resource() noexcept {}

        locked_resource(const locked_resource&) = delete;
        locked_resource& operator=(const locked_resource&) = delete;

        _Ty* operator->() noexcept {
            lock_guard _Guard(_Mylock);
            return ::std::addressof(_Myres);
        }

        const _Ty* operator->() const noexcept {
            shared_lock_guard _Guard(_Mylock);
            return ::std::addressof(_Myres);
        }

        _Ty* get() noexcept {
            lock_guard _Guard(_Mylock);
            return ::std::addressof(_Myres);
        }

        const _Ty* get() const noexcept {
            shared_lock_guard _Guard(_Mylock);
            return ::std::addressof(_Myres);
        }

        void assign(const _Ty& _New_val) {
            lock_guard _Guard(_Mylock);
            _Myres = _New_val;
        }

    private:
        shared_lock _Mylock;
        _Ty _Myres;
    };

    class sync_flag { // atomic flag for threads synchronization
    public:
        sync_flag(const bool _Val = false) noexcept;
        ~sync_flag() noexcept;

        sync_flag(const sync_flag&) = delete;
        sync_flag& operator=(const sync_flag&) = delete;

        // checks if the flag is set
        bool is_set() const noexcept;

        // clears the flag
        void clear() noexcept;

        // sets the flag
        void set() noexcept;

    private:
        ::std::atomic<bool> _Myval;
    };
} // namespace applocker

#endif // _APPLOCKER_SYNC_HPP_