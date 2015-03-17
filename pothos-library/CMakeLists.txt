########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosLibrary)
enable_language(CXX)
enable_testing()

set(POTHOS_MAJOR_VERSION 0)
set(POTHOS_MINOR_VERSION 2)
set(POTHOS_PATCH_VERSION 0)
set(POTHOS_VERSION ${POTHOS_MAJOR_VERSION}.${POTHOS_MINOR_VERSION}.${POTHOS_PATCH_VERSION})

find_package(Pothos CONFIG REQUIRED)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
option(ENABLE_LIBRARY "Enable Pothos Library component" ON)
add_feature_info(Library ENABLE_LIBRARY "The main library for all Pothos projects")
if (NOT ENABLE_LIBRARY)
    return()
endif()

########################################################################
# Install cmake helper modules
########################################################################
install(
    FILES
        cmake/PothosConfig.cmake
        cmake/PothosStandardFlags.cmake
        cmake/PothosLibraryConfig.cmake
        cmake/PothosUtil.cmake
        #poco support
        cmake/FindPkgMacros.cmake
        cmake/FindPoco.cmake
        cmake/SetupPoco.cmake
    DESTINATION ${POTHOS_CMAKE_DIRECTORY}
    COMPONENT pothos_devel
)

########################################################################
# Enter the subdirectory configuration
########################################################################
add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(apps)
add_subdirectory(docs)

########################################################################
# Print summary
########################################################################
message(STATUS "######################################################")
message(STATUS "## Pothos library build summary:")
message(STATUS "##   Destination: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "##   Version: v${POTHOS_VERSION}")
message(STATUS "######################################################")
