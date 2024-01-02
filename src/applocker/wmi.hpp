// wmi.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _APPLOCKER_WMI_HPP_
#define _APPLOCKER_WMI_HPP_
#include <applocker/event_sink.hpp>
#include <WbemCli.h>

namespace mjx {
    class _Com_instance {
    public:
        _Com_instance() noexcept;
        ~_Com_instance() noexcept;

        // checks if the COM instance is valid
        bool _Valid() const noexcept;

    private:
        // initializes COM
        static bool _Init() noexcept;
        
        // initializes COM security
        static bool _Init_security() noexcept;

        bool _Myinst; // true if the COM instance is initialized
        bool _Mysec; // true if the COM security is initialized
    };

    template <class _Ty>
    constexpr _Ty* _Create_instance(const CLSID& _Clsid, const CLSCTX _Ctx, const IID& _Iid) noexcept {
        _Ty* _Result = nullptr;
        return ::CoCreateInstance(
            _Clsid, nullptr, _Ctx, _Iid, reinterpret_cast<void**>(&_Result)) == S_OK ? _Result : nullptr;
    }

    template <class _Ty>
    class _Com_ptr { // wrapper for COM interfaces
    public:
        _Com_ptr() noexcept : _Myptr(nullptr) {}

        ~_Com_ptr() noexcept {
            if (_Myptr) {
                _Myptr->Release();
                _Myptr = nullptr;
            }
        }

        _Com_ptr& operator=(_Ty* const _New_ptr) noexcept {
            if (_Myptr) {
                _Myptr->Release();
            }

            _Myptr = _New_ptr;
            return *this;
        }

        _Ty* operator->() noexcept {
            return _Myptr;
        }

        const _Ty* operator->() const noexcept {
            return _Myptr;
        }

        bool _Valid() const noexcept {
            return _Myptr != nullptr;
        }

        _Ty* _Get() noexcept {
            return _Myptr;
        }

        const _Ty* _Get() const noexcept {
            return _Myptr;
        }

        _Ty** _Address() noexcept {
            return &_Myptr;
        }

        void** _Raw_address() noexcept {
            return reinterpret_cast<void**>(&_Myptr);
        }

        _Ty* _Exchange(const _Ty* const _New_ptr) noexcept {
            _Ty* const _Old_ptr = _Myptr;
            _Myptr              = _New_ptr;
            return _Old_ptr;
        }

    private:
        _Ty* _Myptr;
    };

    class _Wmi_session { // manages WMI session
    public:
        _Com_instance _Inst;
        _Com_ptr<IWbemLocator> _Locator;
        _Com_ptr<IWbemServices> _Services;
        _Com_ptr<IUnsecuredApartment> _Apartment;
        _Com_ptr<_Event_sink> _Sink;
        _Com_ptr<IUnknown> _Stub;
        _Com_ptr<IWbemObjectSink> _Stub_sink;
    
        _Wmi_session() noexcept;
        ~_Wmi_session() noexcept;

        // connects to the WMI
        [[nodiscard]] bool _Connect();

        // terminates the WMI session
        void _Terminate() noexcept;
    
    private:
        // obtains the WMI locator
        bool _Obtain_wmi_locator() noexcept;

        // connects to the WMI
        bool _Connect_to_wmi() noexcept;

        // sets security levels on the proxy
        bool _Set_proxy_security() noexcept;

        // configures the way the events are received
        bool _Configure_event_reception();

        // sends notification query to WMI
        bool _Send_notification_query() noexcept;
    };
} // namespace mjx

#endif // _APPLOCKER_WMI_HPP_