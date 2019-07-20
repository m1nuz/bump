#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <chrono>

namespace app {

    namespace bs {

        enum class target_build_type { BINARY_UNKNOWN, BINARY_APPLICATION, STATIC_LIBRARY, SHARED_LIBRARY };

        using clock_type = std::chrono::steady_clock;

        struct target;

        struct target {
            target( ) = default;

            std::string name;
            std::string output;
            std::string output_path;
            target_build_type type;

            std::vector<std::string> include_directories;

            std::vector<std::string> sources;
            std::vector<target> sub_targets; // dependent targets
            std::vector<std::string> compiled_files;
            std::vector<std::string> link_libraries;
        };

        struct context {
            context( ) = default;

            struct {
                std::string support_cxx_17 = "-std=c++17";
                std::string verbosity_warnings_all = "-pedantic -Wall";
            } flags;

            std::string base_path;
            std::string build_path;

            clock_type::time_point build_start;
            clock_type::time_point build_end;

            std::string cxx_compiller = "g++";
            std::vector<std::string> cxx_extensions = {".cpp", ".cxx", ".cc"};
            std::vector<std::string> cxx_compile_options = {"-c", "-pipe"};

            std::vector<std::string> include_directories;

            std::vector<target> build_targets;
        };

    } // namespace bs

} // namespace app
