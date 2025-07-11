cmake_minimum_required(VERSION 3.0...4.0)
project(SDL_gesture_tests C)

option(SDLGESTURE_TESTS_STATIC "Link to static SDLx library" OFF)

if(SDLGESTURE_TESTS_STATIC)
    set(SDL3_COMPONENTS "SDL3-static")
    set(SDL3_TARGET "SDL3::SDL3-static")
else()
    set(SDL3_COMPONENTS "SDL3")
    set(SDL3_TARGET "SDL3::SDL3")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")

find_package(SDL3 QUIET CONFIG COMPONENTS ${SDL3_COMPONENTS})
if(NOT SDL3_FOUND)
    message(STATUS "SDL3 not found -> not testing SDL_gesture with SDL3")
endif()

if(NOT TARGET SDL_gesture::SDL_gesture)
    find_package(SDL_gesture REQUIRED)
endif()

if(TARGET ${SDL3_TARGET})
    add_executable(testgesture testgesture.c)
    target_compile_definitions(testgesture PRIVATE TESTGESTURE_SDL3)
    target_link_libraries(testgesture PRIVATE SDL_gesture::SDL_gesture ${SDL3_TARGET})
endif()
