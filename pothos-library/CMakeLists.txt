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

find_package(Pothos NO_MODULE REQUIRED)

########################################################################
# Install cmake helper modules
########################################################################
install(
    FILES
        cmake/PothosConfig.cmake
        cmake/PothosStandardFlags.cmake
        cmake/PothosUtil.cmake
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
