// event_sink.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <applocker/event_sink.hpp>
#include <applocker/service_caches.hpp>
#include <dbmgr/checksum.hpp>
#include <type_traits>

namespace applocker {
    _Variant::_Variant() noexcept : _Mystorage() {
        ::VariantInit(::std::addressof(_Mystorage));
    }

    _Variant::~_Variant() noexcept {
        ::VariantClear(::std::addressof(_Mystorage));
    }

    VARIANT* _Variant::_Get() noexcept {
        return ::std::addressof(_Mystorage);
    }

    const VARIANT* _Variant::_Get() const noexcept {
        return ::std::addressof(_Mystorage);
    }

    _Event_sink::_Event_sink(waitable_event& _Event) noexcept : _Myrefs(1), _Myevent(_Event) {}

    _Event_sink::~_Event_sink() noexcept {}

    IWbemClassObject* _Event_sink::_Get_target_instance(
        IWbemClassObject* const _Obj, _Variant& _Val) noexcept {
        return _Obj->Get(L"TargetInstance", 0, _Val._Get(), nullptr, nullptr) == 0
            ? reinterpret_cast<IWbemClassObject*>(_Val._Get()->punkVal) : nullptr;
    }

    uint32_t _Event_sink::_Get_process_id(IWbemClassObject* const _Inst) noexcept {
        _Variant _Val;
        return _Inst->Get(L"ProcessId", 0, _Val._Get(), nullptr, nullptr) == 0
            ? _Val._Get()->uintVal : 0;
    }

    ::dbmgr::checksum_t _Event_sink::_Get_process_module_checksum(IWbemClassObject* const _Inst) noexcept {
        _Variant _Val;
        return _Inst->Get(L"Name", 0, _Val._Get(), nullptr, nullptr) == 0
            ? ::dbmgr::compute_checksum(_Val._Get()->bstrVal) : 0;
    }

    _Event_sink::_Ref_t __stdcall _Event_sink::AddRef() {
        return ::_InterlockedIncrement(&_Myrefs);
    }

    _Event_sink::_Ref_t __stdcall _Event_sink::Release() {
        const _Ref_t _Refs = ::_InterlockedDecrement(&_Myrefs);
        if (_Refs == 0) {
            delete this;
        }

        return _Refs;
    }

    long __stdcall _Event_sink::QueryInterface(const IID& _Id, void** _Obj) {
        if (_Id == IID_IUnknown || _Id == IID_IWbemObjectSink) {
            *_Obj = static_cast<IWbemObjectSink*>(this);
            AddRef();
            return WBEM_S_NO_ERROR;
        } else {
            return E_NOINTERFACE;
        }
    }

    long __stdcall _Event_sink::Indicate(long _Count, IWbemClassObject** _Objects) {
        ::std::vector<_Process_traits::_Basic_data> _Procs;
        IWbemClassObject* _Inst;
        _Process_traits::_Basic_data _Data;
        for (long _Idx = 0; _Idx < _Count; ++_Idx) {
            _Variant _Val;
            _Inst = _Get_target_instance(_Objects[_Idx], _Val);
            if (_Inst) {
                _Data._Id              = _Get_process_id(_Inst);
                _Data._Module_checksum = _Get_process_module_checksum(_Inst);
                _Procs.push_back(_Data);
            }
        }

        _Service_shared_cache::_Get()._New_procs.assign(_Procs);
        _Myevent.notify(); // notify that new processes have been created
        return WBEM_S_NO_ERROR;
    }

    long __stdcall _Event_sink::SetStatus(long, long, wchar_t*, IWbemClassObject*) {
        return WBEM_S_NO_ERROR;
    }
} // namespace applocker