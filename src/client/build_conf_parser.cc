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
        constexpr char CONF_TARGET_RPATH[] = "rpath";
        constexpr char CONF_TARGET_LINK_LIBRARIES[] = "libaries";
        constexpr char CONF_ALL_TAG[] = "all";

        constexpr char CXX_FLAG_SUPPURT_CXX17[] = "c++17";
        constexpr char CXX_FLAG_ALL_WARNINGS[] = "all-warnings";
        // constexpr char CXX_COMPLIER_LATEST[] = "latest";

        static auto get_build_type( std::string_view value ) {
            if ( value == common::TARGET_TYPE_APP )
                return target_build_type::BINARY_APPLICATION;

            if ( value == common::TARGET_TYPE_STATIC_LIB )
                return target_build_type::STATIC_LIBRARY;

            if ( value == common::TARGET_TYPE_SHARED_LIB )
                return target_build_type::SHARED_LIBRARY;

            return target_build_type::BINARY_UNKNOWN;
        }

        static auto make_application_name( std::string_view name ) {
            return name;
        }

        static auto make_static_library_name( std::string_view name ) {
            std::string lib_name;
            if ( name.compare( 1, 3, "lib" ) == 0 )
                lib_name += "lib";
            lib_name += name;
            lib_name += ".a";

            return lib_name;
        }

        static auto make_shared_library_name( std::string_view name ) {
            std::string lib_name;
            if ( name.compare( 1, 3, "lib" ) == 0 )
                lib_name += "lib";
            lib_name += name;
            lib_name += ".so";

            return lib_name;
        }

        static auto append_sub_target( bs::context &ctx, const YAML::Node &conf, bs::target &current_target ) -> bool;

        static auto append_sub_target( bs::context &ctx, const YAML::Node &conf, bs::target &current_target ) -> bool {
            using namespace std;

            if ( !ctx.build_path.empty( ) ) {
                const auto target_name = conf[CONF_TARGET_NAME].as<string>( );
                const auto target_path = ctx.base_path;
                const auto target_type = conf[CONF_TARGET_TYPE].as<string>( );
                const auto target_runtime_path = conf[CONF_TARGET_RPATH] ? conf[CONF_TARGET_RPATH].as<string>( ) : string{};

                vector<string> target_sources;
                if ( conf[CONF_TARGET_SOURES].IsScalar( ) ) {
                    const auto sources_value = conf[CONF_TARGET_SOURES].as<string>( );
                    if ( sources_value == CONF_ALL_TAG ) {

                        const auto sources_path = string{target_path} + fs::path::preferred_separator + common::DEFAULT_SOURCE_DIR +
                                                  fs::path::preferred_separator + target_name;
                        std::error_code err;
                        if ( !fs::exists( sources_path, err ) ) {
                            LOG_WARNING( APP_TAG, "Source path not exists %1", sources_path );
                        }

                        const auto &extensions = ctx.cxx_extensions;
                        target_sources = fs_utils::get_directory_files_by_ext( sources_path, extensions );
                    }
                } else if ( conf[CONF_TARGET_SOURES].IsSequence( ) ) {
                    target_sources = conf[CONF_TARGET_SOURES].as<vector<string>>( );

                    std::error_code err;
                    for ( auto &s : target_sources ) {
                        if ( !s.empty( ) && !fs::exists( s, err ) ) {
                            const auto sources_path = string{target_path} + fs::path::preferred_separator + common::DEFAULT_SOURCE_DIR +
                                                      fs::path::preferred_separator + target_name + fs::path::preferred_separator + s;
                            if ( fs::exists( sources_path, err ) )
                                s = sources_path;
                        }
                    }
                }

                current_target.name = target_name;
                current_target.sources = target_sources;
                current_target.type = get_build_type( target_type );
                current_target.run_time_path = target_runtime_path;

                if ( conf[CONF_TARGET_LINK_LIBRARIES] ) {
                    current_target.link_libraries = conf[CONF_TARGET_LINK_LIBRARIES].as<std::vector<string>>( );
                }

                switch ( current_target.type ) {
                case target_build_type::BINARY_UNKNOWN:
                    current_target.output = current_target.name;
                    break;
                case target_build_type::BINARY_APPLICATION:
                    current_target.output = make_application_name( current_target.name );
                    break;
                case target_build_type::STATIC_LIBRARY:
                    current_target.output = make_static_library_name( current_target.name );
                    break;
                case target_build_type::SHARED_LIBRARY:
                    current_target.output = make_shared_library_name( current_target.name );
                    break;
                }

                auto sub_targets = conf[CONF_TARGET_LIST];
                for ( const auto &target_conf : sub_targets ) {
                    bs::target new_target;
                    if ( append_sub_target( ctx, target_conf, new_target ) )
                        current_target.sub_targets.push_back( new_target );
                }

                return true;
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

            auto start_time = bs::clock_type::now( );

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
            for ( const auto &target_conf : root_targets ) {
                bs::target current_target;
                if ( append_sub_target( ctx, target_conf, current_target ) ) {
                    ctx.build_targets.push_back( current_target );
                }
            }

            auto end_time = bs::clock_type::now( );

            const chrono::duration<double> parse_time = end_time - start_time;

            LOG_MESSAGE( APP_TAG, "[%2s] Parse '%1' done", conf_path, parse_time.count( ) );

            return true;
        }

    } // namespace bs

} // namespace app
