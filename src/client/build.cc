#include <algorithm>
#include <filesystem>
#include <string_view>

#include <config.h>
#include <journal.h>
#include <yaml-cpp/yaml.h>

#include <string_utils.h>
#include "context.h"

namespace fs = std::filesystem;

namespace app {


    namespace commands {

        auto sys_exec( std::string_view cmd ) {
            using namespace std;

            array<char, 512> buffer;
            string result;
            shared_ptr<FILE> pipe( popen( cmd.data( ), "r" ), pclose );
            if ( !pipe )
                return result;

            while ( !feof( pipe.get( ) ) ) {
                if ( fgets( data( buffer ), size( buffer ), pipe.get( ) ) != nullptr )
                    result += data( buffer );
            }

            return result;
        }

        auto get_directory_files( const std::string &dir, const std::vector<std::string> &extensions ) {
            using namespace std;

            vector<string> files;
            for ( auto &p : fs::recursive_directory_iterator( dir ) ) {
                if ( fs::is_regular_file( p ) ) {
                    if ( extensions.empty( ) ||
                         find( extensions.begin( ), extensions.end( ), p.path( ).extension( ).string( ) ) != extensions.end( ) ) {
                        files.push_back( p.path( ).string( ) );
                    }
                }
            }

            return files;
        }

        static auto build_target( context &ctx, const YAML::Node &conf, const std::string_view target_path,
                                  const std::string_view build_path ) -> bool {
            using namespace std;

            const auto initial_dir = fs::current_path( );

            if ( !build_path.empty( ) ) {
                fs::current_path( build_path );

                const auto target_name = conf["name"].as<string>( );

                vector<string> target_sources;
                vector<string> target_files;

                if ( conf["sources"].IsScalar( ) ) {
                    const auto sources_value = conf["sources"].as<string>( );
                    if ( sources_value == "all" ) {
                        const auto &extensions = ctx.cxx_extensions;
                        target_sources = get_directory_files( string{target_path} + fs::path::preferred_separator + "src" +
                                                                  fs::path::preferred_separator + target_name,
                                                              extensions );
                    }

                    target_files = target_sources;
                } else if ( conf["sources"].IsSequence( ) ) {
                    target_sources = conf["sources"].as<vector<string>>( );
                } else {
                    LOG_ERROR( APP_TAG, "Nothing to build for target %1", target_name );
                }

                vector<string> compiled_files;

                // Compile files
                for ( const auto &f : target_files ) {
                    fs::path file_path{f};
                    const auto command = ctx.cxx_compiller + " -c -pipe " + f;

                    LOG_INFO( APP_TAG, "Compile: %1", command );

                    const auto res = sys_exec( command );
                    if ( res.empty( ) ) {
                    }

                    //const auto compiled_file = fs::relative(file_path, fs::path{target_path}).replace_extension( ".o" );
                    const auto compiled_file = build_path / file_path.filename().replace_extension( ".o" );

                    compiled_files.push_back( compiled_file );
                }

                namespace su = string_utils;

                const auto all_compiled = su::join( compiled_files, " " );

                // Link files
                {
                    const auto command = ctx.cxx_compiller + " -o " + target_name + " " + all_compiled;

                    const auto res = sys_exec( command );
                    if ( res.empty( ) ) {
                    }

                    LOG_INFO( APP_TAG, "Linked: %1", target_name );
                }

                fs::current_path( initial_dir );

                return true;
            }

            LOG_ERROR( APP_TAG, "%1", "No build directory" );

            return false;
        }

        auto build_all( context &ctx, std::string_view arguments ) -> bool {

            const auto target_path = fs::current_path( ).string( );
            const auto conf_path = target_path + fs::path::preferred_separator + DEFAULT_BUMP_FILE;

            if ( fs::exists( conf_path ) ) {
                auto conf = YAML::LoadFile( conf_path );

                const auto build_path = target_path + fs::path::preferred_separator + "build";

                fs::create_directory( build_path );

                const auto project_name = conf["project"].as<std::string>( );

                LOG_INFO( APP_TAG, "Build '%1'", project_name );

                auto root_targets = conf["build"];
                for ( const auto &target : root_targets ) {
                    build_target( ctx, target, target_path, build_path );
                }

                LOG_INFO( APP_TAG, "Build '%1' done", project_name );

                return true;
            }

            LOG_ERROR( APP_TAG, "%1", "No config for build" );

            return false;
        }

    } // namespace commands

} // namespace app
