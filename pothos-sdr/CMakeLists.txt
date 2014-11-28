########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosSDR)
enable_language(CXX)

find_package(Pothos CONFIG REQUIRED)

########################################################################
## Find SoapySDR
########################################################################
find_package(SoapySDR CONFIG)

if (SoapySDR_FOUND)
    message(STATUS "SoapySDR_INCLUDE_DIRS: ${SoapySDR_INCLUDE_DIRS}")
    message(STATUS "SoapySDR_LIBRARIES: ${SoapySDR_LIBRARIES}")
else()
    message(WARNING "Pothos SDR blocks require SoapySDR, skipping...")
    return()
endif()

########################################################################
## Generate block descriptions
########################################################################
set(TITLE "Source")
set(CATEGORY "/Sources")
set(PATH "/sdr/source")
set(XCVR "receiver")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/SDRBlockDesc.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/SDRSourceDesc.hpp
@ONLY)

set(TITLE "Sink")
set(CATEGORY "/Sinks")
set(PATH "/sdr/sink")
set(XCVR "transmitter")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/SDRBlockDesc.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/SDRSinkDesc.hpp
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
        DemoController.cpp
        Logger.cpp
        SDRInfo.cpp
    LIBRARIES ${SoapySDR_LIBRARIES}
    DESTINATION sdr
    DOC_SOURCES
        DemoController.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/SDRSourceDesc.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/SDRSinkDesc.hpp
)
