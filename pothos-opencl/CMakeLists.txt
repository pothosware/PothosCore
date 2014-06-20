########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosOpenCL)
enable_language(CXX)

if (NOT POTHOS_MODULE_PATH)
    set(POTHOS_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/share/Pothos/cmake/Modules")
endif()

include(${POTHOS_MODULE_PATH}/PothosUtil.cmake)

########################################################################
## Find OpenCl
########################################################################
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
find_package(OpenCL)

message(STATUS "OPENCL_FOUND: ${OPENCL_FOUND}")
message(STATUS "OPENCL_INCLUDE_DIRS: ${OPENCL_INCLUDE_DIRS}")
message(STATUS "OPENCL_LIBRARIES: ${OPENCL_LIBRARIES}")

if(OPENCL_FOUND)
    include_directories(${OPENCL_INCLUDE_DIRS})
else()
    return()
endif()

set(SOURCES
    OpenClInfo.cpp
    OpenClErrToStr.cpp
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
