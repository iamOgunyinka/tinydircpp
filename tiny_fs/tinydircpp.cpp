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
#include <tuple>

#ifdef _WIN32
#include <winbase.h>
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

        bool operator==( file_status const & stat1, file_status const & stat2 )
        {
            return stat1.ft_ == stat2.ft_ && stat2.permission_ == stat1.permission_;
        }

        path current_path()
        {
            wchar_t file_path[ TINYDIR_FILENAME_MAX + 2 ] = {};
            DWORD const len_current_directory{ GetCurrentDirectoryW( TINYDIR_FILENAME_MAX, file_path ) };
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
            if ( SetCurrentDirectoryW( p.c_str() ) == 0 ) { // failed
                FSTHROW( std::errc::no_such_file_or_directory, p );
            }
        }

        void current_path( path const & p, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( current_path( p ), ec );
        }

        path abspath( path const & p )
        {
            wchar_t fullpath[ TINYDIR_PATH_MAX + TINYDIR_PATH_EXTRA ]{};
            if ( GetFullPathNameW( p.c_str(), TINYDIR_PATH_MAX, fullpath, nullptr ) == 0 ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::unknown_io_error, p );
            }
            return path{ fullpath };
        }

        path basename( path const & p )
        {
            if ( p.empty() ) return p;
            auto const full_path = p.string();
            if ( IS_DIR_SEPARATOR( full_path.back() ) ) return path{};
            auto const pos = full_path.rfind( SLASH );
            if ( pos != full_path.npos ) return path{ full_path.substr( pos + 1 ) };
            return p;
        }

        path get_home_path()
        {
            wchar_t home_path[ TINYDIR_PATH_MAX ]{};
            wchar_t home_drive[ TINYDIR_PATH_MAX ]{};
            //if these first two fails, we would still go ahead and check for other environment variables
            if ( GetEnvironmentVariableW( L"HOME", home_path, TINYDIR_PATH_MAX ) != 0 ) {
                return path{ home_path };
            }
            if ( GetEnvironmentVariableW( L"USERPROFILE", home_drive, TINYDIR_PATH_MAX ) != 0 ) {
                return path{ home_drive };
            }

            if ( GetEnvironmentVariableW( L"HOMEPATH", home_path, TINYDIR_PATH_MAX ) == 0 ||
                GetEnvironmentVariableW( L"HOMEDRIVE", home_drive, TINYDIR_PATH_MAX ) == 0 ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::directory_name_unobtainable, {} );
            }
            return path{ fs::str_t<wchar_t>{ home_drive } +home_path };
        }

        path directory_name( path const & p )
        {
            if ( p.empty() ) return p;
            auto const native_name = p.native();
            auto const pos = native_name.rfind( WSLASH );
            return pos != native_name.npos ? path{ native_name.substr( 0, pos ) } : p;
        }

        void copy( path const & from, path const & to )
        {
            BOOL const fail_if_exists = true;
            BOOL const copying_succeeded = CopyFileW( from.c_str(), to.c_str(), fail_if_exists );
            if ( !copying_succeeded ) {
                FSTHROW_MANUAL_DPATH( fs::filesystem_error_codes::unknown_io_error, from, to );
            }
        }

        void copy( path const & from, path const & to, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( copy( from, to ), ec );
        }

        bool exists( path const & p )
        {
            return GetFileAttributesW( p.c_str() ) != INVALID_FILE_ATTRIBUTES;
        }
        bool exists( path const & p, std::error_code & ec ) noexcept
        {
            bool const found = GetFileAttributesW( p.c_str() ) != INVALID_FILE_ATTRIBUTES;
            if ( !found ) {
                ec = std::make_error_code( std::errc::no_such_file_or_directory );
            }
            return found;
        }

        bool equivalent( path const & a, path const & b )
        {
            if ( ( !exists( a ) && !exists( b ) ) || ( is_other( a ) && is_other( b ) ) ) {
                FSTHROW_DPATH( std::errc::no_such_file_or_directory, a, b );
            }
            details::smart_handle a_file_handle{ CreateFileW( a.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, nullptr ) };
            details::smart_handle b_file_handle{ CreateFileW( b.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, nullptr ) };
            if ( !a_file_handle || !b_file_handle )
                FSTHROW_MANUAL_DPATH( fs::filesystem_error_codes::handle_not_opened, a, b );
            BY_HANDLE_FILE_INFORMATION a_info{}, b_info{};

            if ( GetFileInformationByHandle( a_file_handle, &a_info ) == 0
                || GetFileInformationByHandle( b_file_handle, &b_info ) == 0 ) {
                FSTHROW_MANUAL_DPATH( fs::filesystem_error_codes::unknown_io_error, a, b );
            }
            auto tuple_getter = []( BY_HANDLE_FILE_INFORMATION const & info ) {
                return std::tie( info.dwVolumeSerialNumber, info.nFileIndexLow, info.nFileIndexHigh,
                    info.nFileSizeLow, info.nFileSizeHigh, info.ftLastWriteTime.dwLowDateTime,
                    info.ftLastWriteTime.dwHighDateTime );
            };
            return status( a ) == status( b ) && ( tuple_getter( a_info ) == tuple_getter( b_info ) );
        }

        bool equivalent( path const & a, path const & b, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return equivalent( a, b ), ec );
            return false;
        }

        void create_hard_link( path const & to, path const & new_hardlink )
        {
            if ( CreateHardLinkW( new_hardlink.c_str(), to.c_str(), nullptr ) == 0 ) {
                FSTHROW_MANUAL_DPATH( fs::filesystem_error_codes::no_link, to, new_hardlink );
            }
        }

        void create_hard_link( path const & to, path const & new_hardlink, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( create_hard_link( to, new_hardlink ), ec );
        }

        void create_directory( path const & p, path const & existing_path )
        {
            if ( CreateDirectoryExW( existing_path.c_str(), p.c_str(), nullptr ) == 0 ) {
                DWORD const last_error = GetLastError();
                if ( last_error == ERROR_ALREADY_EXISTS ) {
                    return; // not an error
                }
                FSTHROW_MANUAL_DPATH( fs::filesystem_error_codes::unknown_io_error, p, existing_path );
            }
        }

        void create_directory( path const & p, path const & existing_path, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( create_directory( p, existing_path ), ec );
        }

        bool create_directory_symlink( path const & to, path const & new_symlink )
        {
            if ( !is_directory( new_symlink ) ) return false;
            if ( CreateSymbolicLinkW( new_symlink.c_str(), to.c_str(), SYMBOLIC_LINK_FLAG_DIRECTORY ) == 0 ) {
                FSTHROW_MANUAL_DPATH( fs::filesystem_error_codes::no_link, to, new_symlink );
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
            if ( CreateSymbolicLinkW( new_symlink.c_str(), to.c_str(), 0 ) == 0 ) {
                FSTHROW_MANUAL_DPATH( fs::filesystem_error_codes::no_link, to, new_symlink );
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

            details::smart_handle file_handle{ CreateFileW( p.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr ) };
            if ( !file_handle ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::handle_not_opened, p );
            }
            LARGE_INTEGER sizeof_file{};
            if ( GetFileSizeEx( file_handle, &sizeof_file ) == 0 ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::could_not_obtain_size, p );
            }
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
            details::smart_handle h{ CreateFileW( p.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, nullptr ) };
            if ( !h ) FSTHROW_MANUAL( fs::filesystem_error_codes::handle_not_opened, p );
            BY_HANDLE_FILE_INFORMATION file_information{};
            if ( GetFileInformationByHandle( h, ( LPBY_HANDLE_FILE_INFORMATION ) &file_information ) == 0 ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::hardlink_count_error, p );
            }
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

        bool is_other( file_status s ) noexcept
        {
            return exists( s ) && !is_regular_file( s ) && !is_directory( s ) && !is_symlink( s );
        }

        bool is_other( path const & p )
        {
            return is_other( status( p ) );
        }

        bool is_other( path const & p, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return is_other( p ), ec );
            return true;
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

        /*bool is_empty( path const & p )
        {
            return is_directory( status( p ) ) ? directory_iterator( p ) == directory_iterator() :
                file_size( p ) == 0;
        }

        bool is_empty( path const & p, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return is_empty( p ), ec );
            return false;
        }
*/
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
            DWORD const file_attrib = GetFileAttributesW( p.c_str() );
            if ( file_attrib == INVALID_FILE_ATTRIBUTES ) {
                ec = std::error_code( fs::filesystem_error_codes::handle_not_opened );
                return file_status{ file_type::not_found };
            }
            ec.clear();
            if ( file_attrib & FILE_ATTRIBUTE_REPARSE_POINT ) {
                WIN32_FIND_DATAW find_data{};
                HANDLE symlink_handle = FindFirstFileW( p.c_str(), &find_data ); // not destructible by CloseHandle
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
            } else if ( file_attrib & FILE_ATTRIBUTE_DIRECTORY ) {
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
            wchar_t temp_path[ TINYDIR_PATH_MAX + 2 ]{};
            DWORD path_length = GetTempPathW( TINYDIR_PATH_MAX, temp_path );
            if ( path_length == 0 ) {
                FSTHROW( std::errc::filename_too_long, {} );
            } else if ( path_length > TINYDIR_PATH_MAX ) { // rare, but this is my fault, didn't allocate enough space
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

        bool operator==( path const & a, path const & b )
        {
            return a.native() == b.native() || fs::equivalent( a, b );
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
            int const result = SHCreateDirectoryExW( nullptr, p.c_str(), nullptr );
            switch ( result ) {
            case ERROR_SUCCESS:
            case ERROR_FILE_EXISTS:
            case ERROR_ALREADY_EXISTS:
                return true;
            case ERROR_FILENAME_EXCED_RANGE:
                FSTHROW( std::errc::filename_too_long, p );
                break;
            case ERROR_PATH_NOT_FOUND:
                FSTHROW( std::errc::no_such_file_or_directory, p );
                break;
            case ERROR_CANCELLED:
                FSTHROW_MANUAL( fs::filesystem_error_codes::operation_cancelled, p );
                break;
            default:
                FSTHROW_MANUAL( fs::filesystem_error_codes::unknown_io_error, p );
            }
        }

        bool create_directories( path const & p, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return create_directories( p ), ec );
            return false;
        }

        bool is_symlink( file_status s ) noexcept
        {
            return s.type() == file_type::symlink;
        }

        bool is_symlink( path const & p )
        {
            return is_symlink( status( p ) );
        }

        bool is_symlink( path const & p, std::error_code & ec ) noexcept
        {
            return is_symlink( status( p, ec ) );
        }

        file_time_type last_write_time( path const & p )
        {
            wchar_t const *fp_path = p.c_str();
            WIN32_FILE_ATTRIBUTE_DATA file_data{};
            if ( GetFileAttributesExW( fp_path, GetFileExInfoStandard, &file_data ) == 0 ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::could_not_obtain_time, p );
            }
            return fs::details::Win32FiletimeToChronoTime( file_data.ftLastWriteTime );
        }

        file_time_type last_write_time( path const & p, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return last_write_time( p ), ec );
            return file_time_type{};
        }

        void set_last_write_time( path const & p, file_time_type new_time )
        {
            details::smart_handle file_handle{ CreateFileW( p.c_str(), GENERIC_WRITE, 0,
                nullptr, OPEN_EXISTING, FILE_WRITE_ATTRIBUTES | FILE_ATTRIBUTE_NORMAL, nullptr ) };
            if ( !file_handle ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::handle_not_opened, p );
            }
            FILETIME const win32_filetime = fs::details::ChronoTimeToWin32Filetime( new_time );
            if ( SetFileTime( file_handle, nullptr, nullptr, &win32_filetime ) == 0 ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::set_filetime_error, p );
            }
        }

        void set_last_write_time( path const & p, file_time_type new_time, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( set_last_write_time( p, new_time ), ec );
        }

        file_time_type last_access_time( path const & p )
        {
            WIN32_FILE_ATTRIBUTE_DATA file_metadata{};
            if ( GetFileAttributesExW( p.c_str(), GetFileExInfoStandard, &file_metadata ) == 0 ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::could_not_obtain_time, p );
            }
            return details::Win32FiletimeToChronoTime( file_metadata.ftLastAccessTime );
        }

        file_time_type last_access_time( path const & p, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return last_access_time( p ), ec );
            return file_time_type{};
        }

        void set_last_access_time( path const & p, file_time_type new_time )
        {
            details::smart_handle file_handle{ CreateFileW( p.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr ) };
            if ( !file_handle ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::handle_not_opened, p );
            }
            auto const access_time = details::ChronoTimeToWin32Filetime( new_time );
            if ( SetFileTime( file_handle, nullptr, &access_time, nullptr ) == 0 ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::set_filetime_error, p );
            }
        }

        void set_last_access_time( path const & p, file_time_type new_time, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( set_last_access_time( p, new_time ), ec );
        }

        file_time_type creation_time( path const & p )
        {
            WIN32_FILE_ATTRIBUTE_DATA file_data{};
            if ( GetFileAttributesExW( p.c_str(), GetFileExInfoStandard, &file_data ) == 0 ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::could_not_obtain_time, p );
            }
            return fs::details::Win32FiletimeToChronoTime( file_data.ftCreationTime );
        }

        file_time_type creation_time( path const & p, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return creation_time( p ), ec );
            return file_time_type{};
        }

        void set_creation_time( path const & p, file_time_type new_time )
        {
            details::smart_handle file_handle{ CreateFileW( p.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr ) };
            if ( !file_handle ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::handle_not_opened, p );
            }
            auto const creation_time = details::ChronoTimeToWin32Filetime( new_time );
            if ( SetFileTime( file_handle, &creation_time, nullptr, nullptr ) == 0 ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::set_filetime_error, p );
            }
        }

        void set_creation_time( path const & p, file_time_type new_time, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( set_creation_time( p, new_time ), ec );
        }

        bool is_relative_path( path const & p )
        {
            auto file_path = p.native();
            decltype( file_path )::size_type length = file_path.size();
            using T = typename path::value_type;
            if ( length == 0 || ( length >= 1 && ( file_path[ 0 ] == T( '.' ) ) )
                || ( length > 1 && ( file_path[ 0 ] == T( '.' ) && file_path[ 1 ] == T( '.' ) ) ) ) {
                return true;
            }
            return details::is_filename( file_path );
        }

        bool is_abs( path const & p )
        {
            return !is_relative_path( p );
        }

        path read_symlink( path const & p )
        {
            if ( !is_symlink( p ) ) return path{};

            details::smart_handle h{ CreateFileW( p.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT , nullptr ) };
            if ( !h ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::handle_not_opened, p );
            }
            DWORD returned_data_size = 0;

            union anon_info {
                char abyss[ offsetof( fs::REPARSE_DATA_BUFFER, GenericReparseBuffer ) + ( 1024 * 16 ) ];
                fs::REPARSE_DATA_BUFFER reparse_buffer;
            } ai; // borrowed from boost::filesystem
            if ( DeviceIoControl( h, FSCTL_GET_REPARSE_POINT, nullptr, 0, &ai.reparse_buffer, sizeof( anon_info ),
                &returned_data_size, nullptr ) == 0 ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::unknown_io_error, p );
            }
            std::wstring target_name{ static_cast< wchar_t* >( ai.reparse_buffer.SymbolicLinkReparseBuffer.PathBuffer )
                + ai.reparse_buffer.SymbolicLinkReparseBuffer.PrintNameOffset / 2,
                static_cast< wchar_t* >( ai.reparse_buffer.SymbolicLinkReparseBuffer.PathBuffer )
                + ai.reparse_buffer.SymbolicLinkReparseBuffer.PrintNameOffset / 2
                + ai.reparse_buffer.SymbolicLinkReparseBuffer.PrintNameLength / 2 };
            return path{ target_name };
        }

        path read_symlink( path const & p, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( return read_symlink( p ), ec );
            return path{};
        }

        void resize_file( path const & p, std::uintmax_t new_size )
        {
            details::smart_handle file_handle{ CreateFileW( p.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
                nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr ) };
            if ( !file_handle ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::handle_not_opened, p );
            }
            if ( SetFilePointer( file_handle, ( LONG ) new_size, nullptr, FILE_BEGIN ) == INVALID_SET_FILE_POINTER ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::invalid_set_file_pointer, p );
            }
        }

        void resize_file( path const & p, std::uintmax_t new_size, std::error_code & ec ) noexcept
        {
            FSERROR_TRY_CATCH( resize_file( p, new_size ), ec );
        }

        space_info space( path const & p )
        {
            wchar_t const *directory_name = p.c_str();
            details::smart_handle disk_handle{ CreateFileW( directory_name, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
                nullptr, OPEN_EXISTING, 0, nullptr ) };
            if ( !disk_handle ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::handle_not_opened, p );
            }
            ULARGE_INTEGER free_bytes_available_to_caller{}, total_bytes{}, total_free_bytes{};
            if ( GetDiskFreeSpaceExW( directory_name, &free_bytes_available_to_caller,
                &total_bytes, &total_free_bytes ) == 0 ) {
                FSTHROW_MANUAL( fs::filesystem_error_codes::could_not_obtain_size, p );
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

        directory_entry::directory_entry( fs::path const & p, file_status st, file_status symlink_stat )
            : path_{ p }, status_{ st }, symlink_status_{ symlink_stat }
        {
        }

        void directory_entry::assign( file_path const & p, file_status st, file_status sym_link )
        {
            path_ = p;
            status_ = st;
            symlink_status_ = sym_link;
        }
        // to-do
        void directory_entry::replace_filename( fs::path const & p, file_status st, file_status sym_link )
        {
            //
            status_ = st;
            symlink_status_ = sym_link;
        }

        directory_entry::file_path directory_entry::path() const noexcept
        {
            return path_;
        }

        file_status directory_entry::status() const noexcept
        {
            if ( !fs::status_known( status_ ) ) {
                if ( fs::status_known( symlink_status_ ) && !fs::is_symlink( symlink_status_ ) ) {
                    status_ = symlink_status_;
                } else {
                    std::error_code ec{};
                    status_ = fs::status( path_, ec );
                }
            }
            return status_;
        }

        file_status directory_entry::symlink_status() const
        {
            if ( !fs::status_known( symlink_status_ ) ) {
                symlink_status_ = fs::symlink_status( path_ );
            }
            return symlink_status_;
        }
        bool directory_entry::operator<( directory_entry const & rhs ) const
        {
            return path_ < rhs.path_;
        }

        bool directory_entry::operator<=( directory_entry const & rhs ) const
        {
            return ( *this == rhs ) || path_ < rhs.path_;
        }

        bool directory_entry::operator==( directory_entry const & rhs ) const
        {
            return path_ == rhs.path_;
        }

        bool directory_entry::operator!=( directory_entry const & rhs ) const
        {
            return !( *this == rhs );
        }

        bool directory_entry::operator>( directory_entry const & rhs ) const
        {
            return path_ > rhs.path_;
        }

        bool directory_entry::operator>=( directory_entry const &rhs ) const
        {
            return !( *this < rhs );
        }

        directory_iterator::directory_iterator( path const & p ) noexcept : full_path{ p }, entry{},
            search_handle{ INVALID_HANDLE_VALUE }
        {
            WIN32_FIND_DATAW find_data{};
            if ( full_path.native().back() != L'*' ) {
                search_handle = FindFirstFileW( ( full_path / path{ "\\*" } ).c_str(), &find_data );
            } else {
                search_handle = FindFirstFileW( full_path.c_str(), &find_data );
            }
            if ( search_handle != INVALID_HANDLE_VALUE ) {
                while ( ( std::wcscmp( find_data.cFileName, L"." ) == 0
                    || std::wcscmp( find_data.cFileName, L".." ) == 0 )
                    && FindNextFileW( search_handle, &find_data ) != 0 );
                entry.assign( full_path / path{ find_data.cFileName } );
            }
        }
        directory_entry const & directory_iterator::operator*() const
        {
            return entry;
        }

        // directory_iterator must satisfy the requirements for input iterator
        bool directory_iterator::operator==( directory_iterator const & iter ) const
        {
            return ( search_handle == iter.search_handle ) && ( entry == iter.entry );
        }

        bool directory_iterator::operator != ( directory_iterator const & iter ) const
        {
            return !( *this == iter );
        }

        directory_iterator& directory_iterator::operator++()
        {
            if ( search_handle != INVALID_HANDLE_VALUE ) {
                WIN32_FIND_DATAW find_data{};
                if ( FindNextFileW( search_handle, &find_data ) != 0 ) {
                    while ( ( std::wcscmp( find_data.cFileName, L"." ) == 0
                        || std::wcscmp( find_data.cFileName, L".." ) == 0 )
                        && FindNextFileW( search_handle, &find_data ) != 0 );
                    entry.assign( full_path / path{ find_data.cFileName } );
                } else {
                    FindClose( search_handle );
                    search_handle = INVALID_HANDLE_VALUE;
                    entry.assign( path{} );
                }
            } else {
                entry = path{};
            }
            return *this;
        }

        directory_iterator& directory_iterator::begin() noexcept
        {
            return *this;
        }

        directory_iterator directory_iterator::end() noexcept
        {
            return directory_iterator{};
        }

        directory_iterator const & directory_iterator::cbegin() const
        {
            return *this;
        }

        directory_iterator const directory_iterator::cend() const
        {
            return directory_iterator{};
        }
    }
}

#undef FSERROR_TRY_CATCH
