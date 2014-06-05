########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosBlocks)
enable_language(CXX)

if (NOT POTHOS_MODULE_PATH)
    set(POTHOS_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/share/Pothos/cmake/Modules")
endif()

include(${POTHOS_MODULE_PATH}/PothosUtil.cmake)

########################################################################
# Build blocks module
########################################################################
set(SOURCES
    TestUnitTestBlocks.cpp
    TestNetworkTopology.cpp
    TestSignalsAndSlots.cpp
    TestProxyTopology.cpp
    sources/WaveformSource.cpp
    sources/FeederSource.cpp
    sinks/CollectorSink.cpp
    sinks/BlackHole.cpp
    misc/Delay.cpp
    misc/Forwarder.cpp
    misc/MessageToSignal.cpp
    misc/SlotToMessage.cpp
    network/NetworkSource.cpp
    network/NetworkSink.cpp
    network/SocketEndpoint.cpp
    network/TestNetworkBlocks.cpp
)

#udt support
file(GLOB UDT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/network/udt4/src/*.cpp)
add_definitions(-DUDT_EXPORTS)
list(APPEND SOURCES ${UDT_SOURCES})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/network/udt4/src)

POTHOS_MODULE_UTIL(
    TARGET GeneralBlocks
    SOURCES ${SOURCES}
    DESTINATION blocks
    ENABLE_DOCS
)

########################## GPL bounded code ############################
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(SOURCES
    sources/NoiseSource.cpp
)

POTHOS_MODULE_UTIL(
    TARGET GeneralBlocksGPL
    SOURCES ${SOURCES}
    DESTINATION blocks
    ENABLE_DOCS
)
