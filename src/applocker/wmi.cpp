// wmi.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <applocker/service_caches.hpp>
#include <applocker/wmi.hpp>
#include <combaseapi.h>
#include <new>

namespace applocker {
    _Com_instance::_Com_instance() noexcept
        : _Myinst(_Init()), _Mysec(_Myinst ? _Init_security() : false) {}

    _Com_instance::~_Com_instance() noexcept {
        if (_Myinst) {
            ::CoUninitialize();
            _Myinst = false;
        }
    }

    bool _Com_instance::_Init() noexcept {
        return ::CoInitializeEx(nullptr, COINIT_MULTITHREADED) == S_OK;
    }

    bool _Com_instance::_Init_security() noexcept {
        return ::CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT,
            RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr) == S_OK;
    }

    bool _Com_instance::_Valid() const noexcept {
        return _Myinst && _Mysec;
    }

    _Wmi_session::_Wmi_session() noexcept
        : _Inst(), _Locator(), _Services(), _Apartment(), _Sink(), _Stub(), _Stub_sink() {}

    _Wmi_session::~_Wmi_session() noexcept {}

    bool _Wmi_session::_Obtain_wmi_locator() noexcept {
        _Locator = _Create_instance<IWbemLocator>(
            ::CLSID_WbemLocator, CLSCTX_INPROC_SERVER, ::IID_IWbemLocator);
        return _Locator._Valid();
    }

    bool _Wmi_session::_Connect_to_wmi() noexcept {
        wchar_t _Namespace[] = LR"(ROOT\CIMV2)";
        return _Locator->ConnectServer(
            _Namespace, nullptr, nullptr, nullptr, 0, nullptr, nullptr, _Services._Address()) >= 0;
    }

    bool _Wmi_session::_Set_proxy_security() noexcept {
        return ::CoSetProxyBlanket(_Services._Get(), RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
            RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE) == S_OK;
    }

    bool _Wmi_session::_Configure_event_reception() noexcept {
        _Apartment = _Create_instance<IUnsecuredApartment>(
            ::CLSID_UnsecuredApartment, CLSCTX_LOCAL_SERVER, ::IID_IUnsecuredApartment);
        if (!_Apartment._Valid()) {
            return false;
        }

        _Sink = new (::std::nothrow) _Event_sink(_Service_shared_cache::_Get()._Task_event);
        if (!_Sink._Valid()) {
            return false;
        }

        if (_Apartment->CreateObjectStub(_Sink._Get(), _Stub._Address()) < 0) {
            return false;
        }

        return _Stub->QueryInterface(::IID_IWbemObjectSink, _Stub_sink._Raw_address()) >= 0;
    }

    bool _Wmi_session::_Send_notification_query() noexcept {
        wchar_t _Language[] = L"WQL";
        wchar_t _Query[]    = L"SELECT * FROM __InstanceCreationEvent WITHIN 1 "
                              L"WHERE TargetInstance ISA 'Win32_Process'";
        return _Services->ExecNotificationQueryAsync(_Language, _Query, WBEM_FLAG_SEND_STATUS,
            nullptr, _Stub_sink._Get()) >= 0;
    }

    [[nodiscard]] bool _Wmi_session::_Connect() noexcept {
        if (!_Inst._Valid()) {
            return false;
        }

        return _Obtain_wmi_locator() && _Connect_to_wmi() && _Set_proxy_security()
            && _Configure_event_reception() && _Send_notification_query();
    }

    void _Wmi_session::_Terminate() noexcept {
        _Services->CancelAsyncCall(_Stub_sink._Get());
    }
} // namespace applocker