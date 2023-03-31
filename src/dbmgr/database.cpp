// database.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <dbmgr/database.hpp>
#include <cstring>
#include <type_traits>
#include <Windows.h>

namespace dbmgr {
    path _Get_database_directory_path() {
        wchar_t _Buf[1024];
        const size_t _Buf_size = static_cast<size_t>(::GetModuleFileNameW(nullptr, _Buf, 1024));
        _Buf[_Buf_size]        = L'\0';
        return path{wstring_view{_Buf, _Buf_size}}.remove_filename();
    }

    path _Get_database_file_path() {
        wchar_t _Buf[1024];
        const size_t _Buf_size = static_cast<size_t>(::GetModuleFileNameW(nullptr, _Buf, 1024));
        _Buf[_Buf_size]        = L'\0';
        return path{wstring_view{_Buf, _Buf_size}}.remove_filename() / LR"(apps.db)";
    }

    [[nodiscard]] void* _Open_file(const path& _Target) {
        return ::CreateFileW(_Target.c_str(), GENERIC_READ | GENERIC_WRITE,
            0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    }

    size_t _File_size(void* const _Handle) noexcept {
        LARGE_INTEGER _Large;
        ::GetFileSizeEx(_Handle, ::std::addressof(_Large));
        return static_cast<size_t>(_Large.QuadPart);
    }

    void _Clear_file(void* const _Handle) noexcept {
        ::SetFilePointer(_Handle, 0, nullptr, FILE_BEGIN);
        ::SetEndOfFile(_Handle);
    }

    void _Read_file(
        void* const _Handle, unsigned char* const _Buf, const size_t _Buf_size) noexcept {
#ifdef _M_X64
        (void) ::ReadFile(_Handle, _Buf, static_cast<uint32_t>(_Buf_size), nullptr, nullptr);
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86 vvv
        (void) ::ReadFile(_Handle, _Buf, _Buf_size, nullptr, nullptr);
#endif // _M_X64
    }

    void _Write_file(
        void* const _Handle, const unsigned char* const _Str, const size_t _Size) noexcept {
#ifdef _M_X64
        ::WriteFile(_Handle, _Str, static_cast<uint32_t>(_Size), nullptr, nullptr);
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86 vvv
        ::WriteFile(_Handle, _Str, _Size, nullptr, nullptr);
#endif // _M_X64
    }

    _Database_file::_Database_file() noexcept
        : _Myhandle(_Open_file(database_location::current().file())) {}

    _Database_file::~_Database_file() noexcept {
        if (_Myhandle) {
            ::CloseHandle(_Myhandle);
            _Myhandle = nullptr;
        }
    }

    bool _Database_file::_Good() const noexcept {
        return _Myhandle != nullptr;
    }

    void _Database_file::_Clear() noexcept {
        _Clear_file(_Myhandle);
    }

    size_t _Database_file::_Size() noexcept {
        return _File_size(_Myhandle);
    }

    void _Database_file::_Read(unsigned char* const _Buf, const size_t _Buf_size) noexcept {
        _Read_file(_Myhandle, _Buf, _Buf_size);
    }

    void _Database_file::_Write(const unsigned char* const _Str, const size_t _Size) noexcept {
        _Write_file(_Myhandle, _Str, _Size);
    }

    database_location::database_location()
        : _Mydir(_Get_database_directory_path()), _Myfile(_Get_database_file_path()) {}

    database_location::~database_location() noexcept {}

    database_location& database_location::current() noexcept {
        static database_location _Location;
        return _Location;
    }

    const path& database_location::directory() const {
        return _Mydir;
    }

    const path& database_location::file() const {
        return _Myfile;
    }

    database::database() noexcept : _Myentries(), _Mysave(false) {
        _Load_database();
    }

    database::~database() noexcept {
        if (_Mysave) {
            _Save();
        }
    }

    database::entry_type database::_Make_entry(const wstring_view _Name) noexcept {
        const uint32_t _Checksum = ::dbmgr::compute_checksum(_Name);
        entry_type _Result;
        ::memcpy(_Result.data(), &_Checksum, sizeof(uint32_t));
        return _Result;
    }

    size_t database::_Find_entry(const entry_type& _Entry) const noexcept {
        const auto& _Iter = ::std::find(_Myentries.begin(), _Myentries.end(), _Entry);
        if (_Iter != _Myentries.end()) {
            return ::std::distance(_Myentries.begin(), _Iter);
        } else {
            return _Npos;
        }
    }

    void database::_Load_database() noexcept {
        _Database_file _File;
        if (_File._Good()) {
            const size_t _Size = _File._Size();
            if (_Size <= 1024) { // use stack-based buffer
                unsigned char _Buf[1024];
                _File._Read(_Buf, _Size);
                _Extract_entries_from_bytes(_Buf, _Size);
            } else { // use heap-based buffer
                unique_ptr<unsigned char[]> _Buf(new unsigned char[_Size]);
                _File._Read(_Buf.get(), _Size);
                _Extract_entries_from_bytes(_Buf.get(), _Size);
            }
        }
    }

    void database::_Extract_entries_from_bytes(
        const unsigned char* const _Bytes, size_t _Count) noexcept {
        static constexpr size_t _Bytes_per_entry = 4;
        const size_t _Remainder                  = _Count % _Bytes_per_entry;
        if (_Remainder != 0) {
            _Count -= _Remainder;
        }

        entry_type _Entry;
        for (size_t _Off = 0; _Off < _Count; _Off += _Bytes_per_entry) {
            ::memcpy(_Entry.data(), _Bytes + _Off, _Bytes_per_entry);
            _Myentries.push_back(_Entry);
        }
    }

    void database::_Save() noexcept {
        _Database_file _File;
        if (_File._Good()) {
            _File._Clear();
            for (const entry_type& _Entry : _Myentries) {
                _File._Write(_Entry.data(), _Entry.size());
            }
        }
    }

    database& database::current() noexcept {
        static database _Db;
        return _Db;
    }

    size_t database::entry_count() const noexcept {
        return _Myentries.size();
    }

    bool database::has_entry(const wstring_view _Name) const noexcept {
        return ::std::find(
            _Myentries.begin(), _Myentries.end(), _Make_entry(_Name)) != _Myentries.end();
    }

    const vector<database::entry_type>& database::get_entries() const noexcept {
        return _Myentries;
    }

    void database::clear() noexcept {
        if (!_Myentries.empty()) {
            _Myentries.clear();
            _Mysave = true; // save changes
        }
    }

    [[nodiscard]] bool database::append(const wstring_view _Name) {
        const entry_type& _Entry = _Make_entry(_Name);
        if (_Find_entry(_Entry) == _Npos) {
            _Myentries.push_back(_Entry);
            _Mysave = true; // save changes
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool database::erase(const wstring_view _Name) noexcept {
        const size_t _Off = _Find_entry(_Make_entry(_Name));
        if (_Off != _Npos) {
            _Myentries.erase(_Myentries.begin() + _Off);
            _Mysave = true; // save changes
            return true;
        } else {
            return false;
        }
    }

    void database::reload() noexcept {
        _Myentries.clear();
        _Mysave = false; // reset changes
        _Load_database();
    }
} // namespace dbmgr