set(APP_NAME bump)
set(SOURCES
    help.cc
    build.cc
    init.cc
    search.cc
    new.cc
    clean.cc
    run_target.cc
    shell.cc
    build_conf_parser.cc
    main.cc)

configure_file("${SHARED_INCLUDE_DIR}/config.h.in" "${SHARED_INCLUDE_DIR}/config.h")

#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)
#find_package(Boost 1.65 REQUIRED)

add_executable(${APP_NAME} ${SOURCES} ${HEADERS})
add_dependencies(${APP_NAME} yaml-cpp)

set_target_properties(${APP_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(${APP_NAME} PUBLIC
    ${SHARED_INCLUDE_DIR}
    ${APP_INCLUDES}
    ${YAMLPP_INCLUDE_DIRS}
    ${STLAB_INCLUDE_DIRS}
    ${EXTERNAL_INCLUDE_DIR}
#    ${Boost_INCLUDE_DIR}
)

target_compile_definitions(${APP_NAME} PUBLIC ${APP_DEFINES})

if (UNIX)
    target_compile_options(${APP_NAME} PUBLIC
#        $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>
        -std=c++17
        -pthread
        -pedantic
        -Wall
        -Wextra
        -Wshadow
#        -Werror
        )
elseif (MSVC)
    target_compile_options(${APP_NAME} PUBLIC
        /W3
#        /WX
        )
endif()

target_link_libraries(${APP_NAME} PUBLIC
    ${APP_LIBRARIES}
    ${YAMLPP_LIBRARIES}
#    ${Boost_LIBRARIES}
    stdc++fs
    -pthread
)

install(TARGETS ${APP_NAME} RUNTIME DESTINATION ${INSTALL_DIR})
