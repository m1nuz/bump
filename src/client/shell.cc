#include <array>
#include <memory>

#include <config.h>
#include <journal.h>

#include "shell.h"

namespace shell {

    auto execute( std::string_view cmd ) -> std::string {
        using namespace std;

        LOG_DEBUG( APP_TAG, "Execute: %1", cmd );

        array<char, 512> buffer;
        string result;
        shared_ptr<FILE> pipe( popen( cmd.data( ), "r" ), pclose );
        if ( !pipe )
            return result;

        while ( !feof( pipe.get( ) ) ) {
            if ( fgets( data( buffer ), size( buffer ), pipe.get( ) ) != nullptr )
                result += data( buffer );
        }

        return result;
    }

    auto run( std::string_view cmd ) -> int {
        return system( cmd.data() );
    }

} // namespace shell
