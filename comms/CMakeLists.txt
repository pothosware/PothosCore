########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosComms CXX)

find_package(Pothos CONFIG REQUIRED)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_COMMS "Enable Pothos Comms component" ON "Pothos_FOUND" OFF)
add_feature_info(Comms ENABLE_COMMS "A collection of DSP/communications blocks")

########################################################################
# Spuce library
########################################################################
if(NOT SPUCE_IN_TREE)
    find_package(Spuce CONFIG)
endif(NOT SPUCE_IN_TREE)

if (Spuce_FOUND)
    message(STATUS "Spuce_VERSION: ${Spuce_VERSION}")
    message(STATUS "Spuce_INCLUDE_DIRS: ${Spuce_INCLUDE_DIRS}")
    message(STATUS "Spuce_LIBRARIES: ${Spuce_LIBRARIES}")
else (Spuce_FOUND)
    message(WARNING "Spuce filter designer library not found...")
endif (Spuce_FOUND)

########################################################################
# Build subdirectories
########################################################################
add_subdirectory(demod)
add_subdirectory(digital)
add_subdirectory(fft)
add_subdirectory(filter)
add_subdirectory(mac)
add_subdirectory(math)
add_subdirectory(utility)
add_subdirectory(waveform)
