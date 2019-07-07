#include <filesystem>
#include <fstream>
#include <string_view>

#include <config.h>
#include <journal.h>
#include <yaml-cpp/yaml.h>

#include "common.h"
#include "context.h"

namespace fs = std::filesystem;

namespace app {

    namespace commands {

        auto create_empty_bump_file( const std::string_view path, const std::string_view project_name, std::error_code &ec ) {

            if ( path.empty( ) ) {
                LOG_ERROR( APP_TAG, "%1", "Path is empty" );
                ec = std::make_error_code( std::errc::invalid_argument );
                return false;
            }

            if ( project_name.empty( ) ) {
                LOG_ERROR( APP_TAG, "%1", "Project name is empty" );
                ec = std::make_error_code( std::errc::invalid_argument );
                return false;
            }

            if ( fs::exists( path ) ) {
                LOG_WARNING( APP_TAG, "%1 already exits", path );
                ec = std::make_error_code( std::errc::file_exists );
                return false;
            }

            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "project" << YAML::Value << project_name.data( );
            out << YAML::Key << "build";
            out << YAML::Value << YAML::BeginSeq << YAML::EndSeq;
            out << YAML::EndMap;

            std::ofstream ofs{path.data( )};
            ofs.write( out.c_str( ), static_cast<ptrdiff_t>( out.size( ) ) );
            ofs.close( );

            ec.clear( );

            return fs::exists( path );
        }

        auto default_init( bs::context &ctx, std::string_view project_name ) {
            (void)ctx;

            auto curr_path = fs::current_path( );

            LOG_DEBUG( APP_TAG, "Current work directory '%1'", curr_path.string( ) );

            auto project_path = project_name.empty( ) ? curr_path : ( curr_path / project_name );

            const auto src_path = project_path / common::DEFAULT_SOURCE_DIR;
            const auto include_path = project_path / common::DEFAULT_INCLUDE_DIR;
            const auto external_path = project_path / common::DEFAULT_EXTERNAL_DIR;
            const auto packages_path = project_path / common::DEFAULT_PACKAGE_DIR;
            const auto project_info_path = project_path / DEFAULT_BUMP_FILE;

            bool res = true;

            if ( !project_name.empty( ) )
                res &= fs::create_directory( project_path );

            std::string curr_project_name = project_name.empty( ) ? project_path.filename( ).string( ) : std::string{project_name};

            std::error_code err;
            if ( fs::create_directory( src_path, err ) ) {
                LOG_DEBUG( APP_TAG, "Create directory '%1'", src_path.string( ) );
            } else if ( err ) {
                LOG_ERROR( APP_TAG, "Failed create directory: %1", err.message( ) );
            } else {
                LOG_DEBUG( APP_TAG, "'%1' already exists", src_path.string( ) );
            }
            res &= !err;

            if ( fs::create_directory( include_path, err ) ) {
                LOG_DEBUG( APP_TAG, "Create directory '%1'", include_path.string( ) );
            } else if ( err ) {
                LOG_ERROR( APP_TAG, "Failed create directory: %1", err.message( ) );
            } else {
                LOG_DEBUG( APP_TAG, "'%1' already exists", include_path.string( ) );
            }
            res &= !err;

            if ( fs::create_directory( external_path, err ) ) {
                LOG_DEBUG( APP_TAG, "Create directory '%1'", external_path.string( ) );
            } else if ( err ) {
                LOG_ERROR( APP_TAG, "Failed create directory: %1", err.message( ) );
            } else {
                LOG_DEBUG( APP_TAG, "'%1' already exists", external_path.string( ) );
            }
            res &= !err;

            if ( fs::create_directory( packages_path, err ) ) {
                LOG_DEBUG( APP_TAG, "Create directory '%1'", packages_path.string( ) );
            } else if ( err ) {
                LOG_ERROR( APP_TAG, "Failed create directory: %1", err.message( ) );
            } else {
                LOG_DEBUG( APP_TAG, "'%1' already exists", packages_path.string( ) );
            }
            res &= !err;

            if ( !create_empty_bump_file( project_info_path.c_str( ), curr_project_name, err ) ) {
                if ( err != std::errc::file_exists ) {
                    LOG_ERROR( APP_TAG, "Failed create bump file: %1", err.message( ) );
                } else {
                    err.clear( );
                }
            } else {
                LOG_DEBUG( APP_TAG, "Create bump file %1 %2", project_info_path, res );
            }
            res &= !err;

            return res;
        }

    } // namespace commands

} // namespace app
