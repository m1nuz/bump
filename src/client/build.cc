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

        static auto compile_target( bs::context &ctx, const std::string_view target_path, const bs::target &target ) {
            using namespace std;
            namespace su = string_utils;
            namespace au = algorithm_utils;

            fs::current_path( ctx.build_path );

            vector<string> compiled_files;

            size_t file_num = 0;
            for ( const auto &f : target.sources ) {
                const auto compiled_file = fs::relative( fs::path{f}, fs::path{target_path} ).replace_extension( ".o" );

                const auto work_path = ctx.build_path / fs::path( compiled_file ).remove_filename( );

                auto work_dir = work_path.string( );
                work_dir.pop_back( );

                fs::create_directories( work_dir );
                fs::current_path( work_dir );

                vector all_command_options{ctx.cxx_compiller};

                if ( target.type == bs::target_build_type::BINARY_APPLICATION ) {
                }

                if ( target.type == bs::target_build_type::STATIC_LIBRARY ) {
                    all_command_options.push_back( "-fPIC" );
                }

                if ( target.type == bs::target_build_type::SHARED_LIBRARY ) {
                    all_command_options.push_back( "-fPIC" );
                }

                au::join_copy( all_command_options, ctx.cxx_compile_options );
                all_command_options.push_back( f );

                const auto command = su::join( all_command_options, strings::WHITESPACE );

                file_num++;

                LOG_MESSAGE( APP_TAG, "[%1/%2] Compile: '%3'", file_num, target.sources.size( ), f );

                const auto res = shell::execute( command );
                if ( res.empty( ) ) {
                }

                compiled_files.push_back( compiled_file );
            }

            return compiled_files;
        }

        static auto get_library_link_name( std::string_view name ) {
            std::string link_name{name};
            link_name.replace( 0, 3, "" );
            const auto found = link_name.find_last_of( '.' );
            if ( found != std::string::npos )
                link_name.replace( found, 3, "" );

            return "-l" + link_name;
        }

        static auto link_target( bs::context &ctx, bs::target &target ) -> bool;

        static auto link_target( bs::context &ctx, bs::target &target ) -> bool {
            using namespace std;
            namespace su = string_utils;
            namespace au = algorithm_utils;

            fs::current_path( ctx.build_path );

            target.output_path = ctx.build_path;

            if ( !target.sub_targets.empty( ) ) {
                for ( auto &t : target.sub_targets ) {
                    link_target( ctx, t );
                }
            }

            const auto all_compiled = su::join( target.compiled_files, strings::WHITESPACE );

            vector<string> all_command_options;
            if ( target.type == bs::target_build_type::BINARY_APPLICATION ) {
                all_command_options.push_back( ctx.cxx_compiller );
                all_command_options.push_back( all_compiled );

                vector<string> linker_paths;

                for ( const auto &st : target.sub_targets ) {
                    if ( st.type == bs::target_build_type::STATIC_LIBRARY || st.type == bs::target_build_type::SHARED_LIBRARY ) {

                        const auto link_path = "-L" + st.output_path;

                        auto it = std::find( linker_paths.begin( ), linker_paths.end( ), link_path );
                        if ( it == linker_paths.end( ) )
                            linker_paths.push_back( "-L" + st.output_path );
                    }
                }

                au::join_move(all_command_options, linker_paths);

                for ( const auto &st : target.sub_targets ) {
                    if ( st.type == bs::target_build_type::STATIC_LIBRARY || st.type == bs::target_build_type::SHARED_LIBRARY ) {
                        all_command_options.push_back( get_library_link_name( st.output ) );
                    }
                }

                all_command_options.push_back( "-o" );
                all_command_options.push_back( target.output );
            }

            if ( target.type == bs::target_build_type::STATIC_LIBRARY ) {
                all_command_options.push_back( "ar rcs" );
                all_command_options.push_back( target.output );
                all_command_options.push_back( all_compiled );
            }

            if ( target.type == bs::target_build_type::SHARED_LIBRARY ) {
                all_command_options.push_back( ctx.cxx_compiller );
                all_command_options.push_back( "-shared" );
                all_command_options.push_back( all_compiled );
                all_command_options.push_back( "-o" );
                all_command_options.push_back( target.output );
            }

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

            if ( !target.sub_targets.empty( ) ) {
                for ( auto &t : target.sub_targets ) {
                    build_target( ctx, t );
                }
            }

            if ( !target.sources.empty( ) ) {
                const auto compiled_files = compile_target( ctx, initial_dir.string( ), target );

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

                    fs::create_directory( ctx.build_path, err );

                    if ( !err ) {
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
