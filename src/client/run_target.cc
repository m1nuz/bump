#include <filesystem>

#include <config.h>
#include <journal.h>
#include <yaml-cpp/yaml.h>

#include "common.h"
#include "context.h"
#include "shell.h"

namespace fs = std::filesystem;

namespace app {

    namespace commands {

        auto sys_exec( std::string_view cmd ) -> std::string;
        auto sys_run( std::string_view cmd ) -> int;

        auto run_target( bs::context &ctx, const std::string_view target_name ) -> bool {
            using namespace std;

            const auto target_path = fs::current_path( ).string( );
            const auto conf_path = target_path + fs::path::preferred_separator + DEFAULT_BUMP_FILE;

            if ( fs::exists( conf_path ) ) {
                auto conf = YAML::LoadFile( conf_path );

                auto root_targets = conf["build"];

                for ( const auto &target : root_targets ) {
                    const auto current_target_type = target["type"].as<string>( );
                    const auto current_target_name = target["name"].as<string>( );

                    const auto is_app = current_target_type == "app";
                    if ( is_app && ( target_name == current_target_name || target_name.empty( ) ) ) {

                        LOG_DEBUG( APP_TAG, "Running target '%1'", current_target_name );

                        const auto err = sys::shell::run( ctx.build_path + fs::path::preferred_separator + current_target_name );
                        if ( err ) {
                            LOG_ERROR( APP_TAG, "Failed run target '%1'", current_target_name );
                            return false;
                        }

                        return true;
                    }
                }
            }

            return false;
        }

    } // namespace commands

} // namespace app
