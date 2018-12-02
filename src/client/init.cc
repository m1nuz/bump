#include <experimental/filesystem>
#include <fstream>
#include <string_view>

#include <config.h>
#include <yaml-cpp/yaml.h>

#include "common.h"

namespace fs = std::experimental::filesystem;

namespace app {

    namespace commands {

        auto create_bump_file( const std::string_view path, const std::string_view root_target ) {
            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "build";
            out << YAML::Value << root_target.data( );
            out << YAML::Key << "sources";
            out << YAML::Value << YAML::BeginSeq << "all" << YAML::EndSeq;
            out << YAML::EndMap;

            std::ofstream ofs{path.data( )};
            ofs.write( out.c_str( ), out.size( ) );
            ofs.close( );

            return fs::exists( path );
        }

        auto default_init( std::string_view project_name ) {
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
            res &= create_bump_file( project_info_path.c_str( ), project_name );

            return res;
        }

    } // namespace commands

} // namespace app
