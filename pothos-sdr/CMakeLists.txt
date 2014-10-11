########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosSDR)
enable_language(CXX)

if (NOT POTHOS_MODULE_PATH)
    set(POTHOS_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/share/Pothos/cmake/Modules")
endif()

include(${POTHOS_MODULE_PATH}/PothosUtil.cmake)

########################################################################
## Find SoapySDR
########################################################################
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
find_package(SoapySDR)
if (SOAPY_SDR_FOUND)
    message(STATUS "SOAPY_SDR_INCLUDE_DIRS: ${SOAPY_SDR_INCLUDE_DIRS}")
    message(STATUS "SOAPY_SDR_LIBRARIES: ${SOAPY_SDR_LIBRARIES}")
else()
    message(WARNING "Pothos SDR blocks require SoapySDR to continue")
    return()
endif()

########################################################################
## Build and install module
########################################################################
include_directories(${SOAPY_SDR_INCLUDE_DIRS})
POTHOS_MODULE_UTIL(
    TARGET SDRSupport
    SOURCES SDRSource.cpp SDRSink.cpp
    LIBRARIES ${SOAPY_SDR_LIBRARIES}
    DESTINATION blocks/sdr
    ENABLE_DOCS
)
