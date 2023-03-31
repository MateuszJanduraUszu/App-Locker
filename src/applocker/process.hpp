// process.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _APPLOCKER_PROCESS_HPP_
#define _APPLOCKER_PROCESS_HPP_
#include <dbmgr/database.hpp>
#include <cstdint>
#include <vector>

namespace applocker {
    using ::std::vector;

    extern vector<uint32_t> _Find_process_ids(
        const vector<::dbmgr::database::entry_type>& _Targets) noexcept;
    extern void _Terminate_process(const uint32_t _Id) noexcept;
} // namespace applocker

#endif // _APPLOCKER_PROCESS_HPP_