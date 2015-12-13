########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosAudio CXX)

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
endif()

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_AUDIO "Enable Pothos Audio component" ON "Pothos_FOUND;PORTAUDIO_FOUND" OFF)
add_feature_info(Audio ENABLE_AUDIO "Streaming audio source and sink blocks")
if (NOT ENABLE_AUDIO)
    return()
endif()

########################################################################
## Build and install module
########################################################################
include_directories(${PORTAUDIO_INCLUDE_DIRS})
add_definitions(${PORTAUDIO_DEFINITIONS})
POTHOS_MODULE_UTIL(
    TARGET AudioSupport
    SOURCES
        AudioSource.cpp
        AudioSink.cpp
        AudioInfo.cpp
    LIBRARIES ${PORTAUDIO_LIBRARIES}
    DESTINATION audio
    ENABLE_DOCS
)
