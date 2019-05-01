#include <experimental/filesystem>
#include <fstream>
#include <string_view>

#include <config.h>
#include <journal.h>
#include <yaml-cpp/yaml.h>

#include "common.h"
#include "context.h"

namespace fs = std::experimental::filesystem;

namespace app {

    namespace commands {

        auto create_empty_bump_file( const std::string_view path, const std::string_view project_name ) {

            if ( path.empty( ) ) {
                LOG_ERROR( APP_TAG, "%1", "Path is empty" );
                return false;
            }

            if ( project_name.empty() ) {
                LOG_ERROR( APP_TAG, "%1", "Project name is empty" );
                return false;
            }

            std::string empty_string;

            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "project" << YAML::Value << project_name.data( );
            out << YAML::Key << "build";
            out << YAML::Value << YAML::BeginSeq << YAML::EndSeq;
            out << YAML::EndMap;

            std::ofstream ofs{path.data( )};
            ofs.write( out.c_str( ), static_cast<ptrdiff_t>(out.size( )) );
            ofs.close( );

            return fs::exists( path );
        }

        auto default_init( context& ctx, std::string_view project_name ) {
            (void)ctx;

            auto curr_path = fs::current_path( );

            const auto src_path = curr_path / project_name / common::SOURCE_DIR;
            const auto include_path = curr_path / project_name / common::INCLUDE_DIR;
            const auto external_path = curr_path / project_name / common::EXTERNAL_DIR;
            const auto packages_path = curr_path / project_name / common::PACKAGE_DIR;
            const auto project_info_path = curr_path / project_name / DEFAULT_BUMP_FILE;

            bool res = true;
            res &= fs::create_directory( curr_path / project_name );
            res &= fs::create_directory( src_path );
            res &= fs::create_directory( include_path );
            res &= fs::create_directory( external_path );
            res &= fs::create_directory( packages_path );
            res &= create_empty_bump_file( project_info_path.c_str( ), project_name );

            return res;
        }

    } // namespace commands

} // namespace app
