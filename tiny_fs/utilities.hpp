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


#pragma once
#include <stdexcept>
#include <chrono>
#include <ctime>

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/stat.h>
#endif // _WIN32

#ifndef PRIVATE_IMPL
#define PRIVATE_IMPL
#endif // !PRIVATE_IMPL

namespace tinydircpp
{
    namespace fs {
        
        using file_time_type = std::chrono::time_point<std::chrono::system_clock>;
        using space_info = struct {
            std::uintmax_t available;
            std::uintmax_t capacity;
            std::uintmax_t free;
        };

        class path {
        public:
            path() : path( "" ) {}
            explicit path( std::string const & pathname ) : pathname_{ pathname } {}
            explicit path( char const * pathname ) : pathname_{ pathname } {}
            path( path const & name ) : pathname_{ name.pathname_ } {}
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
            std::string string() const;

            bool empty() const;
            bool has_root_name() const;
            bool has_root_directory() const;
            bool has_filename() const;
            bool is_absolute() const;
            bool is_relative() const;
        private:
            std::string pathname_;
        };

        enum class filesystem_error_codes
        {
            directory_name_unobtainable = 0x80,
            unknown_io_error,
            handle_not_opened,
            could_not_obtain_size,
            could_not_obtain_time,
            hardlink_count_error,
            invalid_set_file_pointer,
            set_writetime_error
        };
        std::error_code make_error_code( filesystem_error_codes code ) {
            return std::error_code( static_cast< int >( code ), std::generic_category() );
        }

        namespace details PRIVATE_IMPL {
            struct invalid_filename : public std::runtime_error {
                invalid_filename( char const * error_message ) : std::runtime_error{ error_message }{}
            };
            struct name_too_long : public std::runtime_error {
                name_too_long( char const * error_message ) : std::runtime_error{ error_message }{}
            };

            std::string get_windows_error( DWORD error_code )
            {
                LPSTR buffer = nullptr;
                DWORD length = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
                    nullptr, error_code, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPSTR)&buffer, 0, 
                    nullptr );
                if ( length == 0 || buffer == nullptr ) { // there was an issue getting the error message from the system
                    return{};
                }
                std::string const message{ buffer };
                LocalFree( buffer );
                return message;
            }

            file_time_type Win32FiletimeToChronoTime( LPFILETIME pFiletime )
            {
                ULARGE_INTEGER ll_now{};
                ll_now.LowPart = pFiletime->dwLowDateTime;
                ll_now.HighPart = pFiletime->dwHighDateTime;
                std::time_t const epoch_time = ll_now.QuadPart / 10'000'000 - 11'644'473'600U;
                return std::chrono::system_clock::from_time_t( epoch_time );
            }

            FILETIME ChronoTimeToWin32Filetime( file_time_type const & ftt )
            {
                ULONGLONG const unix_epoch_time = ftt.time_since_epoch().count();
                ULARGE_INTEGER ll_now{};
                ll_now.QuadPart = 10'000'000 * ( unix_epoch_time + 11'644'473'600U );
                return FILETIME{ ll_now.LowPart, ll_now.HighPart };
            }
        }
        class filesystem_error : public std::system_error {
        public:
            filesystem_error( std::string const & what, std::error_code ec ) : filesystem_error{ what, {}, ec }{}
            filesystem_error( std::string const & what, path const & p, std::error_code ec ) :
                filesystem_error{ what, p, {}, ec } {}
            filesystem_error( std::string const & what, path const & p1, path const & p2, std::error_code ec ):
                std::system_error{ ec, what }, first_path{ p1 }, second_path{ p2 }{}

            path const & path1() const { return first_path;  }
            path const & path2() const { return second_path; }
            using std::system_error::what;

        private:
            path const first_path{};
            path const second_path{};
        };
    }

    enum class file_type : int {
        none = 0,
        not_found = -1,
        regular = 1,
        directory,
        symlink,
        block,
        character,
        fifo,
        socket,
        unknown
    };

    enum class copy_options : int {
        none = 0,
        skip_existing = 0x1,
        overwrite_existing = 0x2,
        update_existing = 0x4,
        recursive = 0x8,
        copy_symlinks = 0x10,
        skip_symlinks = 0x20,
        directories_only = 0x40,
        create_symlinks = 0x80,
        create_hardlinks = 0x100
    };

    enum class perms : int {
        none = 0,
        owner_read = 256,
        owner_write = 128,
        owner_exec = 64,
        owner_all = owner_read | owner_write | owner_exec,
        group_read = 32,
        group_write = 16,
        group_exec = 8,
        group_all = group_read | group_write | group_exec,
        others_read = 4,
        others_write = 2,
        others_exec = 1,
        others_all = others_read | others_write | others_exec,
        all = 511,
        set_uid = 2048,
        set_gid = 1024,
        mask = 512,
        unknown = 0xFFFF,
        symlink_perms = 0x4000
    };

    enum class permissions_status : int {
        replace_bits = 0,
        add_bits = 1,
        remove_bits = 2,
        follow_symlinks = 4
    };
}

namespace std
{
    template<>
    struct is_error_code_enum<tinydircpp::fs::filesystem_error_codes>: public true_type
    {
    };
}
