// task.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <cstdio>
#include <dbmgr/task.hpp>
#include <dbmgr/database.hpp>
#include <mjmem/object_allocator.hpp>

namespace mjx {
    help::help() noexcept {}

    help::~help() noexcept {}

    bool help::execute() {
        ::puts(
            "Usage:\n"
            "    --lock=name - Locks an application.\n"
            "    --unlock=name - Unlocks an application.\n"
            "    --unlock-all - Unlocks all locked applications.\n"
            "    --status=name - Checks if an application is locked."
        );
        return true;
    }

    const char* help::error() const noexcept {
        return nullptr; // error never occurs
    }

    lock::lock(const unicode_string_view _Target) noexcept : _Mytarget(_Target), _Myerror(nullptr) {}

    lock::~lock() noexcept {}

    bool lock::execute() {
        database& _Db = database::current();
        if (_Db.has_entry(_Mytarget)) {
            _Myerror = "The application is already locked.";
            return false;
        }

        if (_Db.append(_Mytarget)) {
            return true;
        } else {
            _Myerror = "Failed to lock the application, try again.";
            return false;
        }
    }

    const char* lock::error() const noexcept {
        return _Myerror;
    }

    unlock::unlock(const unicode_string_view _Target) noexcept : _Mytarget(_Target), _Myerror(nullptr) {}

    unlock::~unlock() noexcept {}

    bool unlock::execute() {
        database& _Db = database::current();
        if (!_Db.has_entry(_Mytarget)) {
            _Myerror = "The application is not locked.";
            return false;
        }

        if (_Db.erase(_Mytarget)) {
            return true;
        } else {
            _Myerror = "Failed to unlock the application, try again.";
            return false;
        }
    }

    const char* unlock::error() const noexcept {
        return _Myerror;
    }

    unlock_all::unlock_all() noexcept {}

    unlock_all::~unlock_all() noexcept {}

    bool unlock_all::execute() {
        database::current().clear();
        return true;
    }

    const char* unlock_all::error() const noexcept {
        return nullptr; // error never occurs
    }

    status::status(const unicode_string_view _Target) noexcept : _Mytarget(_Target) {}

    status::~status() noexcept {}

    bool status::execute() {
        if (database::current().has_entry(_Mytarget)) {
            ::puts("[STATUS]: The application is locked.");
        } else {
            ::puts("[STATUS]: The application is not locked.");
        }

        return true;
    }

    const char* status::error() const noexcept {
        return nullptr; // error never occurs
    }

    [[nodiscard]] task* make_task(const wchar_t* const _Arg) {
        const unicode_string_view _As_view(_Arg);
        const size_t _Eq_pos = _As_view.find(L'=');
        if (_Eq_pos != unicode_string_view::npos) { // command with target
            if (_Eq_pos == _As_view.size() - 1) { // no target provided
                return nullptr;
            }

            const unicode_string_view _Target = _Arg + _Eq_pos + 1;
            if (_As_view.contains(L"--lock")) {
                return ::mjx::create_object<lock>(_Target);
            } else if (_As_view.contains(L"--unlock")) {
                return ::mjx::create_object<unlock>(_Target);
            } else if (_As_view.contains(L"--status")) {
                return ::mjx::create_object<status>(_Target);
            } else { // unknown command
                return nullptr;
            }
        } else { // generic command
            if (_As_view == L"--help") {
                return ::mjx::create_object<help>();
            } else if (_As_view == L"--unlock-all") {
                return ::mjx::create_object<unlock_all>();
            } else { // unknown command
                return nullptr;
            }
        }
    }

    task_executor::task_executor() noexcept : _Mytask(nullptr) {}

    task_executor::~task_executor() noexcept {}

    void task_executor::bind_task(task* const _Task) noexcept {
        _Mytask.reset(_Task);
    }

    bool task_executor::execute() {
        return _Mytask ? _Mytask->execute() : false;
    }

    const char* task_executor::error() const noexcept {
        return _Mytask ? _Mytask->error() : nullptr;
    }

    task_queue::task_queue() noexcept : _Mytasks(), _Myerror(nullptr) {}

    task_queue::~task_queue() noexcept {
        _Clear();
    }

    void task_queue::_Clear() noexcept {
        for (task*& _Task : _Mytasks) {
            ::mjx::delete_object(_Task);
            _Task = nullptr;
        }

        _Mytasks.clear();
    }

    [[nodiscard]] task* task_queue::_Pop() noexcept {
        task* const _Task = _Mytasks.front();
        _Mytasks.erase(_Mytasks.begin());
        return _Task;
    }

    void task_queue::push(task* const _Task) {
        _Mytasks.push_back(_Task);
    }

    bool task_queue::execute() {
        task_executor _Executor;
        while (!_Mytasks.empty()) {
            _Executor.bind_task(_Pop());
            if (!_Executor.execute()) {
                _Myerror = _Executor.error();
                return false;
            }
        }

        return true;
    }

    const char* task_queue::error() const noexcept {
        return _Myerror;
    }
} // namespace mjx