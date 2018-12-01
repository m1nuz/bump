#include <experimental/filesystem>
#include <string_view>
#include <fstream>

#include <config.h>
#include <journal.h>
#include <xargs.hpp>

#include <yaml-cpp/yaml.h>

volatile int log_level = DEFAULT_LOG_LEVEL;

namespace app {

    constexpr char APP_TAG[] = "bump";
    constexpr char DEFAULT_BUMP_FILE[] = ".bump.yml";

    constexpr char OPT_BINARY[] = "--bin";

    namespace fs = std::experimental::filesystem;

    auto create_bump_file( const std::string_view path, const std::string_view root_target ) {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "build";
        out << YAML::Value << root_target.data();
        out << YAML::Key << "sources";
        out << YAML::Value << YAML::BeginSeq << "all" << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream ofs{path.data()};
        ofs.write(out.c_str(), out.size());
        ofs.close();

        return fs::exists( path );
    }

    auto default_init( std::string_view project_name ) {
        auto curr_path = fs::current_path( );

        const auto src_path = curr_path / project_name / "/src";
        const auto include_path = curr_path / project_name / "/include";
        const auto external_path = curr_path / project_name / "/external";
        const auto packages_path = curr_path / project_name / "/.packages";
        const auto project_info_path = curr_path / project_name / DEFAULT_BUMP_FILE;

        bool res = true;
        res &= fs::create_directory( curr_path / project_name );
        res &= fs::create_directory( src_path );
        res &= fs::create_directory( include_path );
        res &= fs::create_directory( external_path );
        res &= fs::create_directory( packages_path );
        res &= create_bump_file( project_info_path.c_str(), project_name );

        return res;
    }

    auto create_target_binary( const std::string_view target_name ) {

    }

    auto create_target_library( const std::string_view target_name ) {

    }

    auto build_target( YAML::Node &conf, const std::string_view target_name ) {
        if ( !target_name.empty() ) {
            const auto target_sources = conf["sources"].as<std::vector<std::string>>();
        } else {

        }
    }

    auto invalid_command( const std::string_view command ) {
        LOG_ERROR( APP_TAG, "Invalid command %1", command );
        exit(EXIT_FAILURE);
    }

    auto failed_command( const std::string_view command, const std::string_view reason ) {
        LOG_ERROR( APP_TAG, "Command %1 failed : '%2'", command, reason );
        exit(EXIT_FAILURE);
    }

    auto dispatch_command( std::string_view command, std::string_view arguments, std::vector<std::string> options ) {
        if ( command == "init" ) {
            if ( !arguments.empty( ) ) {
                if ( default_init( arguments ) )
                    return EXIT_SUCCESS;

                failed_command( command, "Can't create directories and files" );
            }
        }

        if ( command == "build" ) {
            const auto conf_path = fs::current_path( ).string() + '/' + DEFAULT_BUMP_FILE;

            if ( fs::exists(conf_path) ) {
                auto conf = YAML::LoadFile( conf_path );

                const auto root_target = conf["build"].as<std::string>();

                build_target( conf, root_target );
            }

            return EXIT_SUCCESS;
        }

        if ( command == "new" ) {


            return EXIT_SUCCESS;
        }

        if ( command == "show" ) {
            return EXIT_SUCCESS;
        }

        if ( command == "install" ) {
            return EXIT_SUCCESS;
        }

        if ( command == "search" ) {
            return EXIT_SUCCESS;
        }

        invalid_command( command );

        return EXIT_FAILURE;
    }

} // namespace app

extern int main( int argc, char *argv[] ) {

    using namespace std;

    string command;
    string arguments;
    vector<string> options;

    xargs::args args;
    args.add_arg( "<command>", "Command", [&]( const auto &v ) {
        command = v;
    } ).add_arg( "<args>", "Arguments", [&]( const auto &v ) {
        arguments = v;
    } ).add_option( "-h", "Display help", [&]( ) {
        puts( args.usage( argv[0] ).c_str( ) );
        exit( EXIT_SUCCESS );
    } ).add_option( "-v", "Version", [&]( ) {
        fprintf(stdout, "%s %s\n", APP_NAME, APP_VERSION );
        exit( EXIT_SUCCESS );
    } ).add_option( app::OPT_BINARY, "Binary", [&]( ) {
        options.push_back( app::OPT_BINARY );
    });

    args.dispath( argc, argv );

    if ( static_cast<size_t>( argc ) < 2 ) {
        puts( args.usage( argv[0] ).c_str( ) );
        return EXIT_SUCCESS;
    }

    LOG_DEBUG( app::APP_TAG, "Running command: %1 %2 %3", command, arguments, options );

    return app::dispatch_command( command, arguments, options );
}
