########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosLibrary CXX)

set(Pothos_DIR ${PROJECT_SOURCE_DIR}/cmake  CACHE STRING "")
set(POTHOS_IN_TREE_SOURCE_DIR ${PROJECT_SOURCE_DIR} CACHE STRING "")
find_package(Pothos CONFIG REQUIRED)

########################################################################
# gather version information
# packagers may specify -DPOTHOS_EXTVER="foo" to replace the git hash
########################################################################
set(POTHOS_MAJOR_VERSION 0)
set(POTHOS_MINOR_VERSION 3)
set(POTHOS_PATCH_VERSION 0)
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

#POTHOS_ROOT is compiled into the library to locate the install base.
#By default, the POTHOS_ROOT is set to the CMAKE_INSTALL_PREFIX.
#However users may overload this by specifying -DPOTHOS_ROOT=<path>.
if(NOT POTHOS_ROOT)
    file(TO_CMAKE_PATH ${CMAKE_INSTALL_PREFIX} POTHOS_ROOT)
endif(NOT POTHOS_ROOT)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LIBRARY "Enable Pothos Library component" ON "TRUE" OFF)
add_feature_info(Library ENABLE_LIBRARY "The main library for all Pothos projects")

########################################################################
# Enter the subdirectory configuration
########################################################################
add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(cmake)
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
