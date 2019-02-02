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

TEST_CASE( "Testing tinydircpp::fs free functions", "Success and failures" )
{
    namespace fs = tinydircpp::fs;
    using fs::status;
    using fs::path;
    using fs::file_type;

    std::error_code ec{};

    auto path_1 = path{ "C:\\Users" }, 
        symlink_path = path{ "C:\\Users\\Josh\\Desktop\\CC" }, // a symlink created by Window's mklink /D
        cpp_file_path = path{ L"C:\\Users\\Josh\\Desktop\\push_back.cpp" },
        system_file_path = path{ u"C:\\Windows\\System32\\ACCTRES.dll" }, 
        random_dir_path = path{ U"C:\\Users\\Public\\Foxit" };

    auto const a = status( path_1, ec );
    auto const b = status( symlink_path, ec );
    auto const c = status( cpp_file_path, ec );
    auto const d = status( system_file_path, ec );
    auto const e = status( random_dir_path );

    REQUIRE( a.type() == file_type::directory );
    REQUIRE( b.type() == file_type::symlink );
    REQUIRE( c.type() == file_type::regular );
    REQUIRE( d.type() == file_type::regular );
    REQUIRE( e.type() == file_type::not_found );
    REQUIRE( fs::read_symlink( system_file_path, ec ).string().empty() );
    REQUIRE( fs::read_symlink( cpp_file_path ).u32string() == U"" );
}