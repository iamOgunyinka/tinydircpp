#include "utilities.hpp"

namespace tinydircpp
{
    namespace fs
    {
        path::path( std::string const & pathname ) : pathname_{ pathname }
        {
        }
        path::path( char const *p ) : pathname_{ p }
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
            using std::swap;
            swap( *this, std::move( p ) );
            return *this;
        }
        path::path( std::wstring const & pathname ) : pathname_{}
        {
            details::convert_to( pathname, pathname_ );
        }
        path::path( std::u16string const & pathname ) : pathname_{}
        {
            details::convert_to( pathname, pathname_ );
        }

        path::path( std::u32string const & pathname ) : pathname_{}
        {
            details::convert_to( pathname, pathname_ );
        }
        path::path( wchar_t const * pathname ) : path{ std::wstring{ pathname } }
        {
        }
        path::path( path const & p ) : pathname_{ p.pathname_ } {
        }
        path path::filename() const
        {
            if ( pathname_.empty() ) return{};
            pathname_.rfind( SLASH );
        }
        std::u32string path::u32string() const
        {
            std::u32string ret{};
            details::convert_to( pathname_, ret );
            return ret;
        }
        std::u16string path::u16string() const
        {
            std::u16string ret{};
            details::convert_to( pathname_, ret );
            return ret;
        }

        std::wstring path::wstring() const
        {
            std::wstring ret{};
            details::convert_to( pathname_, ret );
            return ret;
        }

        std::string path::string() const
        {
            return pathname_;
        }

        std::error_code make_error_code( filesystem_error_codes code ){
            return std::error_code( static_cast< int >( code ), std::generic_category() );
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
        
    }
}
