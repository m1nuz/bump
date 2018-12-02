#include <experimental/filesystem>
#include <string_view>

#include <config.h>
#include <journal.h>

#include "common.h"

namespace fs = std::experimental::filesystem;

namespace app {

    namespace commands {

        auto create_target_binary( const std::string_view target_name ) {
        }

        auto create_target_library( const std::string_view target_name ) {
        }

        auto create_target( const std::string_view target_name ) {
            auto curr_path = fs::current_path( );

            if ( fs::exists( curr_path / DEFAULT_BUMP_FILE ) ) {
                const auto src_path = curr_path / common::SOURCE_DIR;

                bool res = true;
                res &= fs::create_directory( src_path / target_name );

                return res;
            }

            LOG_ERROR( APP_TAG, "Can't create target. No '%1' in current directory", DEFAULT_BUMP_FILE );

            return false;
        }

    } // namespace commands

} // namespace app
