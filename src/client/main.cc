#include <fstream>
#include <string_view>
#include <filesystem>

#include <xargs.hpp>

#include <yaml-cpp/yaml.h>

#include "commands.h"
#include "context.h"
#include "common.h"

namespace app {

    auto dispatch_command( context &ctx, std::string_view command, std::string_view arguments, std::vector<std::string> options ) {
        if ( command == CMD_INIT ) {
            if ( !arguments.empty( ) ) {
                if ( commands::default_init( ctx, arguments ) )
                    return EXIT_SUCCESS;

                commands::failed_command_message( command, "Can't create directories and files" );
                return EXIT_FAILURE;
            }
        }

        if ( command == CMD_BUILD ) {
            if ( commands::build_all( ctx, arguments ) )
                return EXIT_SUCCESS;

            return EXIT_FAILURE;
        }

        if ( command == CMD_NEW ) {
            commands::create_target( arguments );

            return EXIT_SUCCESS;
        }

        if ( command == CMD_ADD ) {
            return EXIT_SUCCESS;
        }

        if ( command == CMD_SHOW ) {
            return EXIT_SUCCESS;
        }

        if ( command == CMD_INSTALL ) {
            return EXIT_SUCCESS;
        }

        if ( command == CMD_HELP ) {
            if ( !commands::help( arguments ) )
                return EXIT_FAILURE;

            return EXIT_SUCCESS;
        }

        if ( command == CMD_SEARCH ) {
            commands::search_package( arguments );

            return EXIT_SUCCESS;
        }

        if ( command == CMD_CLEAN ) {
            if ( commands::clean( ctx, arguments ) )
                return EXIT_SUCCESS;

            commands::failed_command_message( command, "Couldn't clean" );
            return EXIT_FAILURE;
        }

        commands::invalid_command_message( command );

        return EXIT_FAILURE;
    }

    auto init( context &ctx ) {
        namespace fs = std::filesystem;

        const auto default_build_path = fs::current_path( ) / common::DEFAULT_BUILD_DIR;

        if ( ctx.build_path.empty( ) ) {
            ctx.build_path = default_build_path;
        }
    }

} // namespace app

volatile int log_level = DEFAULT_LOG_LEVEL;

extern int main( int argc, char *argv[] ) {

    using namespace std;

    string command;
    string arguments;
    vector<string> options;
    app::context context;

    xargs::args args;
    args.add_arg( "<command>", "Command", [&]( const auto &v ) { command = v; } )
        .add_arg( "<args>", "Arguments", [&]( const auto &v ) { arguments = v; } )
        .add_option( "-h", "Display help",
                     [&]( ) {
                         puts( args.usage( argv[0] ).c_str( ) );
                         exit( EXIT_SUCCESS );
                     } )
        .add_option( "-v", "Version",
                     [&]( ) {
                         fprintf( stdout, "%s %s\n", APP_NAME, APP_VERSION );
                         exit( EXIT_SUCCESS );
                     } )
        .add_option( OPT_BINARY, "Binary", [&]( ) { options.push_back( OPT_BINARY ); } );

    args.dispath( argc, argv );

    if ( static_cast<size_t>( argc ) < 2 ) {
        puts( args.usage( argv[0] ).c_str( ) );
        return EXIT_SUCCESS;
    }

    LOG_DEBUG( APP_TAG, "Running command: %1 %2 %3", command, arguments, options );

    app::init( context );

    return app::dispatch_command( context, command, arguments, options );
}
