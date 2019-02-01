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
#include <system_error>
#include <codecvt>
#include <locale>
#include <type_traits>
#include <cstdlib>


#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/stat.h>
#endif // _WIN32

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#define TINY_CHAR16_T int16_t
#define TINY_CHAR32_T int32_t

#else
#define TINY_CHAR16_T char16_t
#define TINY_CHAR32_T char32_t
#endif // _MSC_VER

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
            path() = default;
            ~path() = default;
            path( path && p ) = default;
            path( path const & p );

            explicit path( std::wstring const & pathname );
            explicit path( std::string const & pathname );
            explicit path( std::u16string const & pathname );
            explicit path( std::u32string const & pathname );
            explicit path( wchar_t const * pathname );
            explicit path( char const * pathname );

            /*
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
            */

            std::u32string u32string() const;
            std::u16string u16string() const;
            std::wstring wstring() const;
            std::string string() const;
            /*
            bool empty() const;
            bool has_root_name() const;
            bool has_root_directory() const;
            bool has_filename() const;
            bool is_absolute() const;
            bool is_relative() const; */
        private:
            std::string pathname_; // basic-string
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

        std::error_code make_error_code( filesystem_error_codes code );

        namespace details {
            struct invalid_filename : public std::runtime_error {
                invalid_filename( char const * error_message ) : std::runtime_error{ error_message }{}
            };
            struct name_too_long : public std::runtime_error {
                name_too_long( char const * error_message ) : std::runtime_error{ error_message }{}
            };

            std::string get_windows_error( DWORD error_code );

            file_time_type Win32FiletimeToChronoTime( LPFILETIME pFiletime );
            FILETIME ChronoTimeToWin32Filetime( file_time_type const & ftt );

            template<typename T>
            using str_t = std::basic_string<T, std::char_traits<T>>;

            template<typename From, typename To>
            void convert_to( str_t<From> const & from, str_t<To> & to );
            // str_t<wchar_t && char16_t && char32_t> --> str_t<char>
            template<typename From>
            void convert_to( str_t<From> const & from, str_t<char> & to )
            {
                auto & converter = std::use_facet<std::codecvt<From, char, std::mbstate_t>>( std::locale() );
                std::mbstate_t state{};
                to.resize( from.size() * converter.max_length() );
                typename std::add_pointer<typename std::add_const<From>::type>::type f{};
                char* t{};
                converter.out( state, &from[ 0 ], &from[ from.size() ], f, &to[ 0 ], &to[ to.size() ], t );
            }

            // str_t<char> --> str_t<char16_t, char32_t>
            template<typename T, typename = typename std::enable_if<
                    std::is_same<T, TINY_CHAR16_T>::value || std::is_same<T, TINY_CHAR32_T>::value>::type>
            void convert_to( str_t<char> const & from, str_t<T> & to )
            {
                using ToType = typename std::conditional<std::is_same<T, str_t<TINY_CHAR16_T>>::value,
                    std::codecvt_utf8_utf16<T>, std::codecvt_utf8<T>>::type;
                to = std::wstring_convert<ToType, T>{}.from_bytes( from );
            }

            // str_t<char> --> str_t<wchar_t>
            template<>
            void convert_to( str_t<char> const & from, str_t<wchar_t> & to )
            {
                std::size_t const len = std::mbstowcs( nullptr, &from[ 0 ], from.size() );
                if ( len == ( std::size_t ) - 1 ) return;
                to.resize( len + 1 );
                std::mbstowcs( &to[ 0 ], from.c_str(), from.size() );
            }
        }
        class filesystem_error : public std::system_error {
        public:
            filesystem_error( std::string const & what, std::error_code ec ) : filesystem_error{ what, {}, ec } {}
            filesystem_error( std::string const & what, path const & p, std::error_code ec ) :
                filesystem_error{ what, p, {}, ec } {}
            filesystem_error( std::string const & what, path const & p1, path const & p2, std::error_code ec ) :
                std::system_error{ ec, what }, first_path{ p1 }, second_path{ p2 }{}

            path const & path1() const { return first_path; }
            path const & path2() const { return second_path; }
            using std::system_error::what;

        private:
            path const first_path;
            path const second_path;
        };

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
        //supposed to be in <ntifs.h> but weirdly this header isn't available to use but MSDN shows what it looks like
        struct REPARSE_DATA_BUFFER
        {
            ULONG  ReparseTag;
            USHORT  ReparseDataLength;
            USHORT  Reserved;
            union {
                struct {
                    USHORT  SubstituteNameOffset;
                    USHORT  SubstituteNameLength;
                    USHORT  PrintNameOffset;
                    USHORT  PrintNameLength;
                    ULONG  Flags;
                    WCHAR  PathBuffer[ 1 ];
                } SymbolicLinkReparseBuffer;
                struct {
                    USHORT  SubstituteNameOffset;
                    USHORT  SubstituteNameLength;
                    USHORT  PrintNameOffset;
                    USHORT  PrintNameLength;
                    WCHAR  PathBuffer[ 1 ];
                } MountPointReparseBuffer;
                struct {
                    UCHAR  DataBuffer[ 1 ];
                } GenericReparseBuffer;
            };
        };
        enum class permissions_status : int {
            replace_bits = 0,
            add_bits = 1,
            remove_bits = 2,
            follow_symlinks = 4
        };
    }
}

namespace std
{
    template<>
    struct is_error_code_enum<tinydircpp::fs::filesystem_error_codes> : public true_type
    {
    };
}
