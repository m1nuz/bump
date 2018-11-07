#include <string_view>
#include <filesystem>

#include <config.h>
#include <journal.h>
#include <xargs.hpp>

volatile int log_level;

namespace fs = std::filesystem;

auto dispatch_command( std::string_view command ) {

    if ( command == "init" ) {
    }

    if ( command == "build" ) {
    }

    if ( command == "install" ) {
    }

    if ( command == "search" ) {
    }

    return 0;
}

extern int main( int argc, char *argv[] ) {

    using namespace std;

    string command;
    string arguments;

    xargs::args args;
    args.add_arg( "<command>", "Command", [&]( const auto &v ) { command = v; } )
        .add_arg( "<args>", "Arguments", [&]( const auto &v ) { arguments = v; } )
        .add_option( "-h", "Display help",
                     [&]( ) {
                         puts( args.usage( argv[0] ).c_str( ) );
                         exit( EXIT_SUCCESS );
                     } )
        .add_option( "-v", "Version", [&]( ) {
            fprintf( stdout, "%s %s\n", APP_NAME, APP_VERSION );
            exit( EXIT_SUCCESS );
        } );

    args.dispath( argc, argv );

    if ( static_cast<size_t>( argc ) < args.count( ) ) {
        puts( args.usage( argv[0] ).c_str( ) );
        return EXIT_SUCCESS;
    }

    return dispatch_command( command );
}
