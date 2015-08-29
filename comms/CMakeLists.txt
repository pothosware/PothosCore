########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosComms)
enable_language(CXX)

find_package(Pothos CONFIG REQUIRED)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_COMMS "Enable Pothos Comms component" ON "ENABLE_LIBRARY" OFF)
add_feature_info(Comms ENABLE_COMMS "A collection of communications blocks")
if (NOT ENABLE_COMMS)
    return()
endif()

########################################################################
# Build blocks modules
########################################################################
add_subdirectory(math)
add_subdirectory(filter)
add_subdirectory(waveform)
add_subdirectory(digital)
add_subdirectory(utility)
add_subdirectory(mac)
