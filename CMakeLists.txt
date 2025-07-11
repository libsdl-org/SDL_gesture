cmake_minimum_required(VERSION 3.0...4.0)
project(SDL_gesture VERSION 1.0.0 LANGUAGES C)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

option(SDLGESTURE_TESTS "Build SDL_gesture tests" ON)

add_library(SDL_gesture INTERFACE)
add_library(SDL_gesture::SDL_gesture ALIAS SDL_gesture)
target_include_directories(SDL_gesture INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

if(SDLGESTURE_TESTS)
    add_subdirectory(test)
endif()

if(WINDOWS AND NOT MINGW)
    set(SDLGESTURE_INSTALL_CMAKEDIR_ROOT_DEFAULT "cmake")
else()
    set(SDLGESTURE_INSTALL_CMAKEDIR_ROOT_DEFAULT "${CMAKE_INSTALL_LIBDIR}/cmake")
endif()
set(SDLGESTURE_INSTALL_CMAKEDIR_ROOT "${SDLGESTURE_INSTALL_CMAKEDIR_ROOT_DEFAULT}" CACHE STRING "Root folder where to install SDLConfig.cmake related files (SDL_gesture subfolder for MSVC projects)")

if(WINDOWS AND NOT MINGW)
    set(SDLGESTURE_INSTALL_CMAKEDIR "${SDLGESTURE_INSTALL_CMAKEDIR_ROOT}")
    set(LICENSES_PREFIX "licenses/SDL_gesture")
else()
    set(SDLGESTURE_INSTALL_CMAKEDIR "${SDLGESTURE_INSTALL_CMAKEDIR_ROOT}/SDL_gesture")
    set(LICENSES_PREFIX "${CMAKE_INSTALL_DATAROOTDIR}/licenses/SDL_gesture")
endif()

if(CMAKE_SYSTEM_NAME MATCHES FreeBSD)
    set(SDLGESTURE_INSTALL_PKGCONFIGDIR "libdata/pkgconfig")
else()
    set(SDLGESTURE_INSTALL_PKGCONFIGDIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif()

configure_package_config_file(cmake/SDL_gestureConfig.cmake.in SDL_gestureConfig.cmake
    INSTALL_DESTINATION "${SDLGESTURE_INSTALL_CMAKEDIR}"
)
write_basic_package_version_file(SDL_gestureConfigVersion.cmake
    COMPATIBILITY AnyNewerVersion
)

install(TARGETS SDL_gesture EXPORT SDLGestureExport)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/SDL_gesture.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
install(EXPORT SDLGestureExport
    DESTINATION "${SDLGESTURE_INSTALL_CMAKEDIR}"
    NAMESPACE SDL_gesture::
    FILE SDL_gestureTargets.cmake
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/SDL_gestureConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/SDL_gestureConfigVersion.cmake"
    DESTINATION "${SDLGESTURE_INSTALL_CMAKEDIR}"
)
configure_file(cmake/sdl-gesture.pc.in sdl-gesture.pc @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sdl-gesture.pc"
    DESTINATION "${SDLGESTURE_INSTALL_PKGCONFIGDIR}"
)
install(FILES "LICENSE.txt" DESTINATION "${LICENSES_PREFIX}")
