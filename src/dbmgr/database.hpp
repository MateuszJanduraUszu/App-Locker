// database.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _DBMGR_DATABASE_HPP_
#define _DBMGR_DATABASE_HPP_
#include <dbmgr/checksum.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string_view>
#include <vector>

namespace dbmgr {
    using path = ::std::filesystem::path;

    struct _File_traits {
        using _Handle_type = void*;

        // opens the file
        [[nodiscard]] static _Handle_type _Open(const path& _Target);

        // clears the file
        static bool _Clear(const _Handle_type _Handle) noexcept;
    
        // returns the size of the file
        static size_t _Size(const _Handle_type _Handle) noexcept;

        // reads data from the file
        static size_t _Read(
            const _Handle_type _Handle, unsigned char* const _Buf, const size_t _Size) noexcept;
    
        // writes data to the file
        static bool _Write(
            const _Handle_type _Handle, const unsigned char* const _Data, const size_t _Size) noexcept;
    };

    class _Database_file {
    public:
        _Database_file() noexcept;
        ~_Database_file() noexcept;

        // checks if the file is ready to use
        bool _Good() const noexcept;
        
        // clears the file
        bool _Clear() noexcept;

        // returns the size of the file
        size_t _Size() noexcept;

        // reads data from the file
        size_t _Read(unsigned char* const _Buf, const size_t _Size) noexcept;
        
        // writes data to the file
        bool _Write(const unsigned char* const _Data, const size_t _Size) noexcept;

    private:
        _File_traits::_Handle_type _Myhandle;
    };

    class database_location {
    public:
        ~database_location() noexcept;

        database_location(const database_location&) = delete;
        database_location& operator=(const database_location&) = delete;

        // returns an instance of this class
        static database_location& current() noexcept;
        
        // returns a path to the database directory
        const path& directory() const;

        // returns a path to the database file
        const path& file() const;
    
    private:
        database_location();

        // returns a path to the database directory
        static path _Get_directory_path();

        path _Mydir;
        path _Myfile;
    };

    class database_entry {
    public:
        using byte_sequence = ::std::array<unsigned char, sizeof(uint32_t)>;

        database_entry() noexcept;
        database_entry(const database_entry& _Other) noexcept;
        database_entry(database_entry&& _Other) noexcept;
        ~database_entry() noexcept;

        explicit database_entry(const uint32_t _Val) noexcept;

        database_entry& operator=(const database_entry& _Other) noexcept;
        database_entry& operator=(database_entry&& _Other) noexcept;

        // compares two entries
        bool operator==(const database_entry& _Other) const noexcept;

        // returns the stored entry as a 4-byte integer
        const uint32_t to_integer() const noexcept;

        // returns the stored entry as a byte sequence
        byte_sequence to_bytes() const noexcept;

    private:
        uint32_t _Myval; // 4-byte entry
    };

    class database {
    public:
        ~database() noexcept;

        database(const database&) = delete;
        database& operator=(const database&) = delete;
        
        // returns an instance of this class
        static database& current() noexcept;
        
        // returns the number of entries
        size_t entry_count() const noexcept;
        
        // checks if the database has the selected entry
        bool has_entry(const ::std::wstring_view _Name) const noexcept;
        
        // returns all entries
        const ::std::vector<database_entry>& get_entries() const noexcept;
        
        // clears the database
        void clear() noexcept;
        
        // adds a new entry
        [[nodiscard]] bool append(const ::std::wstring_view _Name);
        
        // erases the selected entry
        [[nodiscard]] bool erase(const ::std::wstring_view _Name) noexcept;

        // reloads the database
        void reload() noexcept;

    private:
        static constexpr size_t _Npos = static_cast<size_t>(-1); // means entry not found

        database() noexcept;

        // makes a database entry from
        static database_entry _Make_entry(const ::std::wstring_view _Name) noexcept;

        // returns the position of the selected entry
        size_t _Find_entry(const database_entry& _Entry) const noexcept;
        
        // loads the database
        void _Load_database() noexcept;

        // extracts the database entries from raw bytes
        void _Extract_entries_from_bytes(const unsigned char* const _Bytes, size_t _Count) noexcept;
        
        // saves the database
        void _Save() noexcept;

        ::std::vector<database_entry> _Myentries;
        bool _Mysave; // true if the database should be saved
    };
} // namespace dbmgr

#endif // _DBMGR_DATABASE_HPP_