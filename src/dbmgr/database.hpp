// database.hpp

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#ifndef _DBMGR_DATABASE_HPP_
#define _DBMGR_DATABASE_HPP_
#include <array>
#include <cstddef>
#include <dbmgr/checksum.hpp>
#include <mjfs/file.hpp>
#include <mjfs/path.hpp>
#include <string_view>
#include <vector>

namespace dbmgr {
    class database_location {
    public:
        ~database_location() noexcept;

        database_location(const database_location&)            = delete;
        database_location& operator=(const database_location&) = delete;

        // returns an instance of this class
        static database_location& current() noexcept;
        
        // returns a path to the database directory
        const ::mjfs::path& directory() const noexcept;

        // returns a path to the database file
        const ::mjfs::path& file() const noexcept;
    
    private:
        database_location();

        // returns a path to the database directory
        static ::mjfs::path _Get_directory_path();

        ::mjfs::path _Mydir;
        ::mjfs::path _Myfile;
    };

    class database_entry {
    public:
        using byte_sequence = ::std::array<unsigned char, sizeof(checksum_t)>;

        database_entry() noexcept                      = default;
        database_entry(const database_entry&) noexcept = default;
        database_entry(database_entry&&) noexcept      = default;
        ~database_entry() noexcept                     = default;

        explicit database_entry(const checksum_t _Val) noexcept;

        database_entry& operator=(const database_entry&) noexcept = default;
        database_entry& operator=(database_entry&&) noexcept      = default;

        // compares two entries
        bool operator==(const database_entry& _Other) const noexcept;

        // returns the stored entry as a 4-byte integer
        const checksum_t to_integer() const noexcept;

        // returns the stored entry as a byte sequence
        byte_sequence to_bytes() const noexcept;

    private:
        checksum_t _Myval; // 4-byte entry
    };

    class database {
    public:
        ~database() noexcept;

        database(const database&)            = delete;
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
        void _Load_database();

        // extracts the database entries from raw bytes
        void _Extract_entries_from_bytes(const unsigned char* const _Bytes, size_t _Count);
        
        // saves the database
        void _Save() noexcept;

        ::std::vector<database_entry> _Myentries;
        bool _Mysave; // true if the database should be saved
    };
} // namespace dbmgr

#endif // _DBMGR_DATABASE_HPP_