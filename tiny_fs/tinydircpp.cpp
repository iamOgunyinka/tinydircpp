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


#include "tinydircpp.hpp"
#include <system_error>

#ifdef _WIN32
#include <ShlObj.h>
#endif // _WIN32



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
            DWORD const len_current_directory{ GetCurrentDirectoryA( TINYDIR_FILENAME_MAX, file_path ) };
            if ( len_current_directory == 0 )
                throw fs::filesystem_error( "Unable to get directory name", fs::filesystem_error_codes::directory_name_unobtainable );
            else if ( len_current_directory > ( TINYDIR_FILENAME_MAX + 2 ) )
                throw fs::filesystem_error( "filename too long", std::make_error_code( std::errc::filename_too_long ) );
            return path{ file_path };
        }

        path current_path( std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return current_path(), ec );
            return{};
        }

        void current_path( path const & p )
        {
            if ( SetCurrentDirectoryA( p.string().c_str() ) == 0 ) { // failed
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ),
                    std::make_error_code( std::errc::no_such_file_or_directory ) };
            }
        }

        void current_path( path const & p, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( current_path( p ), ec );
        }

        void copy( path const & from, path const & to )
        {
            BOOL const fail_if_exists = true;
            BOOL const copying_succeeded = CopyFileA( from.string().c_str(), to.string().c_str(), fail_if_exists );
            if ( !copying_succeeded ) {
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ), from, to,
                    fs::filesystem_error_codes::unknown_io_error };
            }
        }

        void copy( path const & from, path const & to, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( copy( from, to ), ec );
        }

        bool exists( path const & p )
        {
            return GetFileAttributesA( p.string().c_str() ) != INVALID_FILE_ATTRIBUTES;
        }
        bool exists( path const & p, std::error_code & ec ) noexcept
        {
            bool const found = GetFileAttributesA( p.string().c_str() ) != INVALID_FILE_ATTRIBUTES;
            if ( !found ) {
                ec = std::make_error_code( std::errc::no_such_file_or_directory );
            }
            return found;
        }

        void create_hard_link( path const & to, path const & new_hardlink )
        {
            if ( CreateHardLinkA( new_hardlink.string().c_str(), to.string().c_str(), nullptr ) == 0 ) {
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ),
                to, new_hardlink, std::make_error_code( std::errc::no_link ) };
            }
        }

        void create_hard_link( path const & to, path const & new_hardlink, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( create_hard_link( to, new_hardlink ), ec );
        }

        void create_directory( path const & p, path const & existing_path )
        {
            if ( CreateDirectoryExA( existing_path.string().c_str(), p.string().c_str(), 0 ) == 0 ) {
                DWORD const last_error = GetLastError();
                if ( last_error == ERROR_ALREADY_EXISTS ) {
                    return; // not an error
                }
                throw fs::filesystem_error{ fs::details::get_windows_error( last_error ), p, existing_path,
                    std::error_code( fs::filesystem_error_codes::unknown_io_error ) };
            }
        }

        void create_directory( path const & p, path const & existing_path, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( create_directory( p, existing_path ), ec );
        }


        bool create_directory_symlink( path const & to, path const & new_symlink )
        {
            if ( !is_directory( new_symlink ) ) return false;
            if ( CreateSymbolicLinkA( new_symlink.string().c_str(), to.string().c_str(),
                SYMBOLIC_LINK_FLAG_DIRECTORY ) == 0 ) {
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ),
                    std::make_error_code( std::errc::no_link ) };
            }
            return true;
        }

        bool create_directory_symlink( path const & to, path const & new_symlink, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return create_directory_symlink( to, new_symlink ), ec );
            return false;
        }

        void create_symlink( path const & to, path const & new_symlink )
        {
            if ( !is_regular_file( new_symlink ) ) {
                throw;
            }
            if ( CreateSymbolicLinkA( new_symlink.string().c_str(), to.string().c_str(), 0 ) == 0 ) {
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ),
                    std::make_error_code( std::errc::no_link ) };
            }
        }

        void create_symlink( path const & to, path const & new_symlink, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( create_symlink( to, new_symlink ), ec );
        }
        bool exists( file_status status ) noexcept
        {
            return status_known( status ) && ( status.type() != file_type::not_found );
        }

        std::uintmax_t file_size( path const & p )
        {
            if ( !exists( p ) || !is_regular_file( p ) ) return static_cast< std::uintmax_t >( -1 );

            HANDLE file_handle = CreateFileA( p.string().c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, nullptr );
            if ( file_handle == INVALID_HANDLE_VALUE ) { // handle was not opened here, no need for cleanup
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ),
                    std::error_code( fs::filesystem_error_codes::handle_not_opened ) };
            }
            LARGE_INTEGER sizeof_file{};
            BOOL const size_retrieval_succeeds = GetFileSizeEx( file_handle, &sizeof_file );
            if ( size_retrieval_succeeds == 0 ) {
                CloseHandle( file_handle );
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ),
                std::error_code( fs::filesystem_error_codes::could_not_obtain_size ) };
            }
            CloseHandle( file_handle );
            return sizeof_file.QuadPart;
        }

        std::uintmax_t file_size( path const & p, std::error_code & ec ) noexcept
        {
            std::uintmax_t sizeof_file = static_cast< std::uintmax_t >( -1 );
            FSERROR_TRY_CATCH( sizeof_file = file_size( p ), ec );
            return sizeof_file;
        }

        std::uintmax_t hard_link_count( path const & p )
        {
            HANDLE h = CreateFileA( ( LPCSTR ) p.string().c_str(), GENERIC_READ,
                FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
            if ( h == INVALID_HANDLE_VALUE ) {
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ),
                    fs::filesystem_error_codes::handle_not_opened };
            }
            BY_HANDLE_FILE_INFORMATION file_information{};
            if ( GetFileInformationByHandle( h, ( LPBY_HANDLE_FILE_INFORMATION ) &file_information ) == 0 ) {
                CloseHandle( h );
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ),
                    fs::filesystem_error_codes::hardlink_count_error };
            }
            CloseHandle( h );
            return file_information.nNumberOfLinks;
        }

        std::uintmax_t hard_link_count( path const & p, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return hard_link_count( p ), ec );
            return static_cast< std::uintmax_t >( -1 );
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
            FSERROR_TRY_CATCH( return is_regular_file( p ), ec );
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
            FSERROR_TRY_CATCH( return is_directory( p ), ec );
            return false;
        }

        bool is_empty( path const & p )
        {
            return is_directory( status( p ) ) ? directory_iterator( p ) == directory_iterator() :
                file_size( p ) == 0;
        }

        bool is_empty( path const & p, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return is_empty( p ), ec );
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
        file_status status( path const & p, std::error_code & ec ) noexcept
        {
            DWORD const file_attrib = GetFileAttributesA( p.string().c_str() );
            if ( file_attrib == INVALID_FILE_ATTRIBUTES ) {
                ec = std::error_code( fs::filesystem_error_codes::handle_not_opened );
                return file_status{ file_type::not_found };
            }
            ec.clear();
            if ( file_attrib & FILE_ATTRIBUTE_REPARSE_POINT ) {
                WIN32_FIND_DATAA find_data{};
                HANDLE symlink_handle = FindFirstFileA( p.string().c_str(), &find_data );
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
            else if ( file_attrib & FILE_ATTRIBUTE_DIRECTORY ) {
                return file_status{ file_type::directory };
            }
            return file_status{ file_type::regular };
        }
        bool status_known( file_status s ) noexcept
        {
            return s.type() != file_type::none;
        }
        file_status symlink_status( path const & p )
        {
            auto const stat{ status( p ) };
            return is_symlink( stat ) ? stat : file_status{ file_type::none };
        }

        file_status symlink_status( path const & p, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return symlink_status( p ), ec );
            return file_status{ file_type::none };
        }
        path temporary_directory_path()
        {
            char temp_path[ TINYDIR_PATH_MAX + 2 ]{};
            DWORD path_length = GetTempPathA( TINYDIR_PATH_MAX, ( LPSTR ) temp_path );
            if ( path_length == 0 ) {
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ),
                std::make_error_code( std::errc::filename_too_long ) };
            }
            else if ( path_length > TINYDIR_PATH_MAX ) { // rare, but this is my fault, didn't allocate enough space
                std::string new_path( path_length, ' ' );
                path_length = GetTempPathA( path_length, ( LPSTR ) &new_path[ 0 ] );
                return path{ new_path };
            }
            return path{ temp_path };
        }
        path temporary_directory_path( std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return temporary_directory_path(), ec );
            return path{};
        }

        void copy_symlink( path const & existing_symlink, path const & new_symlink )
        {
            create_symlink( read_symlink( existing_symlink ), new_symlink );
        }

        void copy_symlink( path const & existing_symlink, path const & new_symlink, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( create_symlink( read_symlink( existing_symlink, ec ), new_symlink, ec ), ec );
        }
        bool create_directories( path const & p )
        {
            int const result = SHCreateDirectoryExA( nullptr, p.string().c_str(), nullptr );
            switch ( result ) {
            case ERROR_SUCCESS:
            case ERROR_FILE_EXISTS:
            case ERROR_ALREADY_EXISTS:
                return true;
            case ERROR_FILENAME_EXCED_RANGE:
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ), p,
                    std::make_error_code( std::errc::filename_too_long ) };
                break;
            case ERROR_PATH_NOT_FOUND:
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ), p,
                    std::make_error_code( std::errc::no_such_file_or_directory ) };
                break;
            case ERROR_CANCELLED:
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ), p,
                    std::make_error_code( std::errc::operation_canceled ) };
                break;
            default:
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ), p,
                    std::error_code( fs::filesystem_error_codes::unknown_io_error ) };
            }
        }

        bool create_directories( path const & p, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return create_directories( p ), ec );
            return false;
        }

        file_time_type last_write_time( path const & p )
        {
            char const *fp_path = p.string().c_str();
            WIN32_FILE_ATTRIBUTE_DATA file_data{};
            if ( GetFileAttributesExA( fp_path, GetFileExInfoStandard, &file_data ) == 0 ) {
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ), p,
                    std::error_code( fs::filesystem_error_codes::could_not_obtain_time ) };
            }
            return fs::details::Win32FiletimeToChronoTime( &file_data.ftLastWriteTime );
        }

        file_time_type last_write_time( path const & p, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return last_write_time( p ), ec );
            return file_time_type{};
        }

        void last_write_time( path const & p, file_time_type new_time )
        {
            HANDLE file_handle = CreateFileA( p.string().c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                nullptr, OPEN_EXISTING, FILE_WRITE_ATTRIBUTES | FILE_ATTRIBUTE_NORMAL, nullptr );
            if ( file_handle == INVALID_HANDLE_VALUE ) {
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ), p,
                    std::error_code( fs::filesystem_error_codes::handle_not_opened ) };
            }
            FILETIME const win32_filetime = fs::details::ChronoTimeToWin32Filetime( new_time );
            if ( SetFileTime( file_handle, nullptr, nullptr, &win32_filetime ) == 0 ) {
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ), p,
                    std::error_code( fs::filesystem_error_codes::set_writetime_error ) };
            }
        }

        void last_write_time( path const & p, file_time_type new_time, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( last_write_time( p, new_time ), ec );
        }

        void resize_file( path const & p, std::uintmax_t new_size )
        {
            HANDLE file_handle = CreateFileA( p.string().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
                nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
            if ( file_handle == INVALID_HANDLE_VALUE ) {
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ), p,
                    std::error_code( fs::filesystem_error_codes::handle_not_opened ) };
            }
            if ( SetFilePointer( file_handle, new_size, nullptr, FILE_BEGIN ) == INVALID_SET_FILE_POINTER ) {
                CloseHandle( file_handle );
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ), p,
                    std::error_code( fs::filesystem_error_codes::invalid_set_file_pointer ) };
            }
            CloseHandle( file_handle );
        }

        void resize_file( path const & p, std::uintmax_t new_size, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( resize_file( p, new_size ), ec );
        }

        space_info space( path const & p )
        {
            char const *directory_name = p.string().c_str();
            HANDLE disk_handle = CreateFileA( directory_name, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, 
                OPEN_EXISTING, 0, nullptr );
            if ( disk_handle == INVALID_HANDLE_VALUE ) {
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ), p,
                    std::error_code( fs::filesystem_error_codes::handle_not_opened ) };
            }
            ULARGE_INTEGER free_bytes_available_to_caller{}, total_bytes{}, total_free_bytes{};
            if ( GetDiskFreeSpaceExA( directory_name, &free_bytes_available_to_caller, 
                &total_bytes, &total_free_bytes ) == 0 )
            {
                throw fs::filesystem_error{ fs::details::get_windows_error( GetLastError() ), p,
                    std::error_code( fs::filesystem_error_codes::could_not_obtain_size ) };
            }
            fs::space_info disk_space_info{};
            disk_space_info.free = total_free_bytes.QuadPart;
            disk_space_info.capacity = total_bytes.QuadPart;
            disk_space_info.available = free_bytes_available_to_caller.QuadPart;
            return disk_space_info;
        }

        space_info space( path const & p, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return space( p ), ec );
            return space_info{};
        }

    }
}

#undef FSERROR_TRY_CATCH