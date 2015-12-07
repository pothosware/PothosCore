########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosBlocks CXX)

find_package(Pothos CONFIG REQUIRED)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_BLOCKS "Enable Pothos Blocks component" ON "ENABLE_LIBRARY" OFF)
add_feature_info(Blocks ENABLE_BLOCKS "A collection of general purpose blocks")

########################################################################
# Build subdirectories
########################################################################
add_subdirectory(file)
add_subdirectory(event)
add_subdirectory(network)
add_subdirectory(packet)
add_subdirectory(serialize)
add_subdirectory(stream)
add_subdirectory(testers)
