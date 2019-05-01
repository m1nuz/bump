#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace app {

    struct context {
        context( ) = default;

        std::string cxx_compiller = "/usr/bin/g++";
        std::vector<std::string> cxx_extensions = {".cpp", ".cxx", ".cc"};
    };

} // namespace app
