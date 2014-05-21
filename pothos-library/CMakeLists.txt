########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosLibrary)
enable_language(CXX)
enable_testing()

set(POTHOS_MAJOR_VERSION 0)
set(POTHOS_MINOR_VERSION 1)
set(POTHOS_PATCH_VERSION 0)
set(POTHOS_VERSION ${POTHOS_MAJOR_VERSION}.${POTHOS_MINOR_VERSION}.${POTHOS_PATCH_VERSION})

#TODO this project can only be built as a submodule for now
#we expect POTHOS_SOURCE_DIR, POTHOS_BINARY_DIR, POTHOS_LIBRARIES, POTHOS_INCLUDE_DIRS
#set(POTHOS_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
#include(PothosProjectDefaults)

########################################################################
# Install cmake helper modules
########################################################################
install(
    DIRECTORY cmake/Modules
    DESTINATION share/Pothos/cmake
    COMPONENT pothos_devel
)

########################################################################
# Enter the subdirectory configuration
########################################################################
include(PothosStandardFlags)

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(apps)
add_subdirectory(tests)
