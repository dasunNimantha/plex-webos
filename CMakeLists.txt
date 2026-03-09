cmake_minimum_required(VERSION 3.16)
project(plex-webos LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Desktop vs webOS build
option(WEBOS_BUILD "Cross-compile for webOS ARM" OFF)

find_package(PkgConfig REQUIRED)

if(WEBOS_BUILD)
    add_definitions(-DWEBOS_PLATFORM)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
    pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
    set(OPENGL_LIBRARIES GLESv2 EGL)
    set(STARFISH_LIBRARIES starfish-media-pipeline)
else()
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
    pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
    find_package(OpenGL REQUIRED)
    set(OPENGL_LIBRARIES OpenGL::GL)
    set(STARFISH_LIBRARIES "")
endif()

find_package(CURL REQUIRED)

add_executable(plex-webos
    src/main.cpp
    src/app.cpp
    src/plex/client.cpp
    src/plex/auth.cpp
    src/ui/renderer.cpp
    src/ui/text.cpp
    src/ui/image_cache.cpp
    src/ui/login_screen.cpp
    src/ui/home_screen.cpp
    src/ui/grid_screen.cpp
    src/ui/detail_screen.cpp
    src/ui/playing_screen.cpp
    src/input/remote.cpp
    src/media/player.cpp
)

target_include_directories(plex-webos PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

target_link_libraries(plex-webos PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${CURL_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${STARFISH_LIBRARIES}
    pthread
)

target_compile_options(plex-webos PRIVATE -Wall -Wextra -O2)

# Install target for packaging
install(TARGETS plex-webos DESTINATION .)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/webos/ DESTINATION .)
