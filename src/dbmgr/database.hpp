// database.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _DBMGR_DATABASE_HPP_
#define _DBMGR_DATABASE_HPP_
#include <dbmgr/checksum.hpp>
#include <array>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string_view>
#include <vector>

namespace dbmgr {
    using ::std::array;
    using ::std::unique_ptr;
    using ::std::vector;
    using ::std::wstring_view;
    using ::std::filesystem::path;

    extern path _Get_database_directory_path();
    extern path _Get_database_file_path();
    extern [[nodiscard]] void* _Open_file(const path& _Target);
    extern size_t _File_size(void* const _Handle) noexcept;
    extern void _Clear_file(void* const _Handle) noexcept;
    extern void _Read_file(
        void* const _Handle, unsigned char* const _Buf, const size_t _Buf_size) noexcept;
    extern void _Write_file(
        void* const _Handle, const unsigned char* const _Str, const size_t _Size) noexcept;

    class _Database_file {
    public:
        _Database_file() noexcept;
        ~_Database_file() noexcept;

        bool _Good() const noexcept;
        void _Clear() noexcept;
        size_t _Size() noexcept;
        void _Read(unsigned char* const _Buf, const size_t _Buf_size) noexcept;
        void _Write(const unsigned char* const _Str, const size_t _Size) noexcept;

    private:
        void* _Myhandle;
    };

    class database_location {
    public:
        ~database_location() noexcept;

        database_location(const database_location&) = delete;
        database_location& operator=(const database_location&) = delete;

        static database_location& current() noexcept;
        const path& directory() const;
        const path& file() const;
    
    private:
        database_location();

        path _Mydir;
        path _Myfile;
    };

    class database {
    public:
        using entry_type = array<unsigned char, 4>; // 4-byte entry

        ~database() noexcept;

        database(const database&) = delete;
        database& operator=(const database&) = delete;
        
        static database& current() noexcept;
        size_t entry_count() const noexcept;
        bool has_entry(const wstring_view _Name) const noexcept;
        const vector<entry_type>& get_entries() const noexcept;
        void clear() noexcept;
        [[nodiscard]] bool append(const wstring_view _Name);
        [[nodiscard]] bool erase(const wstring_view _Name) noexcept;
        void reload() noexcept;

    private:
        static constexpr size_t _Npos = static_cast<size_t>(-1); // means entry not found

        database() noexcept;

        static entry_type _Make_entry(const wstring_view _Name) noexcept;
        size_t _Find_entry(const entry_type& _Entry) const noexcept;
        void _Load_database() noexcept;
        void _Extract_entries_from_bytes(const unsigned char* const _Bytes, size_t _Count) noexcept;
        void _Save() noexcept;

        vector<entry_type> _Myentries;
        bool _Mysave;
    };
} // namespace dbmgr

#endif // _DBMGR_DATABASE_HPP_