########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosLibrary)
enable_language(CXX)
enable_testing()

find_package(Pothos CONFIG REQUIRED)

########################################################################
# gather version information
# packagers may specify -DPOTHOS_EXTVER="foo" to replace the git hash
########################################################################
set(POTHOS_MAJOR_VERSION 0)
set(POTHOS_MINOR_VERSION 2)
set(POTHOS_PATCH_VERSION 1)
set(POTHOS_LIBVER ${POTHOS_MAJOR_VERSION}.${POTHOS_MINOR_VERSION}.${POTHOS_PATCH_VERSION})

if (NOT POTHOS_EXTVER)
    include(${PROJECT_SOURCE_DIR}/cmake/GetGitRevisionDescription.cmake)
    get_git_head_revision(GITREFSPEC GITHASH)
    if (GITHASH)
        string(SUBSTRING "${GITHASH}" 0 8 GITHASH)
        set(POTHOS_EXTVER "g${GITHASH}")
    else (GITHASH)
        set(POTHOS_EXTVER "unknown")
    endif (GITHASH)
endif()

set(POTHOS_VERSION "${POTHOS_LIBVER}-${POTHOS_EXTVER}")

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LIBRARY "Enable Pothos Library component" ON "ENABLE_SERIALIZATION" OFF)
add_feature_info(Library ENABLE_LIBRARY "The main library for all Pothos projects")
if (NOT ENABLE_LIBRARY)
    return()
endif()

########################################################################
# Install cmake helper modules
########################################################################
configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/PothosConfigVersion.in.cmake
    ${PROJECT_BINARY_DIR}/PothosConfigVersion.cmake
@ONLY)

install(
    FILES
        cmake/PothosConfig.cmake
        cmake/PothosStandardFlags.cmake
        cmake/PothosLibraryConfig.cmake
        cmake/PothosUtil.cmake
        ${PROJECT_BINARY_DIR}/PothosConfigVersion.cmake
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
