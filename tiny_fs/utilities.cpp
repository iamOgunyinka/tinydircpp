#include "utilities.hpp"

namespace tinydircpp
{
    namespace fs
    {
        path::path( std::string const & pathname ) : pathname_{}
        {
            details::convert_to( pathname, pathname_ );
        }
        path::path( char const *p ) : path{ std::string{ p } }
        {
        }
        path & path::operator=( path const & p )
        {
            if ( this != &p ) {
                pathname_ = p.pathname_;
            }
            return *this;
        }
        path & path::operator=( path && p )
        {
            this->pathname_ = std::move( p.pathname_ );
            return *this;
        }
        path::path( std::wstring const & pathname ) : pathname_{ pathname }
        {
        }
        path::path( std::u16string const & pathname ) : pathname_( pathname.begin(), pathname.end() )
        {
        }

        path::path( std::u32string const & pathname ) : pathname_{}
        {
            details::convert_to( pathname, pathname_ );
        }

        path::path( wchar_t const * pathname ) : pathname_{ pathname }
        {
        }

        path & path::operator/=( path const & p )
        {
            if ( p.empty() ) return *this;
            if ( pathname_.empty() && !p.empty() ) {
                pathname_ = p.native();
                return *this;
            }

            path::string_type& new_path_name = pathname_;
            path::string_type rel_path_name = p.native();

            if ( !IS_DIR_SEPARATORW( new_path_name.back() ) ) {
                new_path_name += ( IS_DIR_SEPARATORW( rel_path_name.front() ) ? L"" : L"\\" ) + rel_path_name;
            } else {
                int const index = rel_path_name.find_first_not_of( L"\\." );
                new_path_name += rel_path_name.substr( index, path::string_type::npos );
            }
            return *this;
        }

        path path::extension() const
        {
            auto const filename = this->filename().string();
            if ( filename.empty() ) return path{};
            auto pos = filename.rfind( "." );
            if ( filename.size() == 1 || filename.size() == 2 || pos == std::string::npos ) return path{};
            return path{ filename.substr( pos ) };
        }

        path path::filename() const
        {
            std::wstring::size_type const loc = pathname_.rfind( WSLASH );
            if ( loc == 0 || loc == pathname_.size() - 1 ) return{};
            return path{ pathname_.cbegin() + ( loc == std::wstring::npos ? 0 : loc + 1 ), pathname_.cend() };
        }

        std::u32string path::u32string() const
        {
            std::u32string ret{};
            details::convert_to( pathname_, ret );
            return ret;
        }

        std::u16string path::u16string() const
        {
            return std::u16string( pathname_.begin(), pathname_.end() );
        }

        std::wstring path::wstring() const
        {
            return pathname_;
        }

        std::string path::string() const
        {
            std::string temp{};
            details::convert_to( pathname_, temp );
            return temp;
        }

        path operator/( path const & p, path const & rel_path )
        {
            if ( p.empty() ) return rel_path;
            if ( rel_path.empty() ) return p;

            path::string_type new_path_name = p.native(), rel_path_name = rel_path.native();
            if ( !IS_DIR_SEPARATORW( new_path_name.back() ) ) {
                new_path_name += ( IS_DIR_SEPARATORW( rel_path_name.front() ) ? L"" : L"\\" ) + rel_path_name;
            } else {
                int const index = rel_path_name.find_first_not_of( L"\\." );
                new_path_name += rel_path_name.substr( index, path::string_type::npos );
            }
            return path{ new_path_name };
        }

        std::error_code make_error_code( filesystem_error_codes code )
        {
            return std::error_code( static_cast< int >( code ), std::generic_category() );
        }

        file_time_type fstime_to_stdtime( filesystem_time const & time ) noexcept( false )
        {
            LPSYSTEMTIME const lp_systime = ( LPSYSTEMTIME ) &time;
            FILETIME filetime{};
            if ( SystemTimeToFileTime( lp_systime, &filetime ) == 0 )
                throw std::runtime_error( details::get_windows_error( GetLastError() ) );
            return details::Win32FiletimeToChronoTime( filetime );
        }

        filesystem_time stdtime_to_fstime( file_time_type const & time ) noexcept( false )
        {
            auto const result = details::ChronoTimeToWin32Filetime( time );
            SYSTEMTIME sys_time{};
            if ( FileTimeToSystemTime( &result, &sys_time ) == 0 )
                throw std::runtime_error( details::get_windows_error( GetLastError() ) );
            return *( filesystem_time* ) &sys_time;
        }

        namespace details
        {
            std::string get_windows_error( DWORD error_code )
            {
                LPSTR buffer = nullptr;
                DWORD length = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    nullptr, error_code, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPSTR ) &buffer, 0,
                    nullptr );
                if ( length == 0 || buffer == nullptr ) { // there was an issue getting the error message from the system
                    return{};
                }
                std::string const message{ buffer };
                LocalFree( buffer );
                return message;
            }

            file_time_type Win32FiletimeToChronoTime( FILETIME const &pFiletime )
            {
                ULARGE_INTEGER ll_now{};
                ll_now.LowPart = pFiletime.dwLowDateTime;
                ll_now.HighPart = pFiletime.dwHighDateTime;
                std::time_t const epoch_time = ( ll_now.QuadPart / 10000000 ) - 11644473600U;
                return std::chrono::system_clock::from_time_t( epoch_time );
            }

            FILETIME ChronoTimeToWin32Filetime( file_time_type const & ftt )
            {
                auto const unix_epoch_time = std::chrono::system_clock::to_time_t( ftt );
                ULARGE_INTEGER ll_now{};
                ll_now.QuadPart = 10000000 * ( unix_epoch_time + 11644473600U );
                return FILETIME{ ll_now.LowPart, ll_now.HighPart };
            }

            void convert_to( str_t<wchar_t> const & from, str_t<char32_t> & to )
            {
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter{};
                std::string const str( converter.to_bytes( from ) );
                std::wstring_convert<std::codecvt_utf8<unsigned int>, unsigned int> c32_converter{};
                auto const new_to = c32_converter.from_bytes( str );
                to = reinterpret_cast< char32_t const* >( new_to.data() );
            }

            void convert_to( str_t<char32_t> const & from, str_t<wchar_t> & to )
            {
                std::wstring_convert<std::codecvt_utf8<unsigned int>, unsigned int> c32_converter{};
                auto const str( c32_converter.to_bytes( reinterpret_cast< unsigned int const* >( from.data() ) ) );
                to = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes( str );
            }

            void convert_to( str_t<wchar_t> const & from, str_t<char> & to )
            {
                auto & converter = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>( std::locale() );
                std::mbstate_t state{};
                to.resize( from.size() * converter.max_length() );
                typename std::add_pointer<typename std::add_const<wchar_t>::type>::type f{};
                char* t{};
                converter.out( state, &from[ 0 ], &from[ from.size() ], f, &to[ 0 ], &to[ to.size() ], t );
            }

            void convert_to( str_t<char16_t> const & from, str_t<char> & to )
            {
                auto & converter = std::use_facet<std::codecvt<unsigned short, char, std::mbstate_t>>( std::locale() );
                std::mbstate_t state{};
                to.resize( from.size() * converter.max_length() );
                typename std::add_pointer<typename std::add_const<unsigned short>::type>::type f{};
                char* t{};
                auto p = reinterpret_cast< unsigned short const * >( from.data() );
                converter.out( state, p, p + from.size(), f, &to[ 0 ], &to[ to.size() ], t );
            }

            void convert_to( str_t<char32_t> const & from, str_t<char> & to )
            {
                auto & converter = std::use_facet<std::codecvt<unsigned int, char, std::mbstate_t>>( std::locale() );
                std::mbstate_t state{};
                to.resize( from.size() * converter.max_length() );
                typename std::add_pointer<typename std::add_const<unsigned int>::type>::type f{};
                char* t{};
                auto p = reinterpret_cast< unsigned int const * >( from.data() );
                converter.out( state, p, p + from.size(), f, &to[ 0 ], &to[ to.size() ], t );
            }
            // str_t<char> --> str_t<char16_t>
            void convert_to( str_t<char> const & from, str_t<char16_t> & to )
            {
                auto result = std::wstring_convert<std::codecvt_utf8_utf16<unsigned short>,
                    unsigned short>{}.from_bytes( from );
                to = str_t<char16_t>( reinterpret_cast< char16_t const* >( result.data() ) );
            }

            void convert_to( str_t<char> const & from, str_t<char32_t> & to )
            {
                auto result = std::wstring_convert<std::codecvt_utf8<unsigned int>, unsigned int>{}.from_bytes( from );
                to = str_t<char32_t>( reinterpret_cast< char32_t const* >( result.data() ) );
            }

            void convert_to( str_t<char> const &from, str_t<char> & to )
            {
                to = from;
            }

            void convert_to( str_t<char> const & from, str_t<wchar_t> & to )
            {
                std::size_t const len = std::mbstowcs( nullptr, &from[ 0 ], from.size() );
                if ( len == ( std::size_t ) - 1 ) throw std::bad_exception{};
                to.resize( len );
                if ( std::mbstowcs( &to[ 0 ], from.c_str(), from.size() ) == ( std::size_t ) - 1 ) {
                    throw std::runtime_error( "Unable to convert std::string to std::wstring" );
                }
            }
        }

    }
}
