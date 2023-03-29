// main.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <dbmgr/task.hpp>

inline int _Entry_point(int _Count, wchar_t** _Args) {
    if (_Count == 1) { // no tasks
        return 0;
    }

    ::dbmgr::task_queue _Queue;
    for (int _Idx = 1; _Idx < _Count; ++_Idx) {
        ::dbmgr::task* const _Task = ::dbmgr::make_task(_Args[_Idx]);
        if (!_Task) {
            ::puts("[ERROR]: No task associated with the given command.");
            return -1;
        }

        _Queue.push(_Task);
    }

    if (_Queue.execute()) {
        return 0;
    } else {
        ::printf("[ERROR]: %s\n", _Queue.error());
        return -1;
    }
}

int wmain(int _Count, wchar_t** _Args) {
    try {
        return _Entry_point(_Count, _Args);
    } catch (...) {
        ::puts("[ERROR]: Generic error.");
        return -1;
    }
}