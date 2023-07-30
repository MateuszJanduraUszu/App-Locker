// sync.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <applocker/sync.hpp>

namespace applocker {
    shared_lock::shared_lock() noexcept : _Myimpl(SRWLOCK_INIT) {}

    shared_lock::~shared_lock() noexcept {}

    void shared_lock::lock() noexcept {
        ::AcquireSRWLockExclusive(::std::addressof(_Myimpl));
    }

    void shared_lock::lock_shared() noexcept {
        ::AcquireSRWLockShared(::std::addressof(_Myimpl));
    }

    void shared_lock::unlock() noexcept {
        ::ReleaseSRWLockExclusive(::std::addressof(_Myimpl));
    }

    void shared_lock::unlock_shared() noexcept {
        ::ReleaseSRWLockShared(::std::addressof(_Myimpl));
    }

    lock_guard::lock_guard(shared_lock& _Lock) noexcept : _Mylock(_Lock) {
        _Mylock.lock();
    }

    lock_guard::~lock_guard() noexcept {
        _Mylock.unlock();
    }

    shared_lock_guard::shared_lock_guard(shared_lock& _Lock) noexcept : _Mylock(_Lock) {
        _Mylock.lock_shared();
    }

    shared_lock_guard::~shared_lock_guard() noexcept {
        _Mylock.unlock_shared();
    }

    sync_flag::sync_flag(const bool _Val) noexcept : _Myval(_Val) {}

    sync_flag::~sync_flag() noexcept {}

    bool sync_flag::is_set() const noexcept {
        return _Myval.load(::std::memory_order_relaxed);
    }

    void sync_flag::clear() noexcept {
        _Myval.store(false, ::std::memory_order_relaxed);
    }

    void sync_flag::set() noexcept {
        _Myval.store(true, ::std::memory_order_relaxed);
    }
} // namespace applocker