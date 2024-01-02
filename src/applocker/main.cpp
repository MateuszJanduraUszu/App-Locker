// main.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <applocker/service.hpp>

int main() {
    constexpr SERVICE_TABLE_ENTRYW _Table[] = {
        {const_cast<wchar_t*>(
            ::mjx::service_launcher::service_name), &::mjx::service_entry},
        {nullptr, nullptr} // null-entry (required)
    };
    return ::StartServiceCtrlDispatcherW(_Table) != 0 ? 0 : -1;
}