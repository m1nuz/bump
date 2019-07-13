#include <algorithm>
#include <filesystem>
#include <string_view>
#include <system_error>
#include <vector>

#include <config.h>
#include <journal.h>
#include <yaml-cpp/yaml.h>

#include <fs_utils.h>
#include <string_utils.h>

#include "common.h"
#include "context.h"
#include "shell.h"

namespace fs = std::filesystem;
namespace fs_utils = filesystem_utils;

namespace app {

    namespace bs {

        auto parse_conf( bs::context &ctx, std::string_view conf_path ) -> bool;

    } // namespace bs

    namespace commands {

        static auto compile_target( bs::context &ctx, const std::string_view target_path, const std::vector<std::string> &input_files ) {
            using namespace std;
            namespace su = string_utils;
            namespace au = algorithm_utils;

            fs::current_path( ctx.build_path );

            vector<string> compiled_files;

            size_t file_num = 0;
            for ( const auto &f : input_files ) {
                const auto compiled_file = fs::relative( fs::path{f}, fs::path{target_path} ).replace_extension( ".o" );

                const auto work_path = ctx.build_path / fs::path( compiled_file ).remove_filename( );

                auto work_dir = work_path.string( );
                work_dir.pop_back( );

                fs::create_directories( work_dir );
                fs::current_path( work_dir );

                vector all_command_options{ctx.cxx_compiller};
                au::join_copy( all_command_options, ctx.cxx_compile_options );
                all_command_options.push_back( f );

                const auto command = su::join( all_command_options, strings::WHITESPACE );

                file_num++;

                LOG_MESSAGE( APP_TAG, "[%1/%2] Compile: '%3'", file_num, input_files.size( ), f );

                const auto res = shell::execute( command );
                if ( res.empty( ) ) {
                }

                compiled_files.push_back( compiled_file );
            }

            return compiled_files;
        }

        static auto link_target( bs::context &ctx, bs::target &target ) {
            using namespace std;
            namespace su = string_utils;
            namespace au = algorithm_utils;

            fs::current_path( ctx.build_path );

            const auto all_compiled = su::join( target.compiled_files, strings::WHITESPACE );

            vector all_command_options{ctx.cxx_compiller};
            all_command_options.push_back( all_compiled );
            all_command_options.push_back( "-o" );
            all_command_options.push_back( std::string{target.name} );

            au::join_copy( all_command_options, target.link_libraries );

            const auto command = su::join( all_command_options, strings::WHITESPACE );

            const auto res = shell::execute( command );
            if ( res.empty( ) ) {
            }

            LOG_MESSAGE( APP_TAG, "Build output: '%1'", target.output );

            return true;
        }

        static auto build_target( bs::context &ctx, bs::target &target ) -> bool;

        static auto build_target( bs::context &ctx, bs::target &target ) -> bool {
            using namespace std;

            const auto initial_dir = fs::current_path( );

            bool build_result = false;

            if ( !target.sources.empty( ) ) {
                const auto compiled_files = compile_target( ctx, initial_dir.string( ), target.sources );

                target.compiled_files = compiled_files;

                build_result = !compiled_files.empty( );
            } else {
                build_result = false;
            }

            fs::current_path( initial_dir );

            return build_result;
        }

        auto build_all( bs::context &ctx, std::string_view arguments ) -> bool {
            (void)arguments;

            ctx.base_path = fs::current_path( ).string( );
            const auto conf_path = ctx.base_path + fs::path::preferred_separator + DEFAULT_BUMP_FILE;

            if ( bs::parse_conf( ctx, conf_path ) ) {
                if ( !ctx.build_targets.empty( ) ) {
                    std::error_code err;

                    const auto default_include_path = ctx.base_path + fs::path::preferred_separator + common::DEFAULT_INCLUDE_DIR;
                    if ( fs::exists( default_include_path, err ) ) {
                        ctx.cxx_compile_options.push_back( "-I" + default_include_path );
                    }

                    const auto default_external_path = ctx.base_path + fs::path::preferred_separator + common::DEFAULT_EXTERNAL_DIR;
                    if ( fs::exists( default_external_path, err ) ) {
                        ctx.cxx_compile_options.push_back( "-I" + default_external_path );
                    }

                    if ( !fs::create_directory( ctx.build_path, err ) ) {

                        for ( auto &t : ctx.build_targets ) {
                            build_target( ctx, t );
                        }

                        for ( auto &t : ctx.build_targets ) {
                            link_target( ctx, t );
                        }

                        return true;
                    }

                    LOG_ERROR( APP_TAG, "Could't create build directory %1", err.message( ) );

                    return false;
                }

                LOG_ERROR( APP_TAG, "%1", "No targets to build" );

                return false;
            }

            LOG_ERROR( APP_TAG, "Couldn't parse build config '%1'", conf_path );

            return false;
        }

    } // namespace commands

} // namespace app
