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
#include <memory>

#include "utilities.hpp"

//According to the msdn documentation, _WIN32 is defined for both 32- and 64bit machines
#ifdef _WIN32
#define _WIN32_LEAN_AND_MEAN
#pragma warning (disable : 4996)
#else
#include <dirent.h>
#include <sys/stat.h>
#endif


constexpr unsigned int TINYDIR_PATH_MAX = 512;
#ifdef _WIN32
constexpr unsigned int TINYDIR_PATH_EXTRA = 2;
#else
constexpr unsigned int TINYDIR_PATH_EXTRA = 0;
#endif
constexpr unsigned int TINYDIR_FILENAME_MAX = 256;


#define FSERROR_TRY_CATCH(throwing_code,catcher) try{ throwing_code; }\
    catch(fs::filesystem_error const & e){ catcher = e.code(); }
#define FSTHROW_MANUAL(enum_code, path1) throw fs::filesystem_error{ \
    fs::details::get_windows_error( GetLastError() ), path1, std::error_code( enum_code )}
#define FSTHROW_MANUAL_DPATH(enum_code, path1, path2) throw fs::filesystem_error{ \
    fs::details::get_windows_error( GetLastError() ), path1, path2, make_error_code(enum_code)}
#define FSTHROW(enum_code, path1) throw fs::filesystem_error{ \
    fs::details::get_windows_error( GetLastError() ), path1, std::make_error_code(enum_code)}
#define FSTHROW_DPATH(enum_code, path1, path2) throw fs::filesystem_error{ \
    fs::details::get_windows_error( GetLastError() ), path1, path2, std::make_error_code(enum_code)}

namespace tinydircpp {
    namespace fs
    {
        class file_status {
        public:
            explicit file_status( file_type ft = file_type::none, perms permission = perms::none ) noexcept;
            //file_status( file_status const & ) noexcept = default;
            //file_status( file_status && ) noexcept = default;
            //~file_status() = default;

            file_type type() const noexcept { return ft_; }
            void type( file_type ) noexcept;
            perms permission() const noexcept { return permission_; }
            void  permission( perms ) noexcept;
            friend bool operator==( file_status const &, file_status const & );
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
            directory_entry( path const & p, file_status stat = file_status{},
                file_status symlink_status = file_status{} );
            ~directory_entry() = default;

            void assign( file_path const & p, file_status st = file_status{}, file_status sym_link = file_status{} );
            void replace_filename( path const & p, file_status st = file_status{}, file_status sym_link = file_status{} );
            directory_entry& operator=( directory_entry const & ) = default;
            directory_entry& operator=( directory_entry && ) = default;

            file_path path() const noexcept;
            file_status status() const noexcept;
            file_status symlink_status() const;

            bool operator<( directory_entry const & ) const;
            bool operator<=( directory_entry const & ) const;
            bool operator==( directory_entry const & ) const;
            bool operator!=( directory_entry const & ) const;
            bool operator>( directory_entry const & ) const;
            bool operator>=( directory_entry const & ) const;

        private:
            file_path path_ {};
            mutable file_status status_ {};
            mutable file_status symlink_status_ {};
        };

        class directory_iterator : public std::iterator<std::input_iterator_tag, directory_entry>
        {
            //std::error_code error_code{};
            path full_path{};
            mutable directory_entry entry{};
            HANDLE search_handle{ INVALID_HANDLE_VALUE };
        public:
            directory_iterator() = default;
            directory_iterator( path const & p ) noexcept;
            directory_iterator( path const & p, std::error_code & ec ) noexcept : directory_iterator{ p }{}
            
            directory_entry const & operator*() const;
            // directory_iterator must satisfy the requirements for input iterator
            bool operator==( directory_iterator const & iter ) const;
            bool operator!= ( directory_iterator const & iter ) const;
            directory_iterator& operator++();
            directory_iterator& begin() noexcept;
            directory_iterator end() noexcept;
            directory_iterator const & cbegin() const;
            directory_iterator const cend() const;
        };

        path current_path();
        path current_path( std::error_code & ec ) noexcept;

        void current_path( path const & p );
        void current_path( path const & p, std::error_code & ec )noexcept;

        path abspath( path const & p );
        path basename( path const & p );

        template<typename Iterator, typename = typename
            std::enable_if<std::is_same<typename std::iterator_traits<Iterator>::value_type, fs::path>::value>::type>
            path common_prefix( Iterator beg, Iterator end )
        {
            if ( beg == end ) return path{};
            path const shortest_address{ *std::min_element( beg, end, []( path const & p1, path const & p2 ) {
                return p1.native().size() < p2.native().size(); } ) };
            size_t counter = 0;
            auto const shortest_filename = shortest_address.native();
            for ( size_t i = 0; i != shortest_filename.size(); ++i ) {
                for ( Iterator iter = beg; iter != end; ++iter ) {
                    auto const &current_path = *iter;
                    if ( shortest_filename[ i ] != current_path.native()[ i ] )
                        return path{ shortest_filename.substr( 0, counter ) };
                }
                counter += 1;
            }
            return counter != shortest_filename.size() ? path{} : path{ shortest_filename.substr( 0, counter ) };
        }

        template<typename Iterator>
        path common_path( Iterator begin, Iterator end )
        {
            return directory_name( common_prefix( begin, end ) );
        }

        path directory_name( path const & p );
        path get_home_path();

        void copy( path const & from, path const & to );
        void copy( path const & from, path const & to, std::error_code & ec ) noexcept;

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

        //bool is_block_file( file_status s ) noexcept;
        //bool is_block_file( path const & p );
        //bool is_block_file( path const & p, std::error_code & ec ) noexcept;

        //bool is_character_file( file_status s ) noexcept;
        //bool is_character_file( path const & p );
        //bool is_character_file( path const & p, std::error_code & ec ) noexcept;

        bool is_directory( file_status s ) noexcept;
        bool is_directory( path const & p );
        bool is_directory( path const & p, std::error_code & ec ) noexcept;

        bool is_empty( path const & p );
        bool is_empty( path const & p, std::error_code & ec ) noexcept;

        /*bool is_fifo( file_status s ) noexcept;
        bool is_fifo( path const & p );
        bool is_fifo( path const & p, std::error_code & ec ) noexcept;
        */
        bool is_other( file_status s ) noexcept;
        bool is_other( path const & p );
        bool is_other( path const & p, std::error_code & ec ) noexcept;

        bool is_regular_file( file_status s ) noexcept;
        bool is_regular_file( path const & p );
        bool is_regular_file( path const & p, std::error_code & ec ) noexcept;

        /*bool is_socket( file_status s ) noexcept;
        bool is_socket( path const & p );
        bool is_socket( path const & p, std::error_code & ec ) noexcept;
        */
        bool is_symlink( file_status s ) noexcept;
        bool is_symlink( path const & p );
        bool is_symlink( path const & p, std::error_code & ec ) noexcept;

        file_time_type last_write_time( path const & p );
        file_time_type last_write_time( path const & p, std::error_code & ec ) noexcept;

        void set_last_write_time( path const & p, file_time_type new_time );
        void set_last_write_time( path const & p, file_time_type new_time, std::error_code & ec ) noexcept;

        file_time_type last_access_time( path const & p );
        file_time_type last_access_time( path const & p, std::error_code & ec ) noexcept;
        void set_last_access_time( path const & p, file_time_type new_time );
        void set_last_access_time( path const & p, file_time_type new_time, std::error_code & ec ) noexcept;

        file_time_type creation_time( path const & p );
        file_time_type creation_time( path const & p, std::error_code & ec ) noexcept;
        void set_creation_time( path const & p, file_time_type new_time );
        void set_creation_time( path const & p, file_time_type new_time, std::error_code & ec ) noexcept;

        bool is_relative_path( path const & p );
        bool is_abs( path const & p );

        //void permissions( path const & p, perms perm );
        //void permissions( path const & p, perms perm, std::error_code & ec ) noexcept;

        path read_symlink( path const & p );
        path read_symlink( path const & p, std::error_code & ec ) noexcept;

        //bool remove( path const & p );
        //bool remove( path const & p, std::error_code & ex ) noexcept;

        //std::uintmax_t remove_all( path const & p );
        //std::uintmax_t remove_all( path const & p, std::error_code &ec ) noexcept;

        //void rename( path const & p, perms perm );
        //void rename( path const & p, perms perm, std::error_code & ec ) noexcept;

        void resize_file( path const & p, std::uintmax_t new_size );
        void resize_file( path const & p, std::uintmax_t new_size, std::error_code & ec ) noexcept;

        space_info space( path const &p );
        space_info space( path const &p, std::error_code & ec ) noexcept;

        file_status status( path const & p );
        file_status status( path const & p, std::error_code & ec ) noexcept;

        bool status_known( file_status s ) noexcept;

        file_status symlink_status( path const & p );
        file_status symlink_status( path const & p, std::error_code &ec ) noexcept;

        //path system_complete( path const & p );
        //path system_complete( path const & p, std::error_code &ec ) noexcept;

        path temporary_directory_path();
        path temporary_directory_path( std::error_code & ec ) noexcept;

        bool operator==( path const & a, path const & b );
        //path unique_path( path const & directory_path = path( "%%%%-%%%%-%%%%-%%%%" ) );
        //path unique_path( path const & p, std::error_code & ec ) noexcept;
    }
}
#endif
