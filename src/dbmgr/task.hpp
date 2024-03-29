// task.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _DBMGR_TASK_HPP_
#define _DBMGR_TASK_HPP_
#include <mjmem/smart_pointer.hpp>
#include <mjstr/string_view.hpp>
#include <vector>

namespace mjx {
    class __declspec(novtable) task { // base class for all tasks
    public:
        virtual bool execute()                     = 0;
        virtual const char* error() const noexcept = 0;
    };

    class help : public task {
    public:
        help() noexcept;
        ~help() noexcept;

        // shows the application usage
        bool execute() override;

        // returns an error (never occurs)
        const char* error() const noexcept override;
    };

    class lock : public task {
    public:
        explicit lock(const unicode_string_view _Target) noexcept;
        ~lock() noexcept;

        // locks the specified application
        bool execute() override;

        // returns an error
        const char* error() const noexcept override;

    private:
        unicode_string_view _Mytarget;
        const char* _Myerror;
    };

    class unlock : public task {
    public:
        explicit unlock(const unicode_string_view _Target) noexcept;
        ~unlock() noexcept;

        // unlocks the specified application
        bool execute() override;
        
        // returns an error
        const char* error() const noexcept override;

    private:
        unicode_string_view _Mytarget;
        const char* _Myerror;
    };

    class unlock_all : public task {
    public:
        unlock_all() noexcept;
        ~unlock_all() noexcept;

        // unlocks all locked applications
        bool execute() override;

        // returns an error
        const char* error() const noexcept override;
    };

    class status : public task {
    public:
        explicit status(const unicode_string_view _Target) noexcept;
        ~status() noexcept;

        // checks if the specified application is locked
        bool execute() override;
        
        // returns an error (never occurs)
        const char* error() const noexcept override;

    private:
        unicode_string_view _Mytarget;
    };

    [[nodiscard]] task* make_task(const wchar_t* const _Arg);

    class task_executor { // manages task lifetime and execution
    public:
        task_executor() noexcept;
        ~task_executor() noexcept;

        // binds a new task
        void bind_task(task* const _Task) noexcept;
        
        // executes the binded task
        bool execute();

        // returns an error
        const char* error() const noexcept;

    private:
        unique_smart_ptr<task> _Mytask;
    };

    class task_queue {
    public:
        task_queue() noexcept;
        ~task_queue() noexcept;

        task_queue(const task_queue&) = delete;
        task_queue& operator=(const task_queue&) = delete;

        // adds a new task to the queue
        void push(task* const _Task);
        
        // executes all tasks stored in the queue
        bool execute();

        // returns an error
        const char* error() const noexcept;

    private:
        // clears the queue
        void _Clear() noexcept;

        // removes and returns the task from the top of the queue
        [[nodiscard]] task* _Pop() noexcept;

        ::std::vector<task*> _Mytasks;
        const char* _Myerror;
    };
} // namespace mjx

#endif // _DBMGR_TASK_HPP_