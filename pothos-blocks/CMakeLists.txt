########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosBlocks)
enable_language(CXX)

find_package(Pothos CONFIG REQUIRED)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_BLOCKS "Enable Pothos Blocks component" ON "ENABLE_LIBRARY" OFF)
add_feature_info(Blocks ENABLE_BLOCKS "A collection of core processing blocks")
if (NOT ENABLE_BLOCKS)
    return()
endif()

########################################################################
# Build blocks modules
########################################################################
add_subdirectory(math)
add_subdirectory(file)
add_subdirectory(filter)
add_subdirectory(utility)
add_subdirectory(network)
add_subdirectory(packet)
add_subdirectory(serialize)
add_subdirectory(testers)
add_subdirectory(waveform)
add_subdirectory(digital)
