// task.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <cstdio>
#include <new>
#include <dbmgr/task.hpp>
#include <dbmgr/database.hpp>

namespace dbmgr {
    help::help() noexcept {}

    help::~help() noexcept {}

    bool help::execute() noexcept {
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

    lock::lock(const ::std::wstring_view _Target) noexcept : _Mytarget(_Target), _Myerror(nullptr) {}

    lock::~lock() noexcept {}

    bool lock::execute() noexcept {
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

    unlock::unlock(const ::std::wstring_view _Target) noexcept : _Mytarget(_Target), _Myerror(nullptr) {}

    unlock::~unlock() noexcept {}

    bool unlock::execute() noexcept {
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

    bool unlock_all::execute() noexcept {
        database::current().clear();
        return true;
    }

    const char* unlock_all::error() const noexcept {
        return nullptr; // error never occurs
    }

    status::status(const ::std::wstring_view _Target) noexcept : _Mytarget(_Target) {}

    status::~status() noexcept {}

    bool status::execute() noexcept {
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

    [[nodiscard]] task* make_task(const wchar_t* const _Arg) noexcept {
        const ::std::wstring_view _As_view(_Arg);
        const size_t _Eq_pos = _As_view.find(L'=');
        if (_Eq_pos != ::std::wstring_view::npos) { // command with target
            if (_Eq_pos == _As_view.size() - 1) { // no target provided
                return nullptr;
            }

            const ::std::wstring_view _Target  = _Arg + _Eq_pos + 1;
            static constexpr size_t _Not_found = ::std::wstring_view::npos;
            if (_As_view.find(L"--lock") != _Not_found) {
                return new (::std::nothrow) lock(_Target);
            } else if (_As_view.find(L"--unlock") != _Not_found) {
                return new (::std::nothrow) unlock(_Target);
            } else if (_As_view.find(L"--status") != _Not_found) {
                return new (::std::nothrow) status(_Target);
            } else { // unknown command
                return nullptr;
            }
        } else { // generic command
            if (_As_view == L"--help") {
                return new (::std::nothrow) help();
            } else if (_As_view == L"--unlock-all") {
                return new (::std::nothrow) unlock_all();
            } else { // unknown command
                return nullptr;
            }
        }
    }

    task_executor::task_executor() noexcept : _Mytask(nullptr) {}

    task_executor::~task_executor() noexcept {
        _Release_task();
    }

    void task_executor::_Release_task() noexcept {
        if (_Mytask) {
            delete _Mytask;
            _Mytask = nullptr;
        }
    }

    void task_executor::bind_task(task* const _Task) noexcept {
        _Release_task();
        _Mytask = _Task;
    }

    bool task_executor::execute() noexcept {
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
            delete _Task;
            _Task = nullptr;
        }

        _Mytasks.clear();
    }

    [[nodiscard]] task* task_queue::_Pop() noexcept {
        task* const _Result = _Mytasks.front();
        _Mytasks.erase(_Mytasks.begin());
        return _Result;
    }

    void task_queue::push(task* const _Task) {
        _Mytasks.push_back(_Task);
    }

    bool task_queue::execute() noexcept {
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
} // namespace dbmgr