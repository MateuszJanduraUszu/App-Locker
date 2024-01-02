// main.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <cstdio>
#include <dbmgr/task.hpp>
#include <mjmem/smart_pointer.hpp>

namespace mjx {
    inline int _Entry_point(int _Count, wchar_t** _Args) {
        if (_Count == 1) { // no tasks
            return 0;
        }

        task_queue _Queue;
        unique_smart_ptr<task> _Task;
        for (int _Idx = 1; _Idx < _Count; ++_Idx) {
            _Task.reset(make_task(_Args[_Idx]));
            if (!_Task) {
                ::puts("[ERROR]: No task associated with the given command.");
                return -1;
            }

            _Queue.push(_Task.get());
            _Task.release(); // task_queue::push() succeeded, release task ownership
        }

        if (_Queue.execute()) {
            return 0;
        } else {
            ::printf("[ERROR]: %s\n", _Queue.error());
            return -1;
        }
    }
} // namespace mjx

int wmain(int _Count, wchar_t** _Args) {
    try {
        return ::mjx::_Entry_point(_Count, _Args);
    } catch (...) {
        ::puts("[ERROR]: Unknown error.");
        return -1;
    }
}