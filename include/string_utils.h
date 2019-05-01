#pragma once

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace string_utils {

    template <typename Range, typename Value = typename Range::value_type>
    std::string join( Range const &elements, std::string_view delimiter ) {
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

} // namespace string_utils
