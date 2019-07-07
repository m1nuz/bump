#include <string_view>
#include <filesystem>

#include <fs_utils.h>
#include <journal.h>

#include "config.h"
#include "context.h"

namespace fs = std::filesystem;
namespace fs_utils = filesystem_utils;

namespace app {

    namespace commands {

        auto clean_packages( bs::context &ctx ) {
        }

        auto clean_target( bs::context &ctx ) {
        }

        auto clean_build( bs::context &ctx ) {
            auto dir_list = fs_utils::get_directory_list( ctx.build_path );

            for ( const auto &f : dir_list ) {
                LOG_DEBUG( APP_TAG, "%1", f );
            }

            for ( const auto &f : dir_list ) {
                std::error_code ec;
                fs::remove_all( f, ec );
            }

            return true;
        }

        auto clean_all( bs::context &ctx ) -> bool {
            return false;
        }

        auto clean( bs::context &ctx, std::string_view arguments ) -> bool {
            if ( arguments.empty( ) ) {
                return clean_build( ctx );
            }

            return false;
        }

    } // namespace commands

} // namespace app
