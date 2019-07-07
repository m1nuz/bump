#include <filesystem>
#include <fstream>
#include <string_view>

#include <xargs.hpp>

#include <yaml-cpp/yaml.h>

#include "commands.h"
#include "common.h"
#include "context.h"

namespace app {

    inline auto apply_general_options( const std::vector<std::string> &options ) {
        for ( const auto &opt : options ) {
            if ( opt == OPT_VERBOSE ) {
                log_level = LOG_LEVEL_DEBUG;
            }
        }
    }

    static auto dispatch_command( bs::context &ctx, std::string_view command, std::string_view arguments,
                                  const std::vector<std::string> &options ) {

        apply_general_options( options );

        if ( command == CMD_INIT ) {
            if ( commands::default_init( ctx, arguments ) )
                return EXIT_SUCCESS;

            commands::failed_command_message( command, "Couldn't init" );
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
            if ( commands::help( arguments ) )
                return EXIT_SUCCESS;

            return EXIT_FAILURE;
        }

        if ( command == CMD_SEARCH ) {
            commands::search_package( arguments );

            return EXIT_SUCCESS;
        }

        if ( command == CMD_CLEAN ) {
            if ( commands::clean( ctx, arguments ) )
                return EXIT_SUCCESS;

            commands::failed_command_message( command, "Couldn't clean" );
        }

        if ( command == CMD_CLEAN_ALL ) {
            if ( commands::clean_all( ctx ) )
                return EXIT_SUCCESS;

            commands::failed_command_message( command, "Couldn't clean all" );
        }

        if ( command == CMD_RUN_TARGET ) {
            if ( commands::run_target( ctx, arguments ) )
                return EXIT_SUCCESS;

            commands::failed_command_message( command, "Couldn't run target" );
        }

        commands::invalid_command_message( command );

        return EXIT_FAILURE;
    }

    static auto init( bs::context &ctx ) {
        namespace fs = std::filesystem;

        const auto default_build_path = fs::current_path( ) / common::DEFAULT_BUILD_DIR;

        if ( ctx.build_path.empty( ) ) {
            ctx.build_path = default_build_path;
        }
    }

} // namespace app

volatile int log_level = LOG_LEVEL_MESSAGE;

extern int main( int argc, char *argv[] ) {

    using namespace std;

    string command;
    string arguments;
    vector<string> options;
    app::bs::context context;

    xargs::args args;
    args.add_arg( "<command>", "Command",
                  [&]( const auto &v ) {
                      const auto res = app::commands::is_command( v );
                      if ( res ) {
                          command = v;
                      }

                      return res;
                  } )
        .add_arg( "<args>", "Arguments",
                  [&]( const auto &v ) {
                      arguments = v;
                      return true;
                  } )
        .add_option( OPT_HELP, OPT_HELP_DESCRIPTION,
                     [&]( ) {
                         puts( args.usage( argv[0] ).c_str( ) );
                         exit( EXIT_SUCCESS );
                     } )
        .add_option( OPT_VERSION, OPT_VERSION_DESCRIPTION,
                     [&]( ) {
                         fprintf( stdout, "%s %s\n", APP_NAME, APP_VERSION );
                         exit( EXIT_SUCCESS );
                     } )
        .add_option( OPT_BINARY, OPT_BINARY_DESCRIPTION, [&]( ) { options.push_back( OPT_BINARY ); } )
        .add_option( OPT_VERBOSE, OPT_VERBOSE_DESCRIPTION, [&]( ) { options.push_back( OPT_VERBOSE ); } );

    args.dispath( argc, argv );

    if ( static_cast<size_t>( argc ) < 2 ) {
        puts( args.usage( argv[0] ).c_str( ) );
        return EXIT_SUCCESS;
    }

    LOG_DEBUG( APP_TAG, "Running command: %1 %2 %3", command, arguments, options );

    app::init( context );

    return app::dispatch_command( context, command, arguments, options );
}
