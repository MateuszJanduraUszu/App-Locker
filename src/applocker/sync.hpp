// sync.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _APPLOCKER_SYNC_HPP_
#define _APPLOCKER_SYNC_HPP_
#include <atomic>
#include <mjsync/srwlock.hpp>
#include <type_traits>

namespace mjx {
    template <class _Ty>
    class _Locked_resource { // manages access to the shared resource
    public:
        _Locked_resource() noexcept : _Myres(), _Mylock() {}

        ~_Locked_resource() noexcept {}

        _Locked_resource(const _Locked_resource&)            = delete;
        _Locked_resource& operator=(const _Locked_resource&) = delete;

        _Ty& _Get() noexcept {
            lock_guard _Guard(_Mylock);
            return _Myres;
        }

        const _Ty& _Get() const noexcept {
            shared_lock_guard _Guard(_Mylock);
            return _Myres;
        }

        void _Assign(const _Ty& _New_val) {
            lock_guard _Guard(_Mylock);
            _Myres = _New_val;
        }

        void _Assign(_Ty&& _New_val) {
            lock_guard _Guard(_Mylock);
            _Myres = ::std::move(_New_val);
        }

    private:
        _Ty _Myres;
        shared_lock _Mylock;
    };

    class _Sync_flag { // atomic flag for threads synchronization
    public:
        _Sync_flag(const bool _Val = false) noexcept;
        ~_Sync_flag() noexcept;

        _Sync_flag(const _Sync_flag&)            = delete;
        _Sync_flag& operator=(const _Sync_flag&) = delete;

        // checks if the flag is set
        bool _Is_set() const noexcept;

        // clears the flag
        void _Clear() noexcept;

        // sets the flag
        void _Set() noexcept;

    private:
        ::std::atomic<bool> _Myval;
    };
} // namespace mjx

#endif // _APPLOCKER_SYNC_HPP_