#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace app {

    namespace bs {

        enum class target_build_type {
            BINARY_UNKNOWN,
            BINARY_APPLICATION,
            STATIC_LIBRARY,
            SHARED_LIBRARY
        };

        struct target;

        struct target {
            target( ) = default;

            std::string name;
            target_build_type type;

            std::vector<std::string> sources;
            std::vector<target> sub_targets; // dependent targets
            std::vector<std::string> compiled_files;
        };

        struct context {
            context( ) = default;

            std::string build_path;

            std::string cxx_compiller = "g++";
            std::vector<std::string> cxx_extensions = {".cpp", ".cxx", ".cc"};
            std::vector<std::string> cxx_compile_options = {"-c", "-pipe"};

            std::vector<target> build_targets;
        };

    } // namespace bs

} // namespace app
