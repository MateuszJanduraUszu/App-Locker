// database.cpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <cstring>
#include <dbmgr/database.hpp>
#include <memory>
#include <mjfs/file_stream.hpp>
#include <type_traits>
#ifdef _APPLOCKER_SERVICE
#include <Windows.h>
#endif // _APPLOCKER_SERVICE

namespace dbmgr {
    database_location::database_location()
        : _Mydir(_Get_directory_path()), _Myfile(_Mydir / L"apps.db") {}

    database_location::~database_location() noexcept {}

    ::mjfs::path database_location::_Get_directory_path() {
        // Note: This function is designed to handle both console applications and services.
        //       In a console application, it is sufficient to call mjfs::current_path() to retrive the
        //       current working directory. However, services start in a different directory by default.
        //       To accommodate this difference, we use a conditional approach: _APPLOCKER_SERVICE is
        //       defined during the compilation of applocker.exe only. In the case of a service,
        //       we employ the GetModuleFileNameW() function to obtain the current directory reliably.
#ifdef _APPLOCKER_SERVICE
        size_t _Buf_size = 260; // MAX_PATH (in fact MAX_PATH + 1, because std::wstring includes null-terminator)
        ::mjfs::path::string_type _Buf(_Buf_size, L'\0');
        size_t _Copied; // number of elements copied into the buffer
        unsigned long _Error; // last error
        for (;;) {
#ifdef _M_X64
            _Copied = static_cast<size_t>(
                ::GetModuleFileNameW(nullptr, _Buf.data(), static_cast<unsigned long>(_Buf_size + 1)));
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86
            _Copied = ::GetModuleFileNameW(nullptr, _Buf.data(), _Buf_size + 1);
#endif // _M_X64
            _Error  = ::GetLastError();
            if (_Error == ERROR_SUCCESS) { // buffer was sufficient, break
                break;
            } else if (_Error == ERROR_INSUFFICIENT_BUFFER) { // increase the buffer and try again
                _Buf_size *= 2;
                _Buf.resize(_Buf_size);
            } else { // an error occured, break
                return ::mjfs::path{};
            }
        }

        if (_Copied < _Buf_size) { // buffer too large, decrease it
            _Buf.resize(_Copied);
        }

        return ::mjfs::path{::std::move(_Buf)}.remove_filename();
#else // ^^^ _APPLOCKER_SERVICE ^^^ / vvv !_APPLOCKER_SERVICE vvv
        return ::mjfs::current_path();
#endif // _APPLOCKER_SERVICE
    }

    database_location& database_location::current() noexcept {
        static database_location _Location;
        return _Location;
    }

    const ::mjfs::path& database_location::directory() const noexcept {
        return _Mydir;
    }

    const ::mjfs::path& database_location::file() const noexcept {
        return _Myfile;
    }

    database_entry::database_entry(const checksum_t _Val) noexcept : _Myval(_Val) {}

    bool database_entry::operator==(const database_entry& _Other) const noexcept {
        return _Myval == _Other._Myval;
    }

    const checksum_t database_entry::to_integer() const noexcept {
        return _Myval;
    }

    database_entry::byte_sequence database_entry::to_bytes() const noexcept {
        byte_sequence _Result;
        ::memcpy(_Result.data(), &_Myval, _Result.size());
        return _Result;
    }

    database::database() noexcept : _Myentries(), _Mysave(false) {
        _Load_database();
    }

    database::~database() noexcept {
        if (_Mysave) {
            _Save();
        }
    }

    database_entry database::_Make_entry(const ::std::wstring_view _Name) noexcept {
        return database_entry{::dbmgr::compute_checksum(_Name)};
    }

    size_t database::_Find_entry(const database_entry& _Entry) const noexcept {
        for (size_t _Idx = 0; _Idx < _Myentries.size(); ++_Idx) {
            if (_Myentries[_Idx] == _Entry) {
                return _Idx;
            }
        }

        return _Npos;
    }

    void database::_Load_database() {
        ::mjfs::file _File(
            database_location::current().file(), ::mjfs::file_access::read, ::mjfs::file_share::read);
        ::mjfs::file_stream _Stream(_File);
        if (_Stream.is_open()) {
            static constexpr size_t _Stack_buf_size = 1024;
#ifdef _M_X64
            const size_t _Size                      = _File.size();
#else // ^^^ _M_X64 ^^^ / vvv _M_IX86 vvv
            const size_t _Size                      = static_cast<size_t>(_File.size());
#endif // _M_X64
            if (_Size <= _Stack_buf_size) { // use stack buffer
                unsigned char _Buf[_Stack_buf_size];
                if (_Stream.read(_Buf, _Size) == _Size) {
                    _Extract_entries_from_bytes(_Buf, _Size);
                }
            } else { // use heap buffer
                auto _Buf = ::std::make_unique<unsigned char[]>(_Size);
                if (_Stream.read(_Buf.get(), _Size) == _Size) {
                    _Extract_entries_from_bytes(_Buf.get(), _Size);
                }
            }
        }
    }

    void database::_Extract_entries_from_bytes(const unsigned char* const _Bytes, size_t _Count) {
        static constexpr size_t _Bytes_per_entry = 4; // 4-byte entry
        const size_t _Remainder                  = _Count % _Bytes_per_entry;
        if (_Remainder != 0) { // skip incomplete entries
            _Count -= _Remainder;
        }

        checksum_t _Val;
        for (size_t _Off = 0; _Off < _Count; _Off += _Bytes_per_entry) {
            ::memcpy(&_Val, _Bytes + _Off, _Bytes_per_entry);
            _Myentries.push_back(database_entry{_Val});
        }
    }

    void database::_Save() noexcept {
        ::mjfs::file _File(database_location::current().file(), ::mjfs::file_access::write);
        ::mjfs::file_stream _Stream(_File);
        if (_Stream.is_open()) {
            if (_File.resize(0)) { // must be empty
                database_entry::byte_sequence _Bytes;
                for (const database_entry& _Entry : _Myentries) {
                    _Bytes = _Entry.to_bytes();
                    if (!_Stream.write(_Bytes.data(), _Bytes.size())) { // something went wrong, break
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
        return _Find_entry(_Make_entry(_Name)) != _Npos;
    }

    const ::std::vector<database_entry>& database::get_entries() const noexcept {
        return _Myentries;
    }

    void database::clear() noexcept {
        if (!_Myentries.empty()) {
            _Myentries.clear();
            _Mysave = true; // save changes
        }
    }

    [[nodiscard]] bool database::append(const ::std::wstring_view _Name) {
        const database_entry& _Entry = _Make_entry(_Name);
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