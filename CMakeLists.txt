cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(bump VERSION 0.1.0)

set(INSTALL_DIR /usr/bin)
set(SHARED_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(external)

set(APP_NAME bump)
set(SOURCES src/main.cc)

configure_file("${SHARED_INCLUDE_DIR}/config.h.in" "${SHARED_INCLUDE_DIR}/config.h")

add_executable(${APP_NAME} ${SOURCES} ${HEADERS})

set_target_properties(${APP_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(${APP_NAME} PUBLIC
    ${SHARED_INCLUDE_DIR}
    ${APP_INCLUDES}
    external
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
    stdc++fs
)

install(TARGETS ${APP_NAME} RUNTIME DESTINATION ${INSTALL_DIR})
