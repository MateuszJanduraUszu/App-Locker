// event_sink.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _APPLOCKER_EVENT_SINK_HPP_
#define _APPLOCKER_EVENT_SINK_HPP_
#include <applocker/process.hpp>
#include <applocker/waitable_event.hpp>
#include <guiddef.h>
#include <WbemIdl.h>

namespace applocker {
    class _Variant {
    public:
        _Variant() noexcept;
        ~_Variant() noexcept;

        // returns a pointer to the associated storage
        VARIANT* _Get() noexcept;

        // returns a non-mutable pointer to the associated storage
        const VARIANT* _Get() const noexcept;

    private:
        VARIANT _Mystorage;
    };

    class _Event_sink : public IWbemObjectSink {
    public:
        using _Ref_t = unsigned long;

        explicit _Event_sink(waitable_event& _Event) noexcept;
        ~_Event_sink() noexcept;

        // increments the reference count
        _Ref_t __stdcall AddRef() override;

        // decrements the reference count
        _Ref_t __stdcall Release() override;

        // queries another interface from this class
        long __stdcall QueryInterface(const IID& _Id, void** _Obj) override;

        // receives notification objects (notifies waiting thread)
        long __stdcall Indicate(long _Count, IWbemClassObject** _Objects) override;

        // changes the current status (does nothing)
        long __stdcall SetStatus(
            long _Flags, long _Result, wchar_t* _Param, IWbemClassObject* _Obj) override;

    private:
        // obtains the process instance
        static IWbemClassObject* _Get_target_instance(
            IWbemClassObject* const _Obj, _Variant& _Val) noexcept;

        // obtains the process ID from the target instance
        static uint32_t _Get_process_id(IWbemClassObject* const _Inst) noexcept;

        // obtains the process module checksum from the target instance
        static ::dbmgr::checksum_t _Get_process_module_checksum(IWbemClassObject* const _Inst) noexcept;

        _Ref_t _Myrefs;
        waitable_event& _Myevent;
    };
} // namespace applocker

#endif // _APPLOCKER_EVENT_SINK_HPP_