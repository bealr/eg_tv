cmake_minimum_required(VERSION 3.15)

project(eg_tv C)

set(CMAKE_C_STANDARD 99)

find_package(PkgConfig REQUIRED)

# Raylib
find_package(raylib REQUIRED)

# GStreamer
pkg_check_modules(GSTREAMER REQUIRED
    gstreamer-1.0
    gstreamer-video-1.0
    gstreamer-audio-1.0
)

add_executable(eg_tv main.c menu.c files.c player.c)

target_include_directories(eg_tv PRIVATE
    ${GSTREAMER_INCLUDE_DIRS}
)

target_link_directories(eg_tv PRIVATE
    ${GSTREAMER_LIBRARY_DIRS}
)

target_compile_options(eg_tv PRIVATE
    ${GSTREAMER_CFLAGS_OTHER}
)

target_link_libraries(eg_tv PRIVATE
    raylib
    ${GSTREAMER_LIBRARIES}
)