#include <iostream>
#include <string>
#include <vector>

#include <config.h>

#include "common.h"

namespace app {

    namespace commands {

        static std::vector<std::string> commands_list = {
            {CMD_INIT},
            {CMD_BUILD},
            {CMD_NEW},
            {CMD_ADD},
            {CMD_SHOW},
            {CMD_INSTALL},
            {CMD_SEARCH},
            {CMD_CLEAN},
            {CMD_CLEAN_ALL},
            {CMD_RUN_TARGET}
        };

        auto is_command( const std::string_view cmd ) -> bool {
            for (const auto& c : commands_list) {
                if (cmd == c)
                    return true;
            }

            return false;
        }


        auto help( std::string_view args ) {
            using namespace std;

            if ( args.empty( ) ) {
                for ( const auto& c : commands_list ) {
                    cout << '\t' << c << '\n';
                }

                return true;
            }

            return true;
        }

    } // namespace commands

} // namespace app
