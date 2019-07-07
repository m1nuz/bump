#pragma once

#include <config.h>
#include <journal.h>


namespace app {

    namespace bs {

        struct context;

    } // namespace bs

    namespace commands {

        auto invalid_command_message( const std::string_view command ) {
            LOG_ERROR( APP_TAG, "Invalid command %1", command );
            exit( EXIT_FAILURE );
        }

        auto failed_command_message( const std::string_view command, const std::string_view reason ) {
            LOG_ERROR( APP_TAG, "Command %1 failed : '%2'", command, reason );
            exit( EXIT_FAILURE );
        }

        auto is_command( const std::string_view cmd ) -> bool;

        auto default_init( bs::context &ctx, std::string_view project_name ) -> bool;
        auto create_target( const std::string_view target_name ) -> bool;
        auto search_package( const std::string_view package_name ) -> bool;
        auto build_all( bs::context &ctx, std::string_view arguments ) -> bool;
        auto run_target( bs::context &ctx, const std::string_view target_name ) -> bool;
        auto clean( bs::context &ctx, std::string_view arguments ) -> bool;
        auto clean_all( bs::context &ctx ) -> bool;
        auto help( std::string_view args ) -> bool;

    } // namespace commands

} // namespace app
