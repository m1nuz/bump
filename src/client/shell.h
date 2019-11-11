#pragma once

#include <string>

namespace sys {

    namespace shell {

        auto execute( std::string_view cmd ) -> std::string;
        auto run( std::string_view cmd ) -> int;

    } // namespace shell

} // namespace sys
