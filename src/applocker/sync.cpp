// sync.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <applocker/sync.hpp>

namespace mjx {
    _Sync_flag::_Sync_flag(const bool _Val) noexcept : _Myval(_Val) {}

    _Sync_flag::~_Sync_flag() noexcept {}

    bool _Sync_flag::_Is_set() const noexcept {
        return _Myval.load(::std::memory_order_relaxed);
    }

    void _Sync_flag::_Clear() noexcept {
        _Myval.store(false, ::std::memory_order_relaxed);
    }

    void _Sync_flag::_Set() noexcept {
        _Myval.store(true, ::std::memory_order_relaxed);
    }
} // namespace mjx