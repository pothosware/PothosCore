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
    waveform/WaveformSource.cpp
    waveform/NoiseSource.cpp

    testers/TestUnitTestBlocks.cpp
    testers/TestProxyTopology.cpp
    testers/TestSetThreadPool.cpp
    testers/FeederSource.cpp
    testers/CollectorSink.cpp

    misc/Delay.cpp
    misc/Pacer.cpp
    misc/PeriodicTrigger.cpp
    misc/TestPeriodicTrigger.cpp
    misc/Forwarder.cpp
    misc/TestForwarder.cpp
    misc/Copier.cpp
    misc/MessageToSignal.cpp
    misc/SlotToMessage.cpp
    misc/BlackHole.cpp
    misc/TestSignalsAndSlots.cpp
    misc/DynamicRouter.cpp

    network/NetworkSource.cpp
    network/NetworkSink.cpp
    network/SocketEndpoint.cpp
    network/TestNetworkBlocks.cpp
    network/TestNetworkTopology.cpp

    arithmetic/Arithmetic.cpp
    arithmetic/TestArithmeticBlocks.cpp

    file/BinaryFileSource.cpp
    file/BinaryFileSink.cpp
    file/TestBinaryFileBlocks.cpp

    serialize/Serializer.cpp
    serialize/Deserializer.cpp
    serialize/TestSerialize.cpp
)

#udt support
file(GLOB UDT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/network/udt4/src/*.cpp)
add_definitions(-DUDT_EXPORTS)
list(APPEND SOURCES ${UDT_SOURCES})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/network/udt4/src)

if(MSVC)
    add_definitions(/bigobj) #arithmetic too large in debug mode
endif(MSVC)

POTHOS_MODULE_UTIL(
    TARGET GeneralBlocks
    SOURCES ${SOURCES}
    DESTINATION blocks
    ENABLE_DOCS
)
