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
cmake_dependent_option(ENABLE_COMMS "Enable Pothos Comms component" ON "ENABLE_LIBRARY" OFF)
add_feature_info(Comms ENABLE_COMMS "A collection of DSP/communications blocks")

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
