cmake_minimum_required(VERSION 3.0)
project(SDL_gesture_tests C)

option(SDLGESTURE_TESTS_STATIC "Link to static SDLx library" OFF)

if(SDLGESTURE_TESTS_STATIC)
    set(SDL2_COMPONENTS "SDL2-static")
    set(SDL2_TARGET "SDL2::SDL2-static")

    set(SDL3_COMPONENTS "SDL3-static")
    set(SDL3_TARGET "SDL3::SDL3-static")
else()
    set(SDL2_COMPONENTS "SDL2")
    set(SDL2_TARGET "SDL2::SDL2")

    set(SDL3_COMPONENTS "SDL3")
    set(SDL3_TARGET "SDL3::SDL3")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")

find_package(SDL2 CONFIG COMPONENTS ${SDL2_COMPONENTS} SDL2main)
if(NOT SDL2_FOUND OR NOT TARGET SDL2::SDL2)
    find_package(PrivateSDL2 MODULE REQUIRED)
    set(SDL2_TARGET "PrivateSDL2::PrivateSDL2")
endif()
if(NOT TARGET SDL2::SDL2main)
    find_package(SDL2main MODULE QUIET)
    if(NOT SDL2main_FOUND)
        add_library(SDL2::SDL2main INTERFACE)
    endif()
endif()

find_package(SDL3 QUIET CONFIG COMPONENTS ${SDL3_COMPONENTS})
if(NOT SDL3_FOUND)
    message(STATUS "SDL3 not found -> not testing SDL_gesture with SDL3")
endif()

if(NOT TARGET SDL_gesture::SDL_gesture)
    find_package(SDL_gesture REQUIRED)
endif()

add_executable(testgesture_sdl2 testgesture.c)
target_link_libraries(testgesture_sdl2 PRIVATE SDL_gesture::SDL_gesture SDL2::SDL2main  ${SDL2_TARGET})

if(TARGET ${SDL3_TARGET})
    add_executable(testgesture_sdl3 testgesture.c)
    target_compile_definitions(testgesture_sdl3 PRIVATE TESTGESTURE_SDL3)
    target_link_libraries(testgesture_sdl3 PRIVATE SDL_gesture::SDL_gesture ${SDL3_TARGET})
endif()
