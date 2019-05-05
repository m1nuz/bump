#pragma once

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace filesystem_utils {

    auto get_directory_files_by_ext( const std::string_view dir, const std::vector<std::string> &extensions ) -> std::vector<std::string>;
    auto get_directory_list( const std::string_view dir ) -> std::vector<std::string>;

    inline auto get_directory_files_by_ext( const std::string_view dir, const std::vector<std::string> &extensions ) -> std::vector<std::string> {
        using namespace std;
        namespace fs = std::filesystem;

        vector<string> files;
        for ( auto &p : fs::recursive_directory_iterator( dir ) ) {
            if ( fs::is_regular_file( p ) ) {
                if ( extensions.empty( ) ||
                     std::find( extensions.begin( ), extensions.end( ), p.path( ).extension( ).string( ) ) != extensions.end( ) ) {
                    files.push_back( p.path( ).string( ) );
                }
            }
        }

        return files;
    }

    inline auto get_directory_list( const std::string_view dir ) -> std::vector<std::string> {
        using namespace std;
        namespace fs = std::filesystem;

        vector<string> dir_list;
        for ( auto &p : fs::directory_iterator( dir ) ) {
            dir_list.push_back( p.path( ).string( ) );
        }

        return dir_list;
    }

} // namespace filesystem_utils
