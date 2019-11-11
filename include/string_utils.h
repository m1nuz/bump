#pragma once

#include <algorithm>
#include <charconv>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace string_utils {

    template <typename Range, typename Value = typename Range::value_type>
    inline std::string join( const Range &elements, const std::string_view delimiter ) {
        std::ostringstream os;
        auto b = begin( elements ), e = end( elements );

        if ( b != e ) {
            std::copy( b, prev( e ), std::ostream_iterator<Value>( os, delimiter.data( ) ) );
            b = prev( e );
        }

        if ( b != e ) {
            os << *b;
        }

        return os.str( );
    }

    inline std::vector<std::string> split( const std::string_view text, const std::string_view delimiter ) {
        std::vector<std::string> tokens;

        auto start = text.find_first_not_of( delimiter );
        auto end = std::string::size_type{0};

        while ( ( end = text.find_first_of( delimiter, start ) ) != std::string::npos ) {
            tokens.push_back( std::string( text.substr( start, end - start ) ) );
            start = text.find_first_not_of( delimiter, end );
        }

        if ( start != std::string::npos )
            tokens.push_back( std::string( text.substr( start ) ) );

        return tokens;
    }

    inline std::optional<int> to_int( const std::string_view str ) noexcept {
        int result;
        if ( auto [p, ec] = std::from_chars( str.data( ), str.data( ) + str.size( ), result ); ec == std::errc( ) )
            return result;

        return {};
    }

    inline std::string to_upper_copy( const std::string_view s ) {
        std::string result;
        result.reserve( s.size( ) );
        std::transform( s.begin( ), s.end( ), std::back_inserter( result ), static_cast<int ( * )( int )>( std::toupper ) );
        return result;
    }

} // namespace string_utils
