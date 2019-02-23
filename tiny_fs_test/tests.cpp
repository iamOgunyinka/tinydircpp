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

#define CATCH_CONFIG_MAIN

#include "external\catch.hpp"
#include "..\tiny_fs\tinydircpp.hpp"

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif // !_UNICODE

#include <list>
#include <deque>

namespace std
{
    template<>
    struct less<tinydircpp::fs::path> {
        bool operator()( tinydircpp::fs::path const &a, tinydircpp::fs::path const &b ) const
        {
            return a.native() < b.native();
        }
    };
}
TEST_CASE( "Testing tinydircpp::fs free functions", "Success and failures" )
{
    namespace fs = tinydircpp::fs;
    using fs::path;

    std::error_code ec{};

    auto const path_1 = path{ "C:\\Users" },
        symlink_path = path{ "C:\\Users\\Josh\\Desktop\\CC" }, // a symlink created by Window's mklink /D
        cpp_file_path = path{ L"C:\\Users\\Josh\\Desktop\\push_back.cpp" },
        system_file_path = path{ u"C:\\Windows\\System32\\ACCTRES.dll\\" },
        the_this = path{ u"." },
        rel_dir_path = path{ U"..\\Debug" };

    SECTION( "Testing fs::basename" )
    {
        auto const result = fs::basename( path_1 );
        REQUIRE( result.string() == std::string{ "Users" } );
        REQUIRE( fs::basename( symlink_path ).native() == L"CC" );
        REQUIRE( fs::basename( cpp_file_path ).native() == L"push_back.cpp" );
        REQUIRE( fs::basename( system_file_path ).native() == L"" );
        REQUIRE( fs::basename( path( "." ) ).native() != L".." );
        REQUIRE( fs::basename( the_this ).native() == L"." );
    }
    SECTION( "Testing fs::absolute && fs::abspath" )
    {
        REQUIRE( fs::abspath( path{ "." } ).native() != std::wstring{ L"C:\\Users" } );
        REQUIRE( fs::abspath( path{ ".." } ).u16string() != system_file_path.u16string() );
    }

    SECTION( "Testing fs::common_path" )
    {
        std::set<path> const paths{ path_1, symlink_path, cpp_file_path, system_file_path, the_this };

        auto const common_p = fs::common_path( paths.cbegin(), paths.cend() );
        REQUIRE( common_p.wstring() == L"C:\\" );
        REQUIRE( common_p.native() == L"C" );
    }
    SECTION( "Testing fs::common_prefix" )
    {
        std::list<path> paths{ path_1, symlink_path, cpp_file_path, the_this };
        auto const common_pref = fs::common_prefix( paths.begin(), paths.end() );
        REQUIRE( common_pref.wstring() == L"C:\\" );
        REQUIRE( common_pref.native() == L"C" );
    }
    SECTION( "testing is_absolute" )
    {
        REQUIRE( fs::is_abs( the_this ) );
        REQUIRE( fs::is_abs( rel_dir_path ) );
        REQUIRE( !fs::is_abs( cpp_file_path ) );
    }
    SECTION( "testing read_symlink and abspath" )
    {
        path const xpath{ symlink_path };
        auto const p = fs::read_symlink( xpath );
        REQUIRE( fs::abspath( p ).native() == L"." );
        REQUIRE( fs::is_abs( p ) );
    }
    SECTION( "iterating directory_entry" )
    {
        std::deque<fs::directory_entry> paths( fs::directory_iterator{ path_1 }, fs::directory_iterator{} );
        std::cout << paths.size() << " entries.\n";
        for ( auto const & c : paths ) {
            std::wcout << c.path().c_str();
            switch ( c.status().type() ) {
            case fs::file_type::directory:
                std::cout << " is a directory\n";
                break;
            case fs::file_type::regular:
                std::cout << " is a regular file\n";
                break;
            case fs::file_type::symlink:
                std::cout << " is a symbolic link targeting: " << fs::read_symlink( c.path() ).c_str() << "\n";
                break;
            case fs::file_type::unknown:
                std::cout << "'s type is unknown\n";
                break;
            default:
                std::cout << " is indeterminate\n";
            }
        }
    }
}