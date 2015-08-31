########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosPython CXX)

find_package(Pothos CONFIG REQUIRED)
list(INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/cmake)
include(PothosPythonUtil)

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

#on windows, we require a pythonxx_d.lib in debug mode
#require that the PYTHON_DEBUG_LIBRARY flag is set
#or the build assumes that the debug library DNE
set(PYTHON_DEBUG_OK TRUE)
if(WIN32 AND NOT PYTHON_DEBUG_LIBRARY AND "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message(WARNING "WIN32 Debug mode requires PYTHON_DEBUG_LIBRARY")
    set(PYTHON_DEBUG_OK FALSE)
endif()

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_PYTHON "Enable Pothos Python component" ON "ENABLE_LIBRARY;PYTHONLIBS_FOUND;PYTHON_DEBUG_OK" OFF)
add_feature_info(Python ENABLE_PYTHON "Python bindings for the Pothos framework")
if (NOT ENABLE_PYTHON)
    return()
endif()

########################################################################
## Python<->Pothos proxy wrapper
########################################################################
include_directories(${PYTHON_INCLUDE_DIRS})

#generate include with build configuration params
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/PythonSupport.in.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/PythonSupport.hpp
@ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

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

########################################################################
# Install cmake helper modules
########################################################################
install(
    FILES
        cmake/PothosPythonUtil.cmake
        cmake/PothosPythonBlockFactory.in.cpp
    DESTINATION ${POTHOS_CMAKE_DIRECTORY}
)

########################################################################
# Enter the subdirectory configuration
########################################################################
add_subdirectory(Pothos)
add_subdirectory(TestBlocks)
