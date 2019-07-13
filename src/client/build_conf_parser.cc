#include <chrono>
#include <filesystem>

#include <fs_utils.h>
#include <journal.h>
#include <yaml-cpp/yaml.h>

#include "common.h"
#include "config.h"
#include "context.h"

namespace fs = std::filesystem;
namespace fs_utils = filesystem_utils;

namespace app {

    namespace bs {

        constexpr char CONF_PROJECT_NAME[] = "project";
        constexpr char CONF_CXX_FLAGS[] = "cxx-flags";
        constexpr char CONF_CXX_COMPILLER[] = "cxx-compiler";
        constexpr char CONF_TARGET_LIST[] = "build";
        constexpr char CONF_TARGET_NAME[] = "name";
        constexpr char CONF_TARGET_SOURES[] = "sources";
        constexpr char CONF_TARGET_TYPE[] = "type";
        constexpr char CONF_TARGET_LINK_LIBRARIES[] = "libaries";

        constexpr char CXX_FLAG_SUPPURT_CXX17[] = "c++17";
        constexpr char CXX_FLAG_ALL_WARNINGS[] = "all-warnings";
        //constexpr char CXX_COMPLIER_LATEST[] = "latest";

        static auto get_build_type( std::string_view value ) {
            if ( value == common::TARGET_TYPE_APP )
                return target_build_type::BINARY_APPLICATION;

            if ( value == common::TARGET_TYPE_STATIC_LIB )
                return target_build_type::STATIC_LIBRARY;

            if ( value == common::TARGET_TYPE_SHARED_LIB )
                return target_build_type::SHARED_LIBRARY;

            return target_build_type::BINARY_UNKNOWN;
        }

        static auto append_sub_target( bs::context &ctx, const YAML::Node &conf, std::string_view base_path ) -> bool;

        static auto append_sub_target( bs::context &ctx, const YAML::Node &conf, std::string_view base_path ) -> bool {

            using namespace std;

            if ( !ctx.build_path.empty( ) ) {
                const auto target_name = conf[CONF_TARGET_NAME].as<string>( );
                const auto target_path = base_path;
                const auto target_type = conf[CONF_TARGET_TYPE].as<string>( );

                vector<string> target_sources;
                if ( conf[CONF_TARGET_SOURES].IsScalar( ) ) {
                    const auto sources_value = conf[CONF_TARGET_SOURES].as<string>( );
                    if ( sources_value == "all" ) {
                        const auto sources_path =
                            string{target_path} + fs::path::preferred_separator + "src" + fs::path::preferred_separator + target_name;
                        const auto &extensions = ctx.cxx_extensions;
                        target_sources = fs_utils::get_directory_files_by_ext( sources_path, extensions );
                    } else if ( conf[CONF_TARGET_SOURES].IsSequence( ) ) {
                        target_sources = conf[CONF_TARGET_SOURES].as<vector<string>>( );
                    }
                }

                bs::target current_target;
                current_target.name = target_name;
                current_target.sources = target_sources;
                current_target.type = get_build_type( target_type );

                if ( conf[CONF_TARGET_LINK_LIBRARIES].IsDefined( ) ) {
                    current_target.link_libraries = conf[CONF_TARGET_LINK_LIBRARIES].as<std::vector<string>>( );
                }

                if ( current_target.type == target_build_type::BINARY_APPLICATION )
                    current_target.output = current_target.name;

                ctx.build_targets.push_back( current_target );
            }

            return false;
        }

        static auto flags_to_compiler_options( bs::context &ctx, const std::vector<std::string> &flags ) {
            std::vector<std::string> options;

            for ( const auto &f : flags ) {
                if ( f == CXX_FLAG_SUPPURT_CXX17 )
                    options.push_back( ctx.flags.support_cxx_17 );

                if ( f == CXX_FLAG_ALL_WARNINGS )
                    options.push_back( ctx.flags.verbosity_warnings_all );
            }

            return options;
        }

        auto parse_conf( bs::context &ctx, std::string_view conf_path ) -> bool {
            using namespace std;

            error_code err;
            if ( !fs::exists( conf_path, err ) ) {
                return false;
            }

            auto start_time = chrono::steady_clock::now( );

            auto conf = YAML::LoadFile( conf_path.data( ) );

            const auto project_name = conf[CONF_PROJECT_NAME].as<string>( );

            ctx.cxx_compiller =
                conf[CONF_CXX_COMPILLER].IsDefined( ) ? conf[CONF_CXX_COMPILLER].as<string>( ) : common::DEFAULT_CXX_COMPILER;

            if ( conf[CONF_CXX_FLAGS].IsSequence( ) ) {
                const auto global_cxx_flags = conf[CONF_CXX_FLAGS].as<vector<string>>( );
                const auto global_cxx_options = flags_to_compiler_options( ctx, global_cxx_flags );

                algorithm_utils::join_move( ctx.cxx_compile_options, global_cxx_options );
            }

            auto root_targets = conf[CONF_TARGET_LIST];

            auto base_path = fs::current_path( ).string( );

            for ( const auto &target : root_targets ) {
                append_sub_target( ctx, target, base_path );
            }

            auto end_time = chrono::steady_clock::now( );

            const chrono::duration<double> build_time = end_time - start_time;

            LOG_MESSAGE( APP_TAG, "[%2s] Parse '%1' done", conf_path, build_time.count( ) );

            return true;
        }

    } // namespace bs

} // namespace app
