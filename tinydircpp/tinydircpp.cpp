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
            file_status symlink_status ): path_{ p }, status_{ status }, symlink_status_{ symlink_status }
        {
        }
        path & path::append( std::string const & str )
        {
            // TODO: insert return statement here
        }
        path & path::append( path const & p )
        {
            // TODO: insert return statement here
        }
        path current_path()
        {
            char file_path[ TINYDIR_FILENAME_MAX + 2 ] = {};
            DWORD const len_current_directory{ GetCurrentDirectory( TINYDIR_FILENAME_MAX, file_path ) };
            if ( len_current_directory == 0 )
                throw fs::filesystem_error( "Unable to get directory name", std::make_error_code( std::errc::permission_denied ) );
            else if ( len_current_directory > ( TINYDIR_FILENAME_MAX + 2 ) )
                throw fs::filesystem_error( "filename too long", std::make_error_code( std::errc::filename_too_long ) );
            return path{ file_path };
        }

        path current_path( std::error_code & ec ) noexcept
        {
            try {
                return current_path();
            }
            catch( fs::filesystem_error const & error ) {
                ec = error.code();
            }
            return{};
        }
    }
}
