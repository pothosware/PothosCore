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
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/PothosConfigVersion.cmake
    VERSION ${POTHOS_VERSION}
    COMPATIBILITY AnyNewerVersion)

install(FILES
    ${PROJECT_SOURCE_DIR}/cmake/Modules/PothosConfig.cmake
    ${PROJECT_SOURCE_DIR}/cmake/Modules/PothosUtil.cmake
    ${PROJECT_BINARY_DIR}/PothosConfigVersion.cmake
    #poco support
    ${PROJECT_SOURCE_DIR}/cmake/Modules/FindPkgMacros.cmake
    ${PROJECT_SOURCE_DIR}/cmake/Modules/FindPoco.cmake
    ${PROJECT_SOURCE_DIR}/cmake/Modules/SetupPoco.cmake
    #SIMD
    ${PROJECT_SOURCE_DIR}/cmake/Modules/PothosConfigSIMD.cmake

    DESTINATION ${POTHOS_CMAKE_DIRECTORY}
    COMPONENT pothos_devel)
