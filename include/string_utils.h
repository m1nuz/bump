#pragma once

#include <algorithm>
#include <iterator>
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

} // namespace string_utils
