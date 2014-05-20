########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosManaged)
enable_language(CXX)

if (NOT POTHOS_MODULE_PATH)
    set(POTHOS_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/share/Pothos/cmake/Modules")
endif()

include(${POTHOS_MODULE_PATH}/PothosUtil.cmake)

########################################################################
# Build the module
########################################################################
set(SOURCES
    ManagedProxy.cpp
    ManagedHandle.cpp
    TestManaged.cpp
    TestManagedOpaque.cpp
    TestManagedWildcard.cpp
)

POTHOS_MODULE_UTIL(
    TARGET ManagedSupport
    SOURCES ${SOURCES}
    DESTINATION proxy/environment
)
