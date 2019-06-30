#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace app {

    struct context {
        context( ) = default;

        std::string build_path;

        std::string cxx_compiller = "g++";
        std::vector<std::string> cxx_extensions = {".cpp", ".cxx", ".cc"};
        std::vector<std::string> cxx_compile_options = {"-c", "-pipe"};
    };

} // namespace app
