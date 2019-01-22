/*
Copyright (c) 2019 - Joshua Ogunyinka
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TINYDIRCPP_HPP
#define TINYDIRCPP_HPP

#include <string>
#include <algorithm>
#include <vector>
#include "utilities.hpp"

//According to the msdn documentation, _WIN32 is defined for both 32- and 64bit machines
#ifdef _WIN32
#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning (disable : 4996)
#else
#include <dirent.h>
#include <sys/stat.h>
#endif


#ifdef _WIN32
#define SLASH "\\"
#else
#define SLASH "/"
#endif

constexpr unsigned int TINYDIR_PATH_MAX = 4096;
#ifdef _WIN32
constexpr unsigned int TINYDIR_PATH_EXTRA = 2;
#else
constexpr unsigned int TINYDIR_PATH_EXTRA = 0;
#endif
constexpr unsigned int TINYDIR_FILENAME_MAX = 256;

namespace tinydircpp {
    namespace fs
    {
        class path {
        public:
            path() : path( "" ) {}
            explicit path( std::string const & pathname ): pathname_{ pathname }{}
            constexpr explicit path( char const * pathname ) : pathname_{ pathname } {}
            path( path const & name ): pathname_{ name.pathname_ }{}
            ~path() = default;
            path& operator/=( path const & p );
            path& append( std::string const & str );
            path& append( path const & p );
            path& operator +=( std::string const & str );
            path& operator+=( path const & p );
            path& operator+=( char p );
            void clear() noexcept;
            path& remove_filename();
            path& replace_filename_with( path const & new_filename );
            path& replace_extension( path const & );
            int compare( path const & ) const noexcept;
            int compare( std::string const & ) const noexcept;
            path root_name() const;
            path root_directory() const;
            path relative_path() const;
            path filename() const;
            path extension() const;

            bool empty() const;
            bool has_root_name() const;
            bool has_root_directory() const;
            bool has_filename() const;
            bool is_absolute() const;
            bool is_relative() const;
        private:
            std::string pathname_;
        };

        class file_status {
        public:
            explicit file_status( file_type ft = file_type::none, perms permission = perms::none ) noexcept;
            file_status( file_status const & ) noexcept = default;
            file_status( file_status && ) noexcept = default;
            ~file_status() = default;

            file_type type() const noexcept { return ft_; }
            void type( file_type ) noexcept;
            perms permission() const noexcept { return permission_; }
            void permission( perms ) noexcept;
        private:
            file_type ft_;
            perms permission_;
        };

        class directory_entry {
            using file_path = path;
        public:
            directory_entry() = default;
            directory_entry( directory_entry const & ) = default;
            directory_entry( directory_entry && ) = default;
            /** marked 'explicit' in the standard, but not here
            I think a directory_entry should be constructible from any string 
            */
            directory_entry( path const & path, file_status status = file_status{},
                file_status symlink_status = file_status{} );
            ~directory_entry() = default;

            void assign( file_path const & p, file_status st = file_status{}, file_status sym_link = file_status{} );
            void replace_filename( path const & p, file_status st = file_status{}, file_status sym_link = file_status{} );
            directory_entry& operator=( directory_entry const & ) = default;
            directory_entry& operator=( directory_entry && ) = default;

            file_path path() const noexcept;
            file_status status() const noexcept;
            file_status status( std::error_code & ec ) const noexcept;
            file_status symlink_status() const;
            file_status symlink_status( std::error_code & ec ) const noexcept;

            bool operator<( directory_entry const & );
            bool operator<=( directory_entry const & );
            bool operator==( directory_entry const & );
            bool operator!=( directory_entry const & );
            bool operator>( directory_entry const & );
            bool operator>=( directory_entry const & );

        private:
            file_path path_;
            file_status status_;
            file_status symlink_status_;
        };

        class directory_iterator: public std::iterator<std::input_iterator_tag, directory_entry>
        {
        public:
            directory_iterator() noexcept;
            directory_iterator( path const & p ) noexcept;
            directory_iterator( path const & p, std::error_code & ec ) noexcept;
            directory_iterator( directory_iterator const & ) noexcept = default;
            directory_iterator( directory_iterator && ) noexcept = default;
            ~directory_iterator() noexcept = default;

            directory_iterator& operator++();
            directory_iterator& operator++( int );
            directory_iterator const& operator*() const;
            directory_iterator const* operator->() const;

            // directory_iterator must satisfy the requirements for input iterator
            bool operator==( directory_iterator const & ); // must EqualityComparable
            operator bool() const; // implicitly convertible to bool
            directory_iterator& begin();
            directory_iterator& end();
            directory_iterator& cbegin() const;
            directory_iterator& cend() const;
        };

        path current_path();
        path current_path( std::error_code & ec ) noexcept;
        
        void current_path( path const & p );
        void current_path( path const & p, std::error_code & ec )noexcept;
        
        path absolute( path const & p, path const & base_path = current_path() );
        
        path canonical( path const & p, path const & base = current_path() );
        path canonical( path const & p, std::error_code & ec ) noexcept;
        path canonical( path const & p, path const & base, std::error_code & ec ) noexcept;
        
        void copy( path const & from, path const & to );
        void copy( path const & from, path const & to, std::error_code & ec ) noexcept;
        void copy( path const & from, path const & to, copy_options options );
        void copy( path const & from, path const & to, copy_options options, std::error_code & ec ) noexcept;

        void copy_file( path const & from, path const & to );
        void copy_file( path const & from, path const & to, std::error_code & ec ) noexcept;
        void copy_file( path const & from, path const & to, copy_options options );
        void copy_file( path const & from, path const & to, copy_options options, std::error_code & ec ) noexcept;

        void copy_symlink( path const & existing_symlink, path const & new_symlink );
        void copy_symlink( path const & existing_symlink, path const & new_symlink, std::error_code & ec ) noexcept;

        bool create_directories( path const & p );
        bool create_directories( path const & p, std::error_code & ec ) noexcept;

        void create_directory( path const & p, path const & existing_path );
        void create_directory( path const & p, path const & existing_path, std::error_code & ec ) noexcept;

        bool create_directory_symlink( path const & to, path const & new_symlink );
        bool create_directory_symlink( path const & to, path const & new_symlink, std::error_code & ec ) noexcept;
        
        void create_hard_link( path const & to, path const & new_hardlink );
        void create_hard_link( path const & to, path const & new_hardlink, std::error_code & ec ) noexcept;

        void create_symlink( path const & to, path const & new_symlink );
        void create_symlink( path const & to, path const & new_symlink, std::error_code & ec ) noexcept;
        
        bool exists( file_status s ) noexcept;
        bool exists( path const & p );
        bool exists( path const & p, std::error_code & ec ) noexcept;
        
        bool equivalent( path const & a, path const & b );
        bool equivalent( path const & a, path const & b, std::error_code & ec ) noexcept;

        std::uintmax_t file_size( path const & p );
        std::uintmax_t file_size( path const & p, std::error_code & ec ) noexcept;
        
        std::uintmax_t hard_link_count( path const & p );
        std::uintmax_t hard_link_count( path const & p, std::error_code & ec ) noexcept;
        
        bool is_block_file( file_status s ) noexcept;
        bool is_block_file( path const & p );
        bool is_block_file( path const & p, std::error_code & ec ) noexcept;
        
        bool is_character_file( file_status s ) noexcept;
        bool is_character_file( path const & p );
        bool is_character_file( path const & p, std::error_code & ec ) noexcept;
        
        bool is_directory( file_status s ) noexcept;
        bool is_directory( path const & p );
        bool is_directory( path const & p, std::error_code & ec ) noexcept;
        
        bool is_empty( path const & p );
        bool is_empty( path const & p, std::error_code & ec ) noexcept;
        
        bool is_fifo( file_status s ) noexcept;
        bool is_fifo( path const & p );
        bool is_fifo( path const & p, std::error_code & ec ) noexcept;
        
        bool is_other( file_status s ) noexcept;
        bool is_other( path const & p );
        bool is_other( path const & p, std::error_code & ec ) noexcept;

        bool is_regular_file( file_status s ) noexcept;
        bool is_regular_file( path const & p );
        bool is_regular_file( path const & p, std::error_code & ec ) noexcept;

        bool is_socket( file_status s ) noexcept;
        bool is_socket( path const & p );
        bool is_socket( path const & p, std::error_code & ec ) noexcept;

        bool is_symlink( file_status s ) noexcept;
        bool is_symlink( path const & p );
        bool is_symlink( path const & p, std::error_code & ec ) noexcept;

        file_time_type last_write_time( path const & p );
        file_time_type last_write_time( path const & p, std::error_code & ec ) noexcept;
        
        void last_write_time( path const & p, file_time_type new_time );
        void last_write_time( path const & p, file_time_type new_time, std::error_code & ec ) noexcept;

        void permissions( path const & p, perms perm );
        void permissions( path const & p, perms perm, std::error_code & ec ) noexcept;

        path read_symlink( path const & p );
        path read_symlink( path const & p, std::error_code & ec ) noexcept;
        
        bool remove( path const & p );
        bool remove( path const & p, std::error_code & ex ) noexcept;

        std::uintmax_t remove_all( path const & p );
        std::uintmax_t remove_all( path const & p, std::error_code &ec ) noexcept;

        void rename( path const & p, perms perm );
        void rename( path const & p, perms perm, std::error_code & ec ) noexcept;
        
        void resize_file( path const & p, std::uintmax_t new_size );
        void resize_file( path const & p, std::uintmax_t new_size, std::error_code & ec ) noexcept;

        space_info space( path const &p );
        space_info space( path const &p, std::error_code & ec ) noexcept;

        file_status status( path const & p );
        file_status status( path const & p, std::error_code & ec ) noexcept;
        
        bool status_known( file_status s ) noexcept;
        
        file_status symlink_status( path const & p );
        file_status symlink_status( path const & p, std::error_code &ec ) noexcept;

        path system_complete( path const & p );
        path system_complete( path const & p, std::error_code &ec ) noexcept;

        path temporary_directory_path();
        path temporary_directory_path( std::error_code & ec) noexcept;
        
        path unique_path( path const & directory_path = "%%%%-%%%%-%%%%-%%%%" );
        path unique_path( path const & p, std::error_code & ec ) noexcept;
    }
}
#endif
