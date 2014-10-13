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
find_package(SoapySDR NO_MODULE)

if (SoapySDR_FOUND)
    message(STATUS "SOAPY_SDR_INCLUDE_DIRS: ${SOAPY_SDR_INCLUDE_DIRS}")
    message(STATUS "SOAPY_SDR_LIBRARIES: ${SOAPY_SDR_LIBRARIES}")
else()
    message(WARNING "Pothos SDR blocks require SoapySDR to continue")
    return()
endif()

########################################################################
## Generate block descriptions
########################################################################
set(TITLE "Source")
set(CATEGORY "/Sources")
set(PATH "/sdr/source")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/SDRBlockDesc.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/SDRSourceDesc.hpp
@ONLY)

########################################################################
## Build and install module
########################################################################
include_directories(${SoapySDR_INCLUDE_DIRS})
POTHOS_MODULE_UTIL(
    TARGET SDRSupport
    SOURCES
        SDRBlock.cpp
        SDRSource.cpp
        SDRSink.cpp
    LIBRARIES ${SoapySDR_LIBRARIES}
    DESTINATION sdr
    DOC_SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/SDRSourceDesc.hpp
)
