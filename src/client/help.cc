#include <iostream>
#include <string>
#include <vector>

#include <config.h>

#include "common.h"

namespace app {

    namespace commands {

        auto help( std::string_view args ) {
            using namespace std;

            vector<string> command_list = {
                {CMD_INIT},
                {CMD_BUILD},
                {CMD_NEW},
                {CMD_ADD},
                {CMD_SHOW},
                {CMD_INSTALL},
                {CMD_SEARCH}
            };

            if ( args.empty( ) ) {
                for ( const auto& c : command_list ) {
                    cout << '\t' << c << '\n';
                }

                return true;
            }

            return true;
        }

    } // namespace commands

} // namespace app
