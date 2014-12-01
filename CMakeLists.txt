########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosAudio)
enable_language(CXX)

find_package(Pothos CONFIG REQUIRED)

########################################################################
# Find PortAudio
########################################################################
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
find_package(Portaudio)

if (PORTAUDIO_FOUND)
    message(STATUS "PORTAUDIO_INCLUDE_DIRS: ${PORTAUDIO_INCLUDE_DIRS}")
    message(STATUS "PORTAUDIO_LIBRARIES: ${PORTAUDIO_LIBRARIES}")
    message(STATUS "PORTAUDIO_DEFINITIONS: ${PORTAUDIO_DEFINITIONS}")
else()
    message(WARNING "Pothos Audio blocks require PortAudio, skipping...")
    return()
endif()

########################################################################
## Build and install module
########################################################################
include_directories(${PORTAUDIO_INCLUDE_DIRS})
add_definitions(${PORTAUDIO_DEFINITIONS})

#TODO
