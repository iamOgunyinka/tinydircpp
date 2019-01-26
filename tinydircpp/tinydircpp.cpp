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

// tinydircpp.cpp : Defines the exported functions for the static library.
//

#include "tinydircpp.hpp"
#include <system_error>

namespace tinydircpp {
    namespace fs {

        file_status::file_status( file_type ft, perms permission ) noexcept:
        ft_{ ft }, permission_{ permission }
        {
        }

        void file_status::type( file_type ft ) noexcept
        {
            ft_ = ft;
        }

        void file_status::permission( perms prms ) noexcept
        {
            permission_ = prms;
        }

        directory_entry::directory_entry( directory_entry::file_path const & p, file_status status,
            file_status symlink_status ) : path_{ p }, status_{ status }, symlink_status_{ symlink_status }
        {
        }

        path current_path()
        {
            char file_path[ TINYDIR_FILENAME_MAX + 2 ] = {};
            DWORD const len_current_directory{ GetCurrentDirectory( TINYDIR_FILENAME_MAX, file_path ) };
            if ( len_current_directory == 0 )
                throw fs::filesystem_error( "Unable to get directory name", fs::filesystem_error_codes::directory_name_unobtainable );
            else if ( len_current_directory > ( TINYDIR_FILENAME_MAX + 2 ) )
                throw fs::filesystem_error( "filename too long", std::make_error_code( std::errc::filename_too_long ) );
            return path{ file_path };
        }

        path current_path( std::error_code & ec ) noexcept
        {
            try {
                return current_path();
            }
            catch ( fs::filesystem_error const & error ) {
                ec = error.code();
            }
            return{};
        }

        void current_path( path const & p )
        {
            if ( SetCurrentDirectory( p.string().c_str() ) == 0 ) { // failed
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ),
                    std::make_error_code( std::errc::no_such_file_or_directory ) };
            }
        }

        void current_path( path const & p, std::error_code & ec ) noexcept
        {
            try {
                current_path( p );
            }
            catch ( filesystem_error const & error ) {
                ec = error.code();
            }
        }

        void copy( path const & from, path const & to )
        {
            BOOL const fail_if_exists = true;
            BOOL const copying_succeeded = CopyFile( from.string().c_str(), to.string().c_str(), fail_if_exists );
            if ( !copying_succeeded ) {
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ), from, to,
                    fs::filesystem_error_codes::unknown_io_error };
            }
        }

        void copy( path const & from, path const & to, std::error_code & ec ) noexcept
        {
            try {
                copy( from, to );
            }
            catch ( fs::filesystem_error const & error ) {
                ec = error.code();
            }
        }

        bool exists( path const & p )
        {
            return GetFileAttributes( p.string().c_str() ) != INVALID_FILE_ATTRIBUTES;
        }
        bool exists( path const & p, std::error_code & ec ) noexcept
        {
            bool const found = GetFileAttributes( p.string().c_str() ) != INVALID_FILE_ATTRIBUTES;
            if ( !found ) {
                ec = std::make_error_code( std::errc::no_such_file_or_directory );
            }
            return found;
        }

        bool exists( file_status status ) noexcept
        {
            return status_known( status ) && ( status.type() != file_type::not_found );
        }

        std::uintmax_t file_size( path const & p )
        {
            if ( !exists( p ) || !is_regular_file( p ) ) return static_cast< std::uintmax_t >( -1 );

            HANDLE file_handle = CreateFile( p.string().c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, nullptr );
            if ( file_handle == INVALID_HANDLE_VALUE ) { // handle was not opened here, no need for cleanup
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ),
                    std::error_code( fs::filesystem_error_codes::handle_not_opened ) };
            }
            LARGE_INTEGER file_size{};
            BOOL const size_retrieval_succeeds = GetFileSizeEx( file_handle, &file_size );
            if ( size_retrieval_succeeds == 0 ) {
                CloseHandle( file_handle );
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ),
                std::error_code( fs::filesystem_error_codes::could_not_obtain_size ) };
            }
            CloseHandle( file_handle );
            return file_size.QuadPart;
        }

        std::uintmax_t file_size( path const & p, std::error_code & ec ) noexcept
        {
            std::uintmax_t file_size = static_cast< std::uintmax_t >( -1 );
            try {
                file_size = file_size( p );
            }
            catch ( fs::filesystem_error const & error ) {
                ec = error.code();
            }
            return file_size;
        }

        bool is_regular_file( path const & p )
        {
            return is_regular_file( status( p ) );
        }

        bool is_regular_file( file_status status ) noexcept
        {
            return status.type() == file_type::regular;
        }

        bool is_regular_file( path const & p, std::error_code & ec ) noexcept
        {
            try {
                return is_regular_file( p );
            }
            catch ( fs::filesystem_error const & e ) {
                ec = e.code();
            }
            return false;
        }


        bool is_directory( file_status s ) noexcept
        {
            return s.type() == file_type::directory;
        }

        bool is_directory( path const & p )
        {
            return is_directory( status( p ) );
        }

        bool is_directory( path const & p, std::error_code & ec ) noexcept
        {
            try {
                return is_directory( p );
            }
            catch ( fs::filesystem_error const & fs_error ) {
                ec = fs_error.code();
            }
            return false;
        }

        bool is_empty( path const & p )
        {
            return is_directory( status( p ) ) ? directory_iterator( p ) == directory_iterator() :
                file_size( p ) == 0;
        }

        bool is_empty( path const & p, std::error_code & ec ) noexcept
        {
            try {
                return is_empty( p );
            }
            catch ( fs::filesystem_error const & err ) {
                ec = err.code();
            }
            return false;
        }

        file_status status( path const & p )
        {
            std::error_code ec{};
            file_status const st = status( p, ec );
            if ( st.type() == file_type::none ) {
                throw fs::filesystem_error{ "", p, ec };
            }
            return st;
        }
        /*
        none = 0,
        block,
        character,
        fifo,
        socket,
        unknown
    */
        // to-do: determine the rest of the file type
        file_status status( path const & p, std::error_code & ec )
        {
            DWORD const file_attrib = GetFileAttributes( p.string().c_str() );
            if ( file_attrib == INVALID_FILE_ATTRIBUTES ) {
                ec = std::make_error_code( std::errc::no_such_file_or_directory );
                return file_status{ file_type::not_found };
            }
            ec.clear();
            if ( file_attrib & FILE_ATTRIBUTE_DIRECTORY ) {
                return file_status{ file_type::directory };
            }
            else if ( file_attrib & FILE_ATTRIBUTE_REPARSE_POINT ) {
                WIN32_FIND_DATA find_data{};
                HANDLE symlink_handle = FindFirstFile( p.string().c_str(), &find_data );
                if ( symlink_handle == INVALID_HANDLE_VALUE ) {
                    ec = std::error_code( fs::filesystem_error_codes::handle_not_opened );
                    return file_status{ file_type::unknown };
                }
                if ( find_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT 
                    && IsReparseTagMicrosoft( find_data.dwReserved0 )
                    && find_data.dwReserved0 == IO_REPARSE_TAG_SYMLINK ) {
                    FindClose( symlink_handle );
                    return file_status{ file_type::symlink };
                }
                FindClose( symlink_handle );
                return file_status{ file_type::unknown };
            }
            return file_status{ file_type::regular };
        }
    }
}
