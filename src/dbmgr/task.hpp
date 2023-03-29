// task.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _DBMGR_TASK_HPP_
#define _DBMGR_TASK_HPP_
#include <string_view>
#include <vector>

namespace dbmgr {
    using ::std::vector;
    using ::std::wstring_view;

    class __declspec(novtable) task { // base class for all tasks
    public:
        virtual bool execute() noexcept            = 0;
        virtual const char* error() const noexcept = 0;
    };

    class help : public task {
    public:
        help() noexcept;
        ~help() noexcept;

        bool execute() noexcept override;
        const char* error() const noexcept override;
    };

    class lock : public task {
    public:
        explicit lock(const wstring_view _Target) noexcept;
        ~lock() noexcept;

        bool execute() noexcept override;
        const char* error() const noexcept override;

    private:
        const wstring_view _Mytarget;
        const char* _Myerror;
    };

    class unlock : public task {
    public:
        explicit unlock(const wstring_view _Target) noexcept;
        ~unlock() noexcept;

        bool execute() noexcept override;
        const char* error() const noexcept override;

    private:
        const wstring_view _Mytarget;
        const char* _Myerror;
    };

    class unlock_all : public task {
    public:
        unlock_all() noexcept;
        ~unlock_all() noexcept;

        bool execute() noexcept override;
        const char* error() const noexcept override;
    };

    class status : public task {
    public:
        explicit status(const wstring_view _Target) noexcept;
        ~status() noexcept;

        bool execute() noexcept override;
        const char* error() const noexcept override;

    private:
        const wstring_view _Mytarget;
    };

    [[nodiscard]] task* make_task(const wchar_t* const _Arg) noexcept;

    class task_invoker { // manages task lifetime and execution
    public:
        task_invoker() noexcept;
        ~task_invoker() noexcept;

        void acquire_task(task* const _Task) noexcept;
        bool execute() noexcept;
        const char* error() const noexcept;

    private:
        void _Release_task() noexcept;

        task* _Mytask;
    };

    class task_queue {
    public:
        task_queue() noexcept;
        ~task_queue() noexcept;

        task_queue(const task_queue&) = delete;
        task_queue& operator=(const task_queue&) = delete;

        void push(task* const _Task) noexcept;
        bool execute() noexcept;
        const char* error() const noexcept;

    private:
        void _Clear() noexcept;
        [[nodiscard]] task* _Pop() noexcept;

        vector<task*> _Mytasks;
        const char* _Myerror;
    };
} // namespace dbmgr

#endif // _DBMGR_TASK_HPP_