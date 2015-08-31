########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosOpenCL CXX)

find_package(Pothos CONFIG REQUIRED)

########################################################################
## Find OpenCl
########################################################################
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
find_package(OpenCL)

message(STATUS "OPENCL_FOUND: ${OPENCL_FOUND}")
message(STATUS "OPENCL_INCLUDE_DIRS: ${OPENCL_INCLUDE_DIRS}")
message(STATUS "OPENCL_LIBRARIES: ${OPENCL_LIBRARIES}")

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_OPENCL "Enable Pothos OpenCL component" ON "ENABLE_LIBRARY;OPENCL_FOUND" OFF)
add_feature_info(OpenCL ENABLE_OPENCL "Support for OpenCL-based processing blocks")
if (NOT ENABLE_OPENCL)
    return()
endif()

########################################################################
## Build the module
########################################################################
include_directories(${OPENCL_INCLUDE_DIRS})

set(SOURCES
    OpenClInfo.cpp
    OpenClErrToStr.cpp
    OpenClContextCache.cpp
    OpenClKernel.cpp
    OpenClBufferManager.cpp
    TestOpenClBlocks.cpp
)

POTHOS_MODULE_UTIL(
    TARGET OpenClBlocks
    SOURCES ${SOURCES}
    LIBRARIES ${OPENCL_LIBRARIES}
    DESTINATION blocks/opencl
    ENABLE_DOCS
)
