########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosPython)
enable_language(CXX)

if (NOT POTHOS_MODULE_PATH)
    set(POTHOS_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/share/Pothos/cmake/Modules")
endif()

include(${POTHOS_MODULE_PATH}/PothosUtil.cmake)

########################################################################
## Find python
########################################################################
find_package(PythonLibs)

message(STATUS "PYTHONLIBS_FOUND: ${PYTHONLIBS_FOUND} - ${PYTHONLIBS_VERSION_STRING}")
message(STATUS "PYTHON_INCLUDE_DIRS: ${PYTHON_INCLUDE_DIRS}")
message(STATUS "PYTHON_LIBRARIES: ${PYTHON_LIBRARIES}")

# enable python debug mode when the debug library is specified
if (PYTHON_DEBUG_LIBRARY)
    add_definitions(-DPy_DEBUG)
    set(PYTHON_DEBUG_POSTFIX _d)
else()
    unset(PYTHON_DEBUG_POSTFIX)
endif()

if(NOT PYTHONLIBS_FOUND)
    return()
endif()

########################################################################
## Python<->Pothos proxy wrapper
########################################################################
include_directories(${PYTHON_INCLUDE_DIRS})

set(SOURCES
   PythonProxy.cpp
   PythonHandle.cpp
   PythonConvert.cpp
   TestPython.cpp
   TestPythonBlock.cpp
   PythonBlock.cpp
   ProxyHelpers.cpp
)

POTHOS_MODULE_UTIL(
    TARGET PythonSupport
    SOURCES ${SOURCES}
    LIBRARIES ${PYTHON_LIBRARIES}
    DESTINATION proxy/environment
)

add_subdirectory(Pothos)
