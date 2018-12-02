#include <experimental/filesystem>
#include <algorithm>
#include <string_view>

#include <yaml-cpp/yaml.h>
#include <config.h>
#include <journal.h>

namespace fs = std::experimental::filesystem;

namespace app {

    namespace commands {

        std::string cxx_compiller = "/usr/bin/gcc";

        auto sys_exec( std::string_view cmd ) {
            using namespace std;

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

        auto get_directory_files( const std::string &dir, const std::vector<std::string> &extensions ) {
            using namespace std;

            vector<string> files;
            for ( auto &p : fs::recursive_directory_iterator( dir ) ) {
                if ( fs::is_regular_file( p ) ) {
                    if ( extensions.empty( ) ||
                         find( extensions.begin( ), extensions.end( ), p.path( ).extension( ).string( ) ) != extensions.end( ) ) {
                        files.push_back( p.path( ).string( ) );
                    }
                }
            }

            return files;
        }

        auto build_target( YAML::Node &conf, const std::string_view target, const std::string_view target_path ) -> bool {
            using namespace std;

            if ( !target.empty( ) ) {
                const auto target_sources = conf["sources"].as<vector<string>>( );
                if ( target_sources.empty( ) ) {
                    LOG_ERROR( APP_TAG, "Nothing to build for target %1", target.data( ) );
                    return false;
                }

                const auto extensions = std::vector<std::string>{".cpp", ".cxx", ".cc"};
                const auto files = get_directory_files( string{target_path} + fs::path::preferred_separator + "src", extensions );

                for ( const auto &f : files ) {
                    const auto command = cxx_compiller + " -c " + f;

                    const auto res = sys_exec( command );
                    if ( res.empty( ) ) {
                    }
                }

                return true;
            } else {
            }

            return false;
        }


        auto build_all( std::string_view arguments ) {

            const auto target_path = fs::current_path( ).string( );
            const auto conf_path = target_path + fs::path::preferred_separator + DEFAULT_BUMP_FILE;

            if ( fs::exists( conf_path ) ) {
                auto conf = YAML::LoadFile( conf_path );

                fs::create_directory( target_path + fs::path::preferred_separator + "build" );

                const auto root_target = conf["build"].as<std::string>( );

                build_target( conf, root_target, target_path );
            }

            return false;
        }

    } // namespace commands

} // namespace app
