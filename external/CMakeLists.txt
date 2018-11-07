include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
    libevent
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/libevent
    GIT_REPOSITORY https://github.com/libevent/libevent.git
    GIT_TAG release-2.1.8-stable
    INSTALL_DIR ""
    INSTALL_COMMAND ""
    )

set(LIBEVENT2_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/libevent/src/libevent/include CACHE INTERNAL "")

ExternalProject_Add(
    yaml-cpp
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG yaml-cpp-0.6.2
    INSTALL_DIR ""
    INSTALL_COMMAND ""
    )

set(YAMLPP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/yaml-cpp/src/yaml-cpp/include CACHE INTERNAL "")


