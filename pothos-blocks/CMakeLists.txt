########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosBlocks)
enable_language(CXX)

find_package(Pothos NO_MODULE REQUIRED)

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
