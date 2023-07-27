// database.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <dbmgr/database.hpp>
#include <cstring>
#include <memory>
#include <Windows.h>

namespace dbmgr {
    [[nodiscard]] _File_traits::_Handle_type _File_traits::_Open(const path& _Target) {
        return ::CreateFileW(_Target.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    }

    bool _File_traits::_Clear(const _Handle_type _Handle) noexcept {
        return ::SetFilePointer(_Handle, 0, nullptr, FILE_BEGIN) != INVALID_SET_FILE_POINTER
            && ::SetEndOfFile(_Handle) != 0;
    }

    size_t _File_traits::_Size(const _Handle_type _Handle) noexcept {
#ifdef _M_X64
        unsigned long _High = 0; // higher 32 bits
        return static_cast<size_t>(::GetFileSize(_Handle, &_High)) | (static_cast<size_t>(_High) << 32);
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86 vvv
        return static_cast<size_t>(::GetFileSize(_Handle, nullptr));
#endif // _M_X64
    }

    size_t _File_traits::_Read(
        const _Handle_type _Handle, unsigned char* const _Buf, const size_t _Size) noexcept {
        if (_Size == 0 || !_Buf) {
            return 0;
        }

        unsigned long _Read = 0;
#ifdef _M_X64
        return ::ReadFile(_Handle, _Buf, static_cast<unsigned long>(_Size), &_Read, nullptr) != 0
            ? static_cast<size_t>(_Read) : 0;
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86 vvv
        return ::ReadFile(_Handle, _Buf, _Size, &_Read, nullptr) != 0 ? _Read : 0;
#endif // _M_X64
    }

    bool _File_traits::_Write(
        const _Handle_type _Handle, const unsigned char* const _Data, const size_t _Size) noexcept {
        if (_Size == 0) {
            return true;
        }

        unsigned long _Written     = 0;
#ifdef _M_X64
        const unsigned long _USize = static_cast<unsigned long>(_Size);
        return ::WriteFile(_Handle, _Data, _USize, &_Written, nullptr) != 0 && _Written == _USize;
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86 vvv
        return ::WriteFile(_Handle, _Data, _Size, &_Written, nullptr) != 0 && _Written == _Size;
#endif // _M_X64
    }

    _Database_file::_Database_file() noexcept
        : _Myhandle(_File_traits::_Open(database_location::current().file())) {}

    _Database_file::~_Database_file() noexcept {
        if (_Myhandle) {
            ::CloseHandle(_Myhandle);
            _Myhandle = nullptr;
        }
    }

    bool _Database_file::_Good() const noexcept {
        return _Myhandle != nullptr;
    }

    bool _Database_file::_Clear() noexcept {
        return _File_traits::_Clear(_Myhandle);
    }

    size_t _Database_file::_Size() noexcept {
        return _File_traits::_Size(_Myhandle);
    }

    size_t _Database_file::_Read(unsigned char* const _Buf, const size_t _Size) noexcept {
        return _File_traits::_Read(_Myhandle, _Buf, _Size);
    }

    bool _Database_file::_Write(const unsigned char* const _Data, const size_t _Size) noexcept {
        return _File_traits::_Write(_Myhandle, _Data, _Size);
    }

    database_location::database_location()
        : _Mydir(_Get_directory_path()), _Myfile(_Mydir / L"apps.db") {}

    database_location::~database_location() noexcept {}

    path database_location::_Get_directory_path() {
        static constexpr unsigned long _Buf_size = 1024;
        wchar_t _Buf[_Buf_size]                  = {L'\0'};
        const size_t _Read                       = static_cast<size_t>(
            ::GetModuleFileNameW(nullptr, _Buf, _Buf_size));
        return path{_Buf, _Buf + _Read}.remove_filename();
    }

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

    database::entry_type database::_Make_entry(const ::std::wstring_view _Name) noexcept {
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
                if (_File._Read(_Buf, _Size) == _Size) {
                    _Extract_entries_from_bytes(_Buf, _Size);
                }
            } else { // use heap-based buffer
                ::std::unique_ptr<unsigned char[]> _Buf(new unsigned char[_Size]);
                if (_File._Read(_Buf.get(), _Size) == _Size) {
                    _Extract_entries_from_bytes(_Buf.get(), _Size);
                }
            }
        }
    }

    void database::_Extract_entries_from_bytes(
        const unsigned char* const _Bytes, size_t _Count) noexcept {
        static constexpr size_t _Bytes_per_entry = 4;
        const size_t _Remainder                  = _Count % _Bytes_per_entry;
        if (_Remainder != 0) { // skip incomplete entries
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
            if (_File._Clear()) { // file must be empty
                for (const entry_type& _Entry : _Myentries) {
                    if (!_File._Write(_Entry.data(), _Entry.size())) { // something went wrong, break
                        break;
                    }
                }
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

    bool database::has_entry(const ::std::wstring_view _Name) const noexcept {
        return ::std::find(_Myentries.begin(), _Myentries.end(), _Make_entry(_Name)) != _Myentries.end();
    }

    const ::std::vector<database::entry_type>& database::get_entries() const noexcept {
        return _Myentries;
    }

    void database::clear() noexcept {
        if (!_Myentries.empty()) {
            _Myentries.clear();
            _Mysave = true; // save changes
        }
    }

    [[nodiscard]] bool database::append(const ::std::wstring_view _Name) {
        const entry_type& _Entry = _Make_entry(_Name);
        if (_Find_entry(_Entry) == _Npos) {
            _Myentries.push_back(_Entry);
            _Mysave = true; // save changes
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool database::erase(const ::std::wstring_view _Name) noexcept {
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