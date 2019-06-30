#pragma once

#include <string>

namespace common {

    constexpr char DEFAULT_SOURCE_DIR[] = "src";
    constexpr char DEFAULT_INCLUDE_DIR[] = "include";
    constexpr char DEFAULT_EXTERNAL_DIR[] = "external";
    constexpr char DEFAULT_PACKAGE_DIR[] = ".package";
    constexpr char DEFAULT_BUILD_DIR[] = "build";



} // namespace common

namespace strings {
    constexpr char WHITESPACE[] = " ";
}

namespace algorithm_utils {

    template <typename Range, typename Value = typename Range::value_type> Range &join_copy( Range &dest, const Range &src ) {
        dest.insert( std::end( dest ), std::begin( src ), std::end( src ) );

        return dest;
    }

    template <typename Range, typename Value = typename Range::value_type> Range &join_move( Range &dest, const Range &src ) {
        dest.insert( std::end( dest ), std::make_move_iterator( std::begin( src ) ), std::make_move_iterator( std::end( src ) ) );

        return dest;
    }

} // namespace algorithm_utils
