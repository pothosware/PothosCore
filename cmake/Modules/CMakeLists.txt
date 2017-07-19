########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_LIBRARY_CMAKE "Enable Pothos Library.CMake component" ON "ENABLE_LIBRARY" OFF)
add_feature_info("  CMake" ENABLE_LIBRARY_CMAKE "CMake project utilities for Pothos library")
if (NOT ENABLE_LIBRARY_CMAKE)
    return()
endif()

########################################################################
# Install cmake helper modules
########################################################################
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/PothosConfigVersion.in.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/PothosConfigVersion.cmake
@ONLY)

install(
    FILES
        PothosConfig.cmake
        PothosStandardFlags.cmake
        PothosLibraryConfig.cmake
        PothosUtil.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/PothosConfigVersion.cmake
        #poco support
        FindPkgMacros.cmake
        FindPoco.cmake
        SetupPoco.cmake
    DESTINATION ${POTHOS_CMAKE_DIRECTORY}
    COMPONENT pothos_devel
)
