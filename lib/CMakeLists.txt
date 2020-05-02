########################################################################
# muparserx library
########################################################################
if(NOT MUPARSERX_IN_TREE)
    find_package(muparserx CONFIG)
endif(NOT MUPARSERX_IN_TREE)

if (muparserx_FOUND)

    message(STATUS "muparserx_VERSION: ${muparserx_VERSION}")
    message(STATUS "muparserx_INCLUDE_DIRS: ${muparserx_INCLUDE_DIRS}")
    message(STATUS "muparserx_LIBRARIES: ${muparserx_LIBRARIES}")

    list(APPEND Pothos_INCLUDE_DIRS ${muparserx_INCLUDE_DIRS})
    list(APPEND Pothos_LIBRARIES ${muparserx_LIBRARIES})
    list(APPEND PC_LIBS_PRIVATE "-lmuparserx")

else (muparserx_FOUND)
    message(WARNING "Pothos library requires dependency muparserx...")
endif (muparserx_FOUND)

########################################################################
# json.hpp header
########################################################################
find_path(JSON_HPP_INCLUDE_DIR NAMES json.hpp PATH_SUFFIXES nlohmann)

list(APPEND Pothos_INCLUDE_DIRS ${JSON_HPP_INCLUDE_DIR})

if (NOT JSON_HPP_INCLUDE_DIR)
    message(WARNING "Pothos library requires dependency json.hpp...")
endif (NOT JSON_HPP_INCLUDE_DIR)

########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_LIBRARY_LIB "Enable Pothos Library.Lib component" ON "ENABLE_LIBRARY;muparserx_FOUND;JSON_HPP_INCLUDE_DIR" OFF)
add_feature_info("  Lib" ENABLE_LIBRARY_LIB "Pothos development and runtime libraries")
if (NOT ENABLE_LIBRARY_LIB)
    return()
endif()

########################################################################
# Module file extension - Loader.in.cpp
########################################################################
if (WIN32)
    set(MODULE_EXT "dll")
elseif (UNIX)
    set(MODULE_EXT "so")
else()
    message(FATAL_ERROR "not win32 or unix")
endif()

########################################################################
# Local includes
########################################################################
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${Pothos_INCLUDE_DIRS})

########################################################################
# NUMA info support
########################################################################
find_library(
    NUMA_LIBRARIES
    NAMES numa
    PATHS /usr/lib /usr/lib64
)

if(NUMA_LIBRARIES)
    add_definitions(-DHAVE_LIBNUMA)
    list(APPEND POTHOS_SOURCES System/NumaInfoLinux.cpp)
    list(APPEND Pothos_LIBRARIES ${NUMA_LIBRARIES})
    list(APPEND PC_LIBS_PRIVATE "-lnuma")
elseif(WIN32)
    list(APPEND POTHOS_SOURCES System/NumaInfoWindows.cpp)
elseif(APPLE)
    list(APPEND POTHOS_SOURCES System/NumaInfoOSX.cpp)
else()
    list(APPEND POTHOS_SOURCES System/NumaInfoOther.cpp)
endif()

########################################################################
# Link libatomic
########################################################################
find_library(
    ATOMIC_LIBRARIES
    NAMES atomic atomic.so.1 libatomic.so.1
    PATHS /usr/lib /usr/lib64
)

if (ATOMIC_LIBRARIES)
    list(APPEND Pothos_LIBRARIES ${ATOMIC_LIBRARIES})
    list(APPEND PC_LIBS_PRIVATE "-latomic")
endif()

########################################################################
# Thread config support
########################################################################
find_library(
    RT_LIBRARIES
    NAMES rt
    PATHS /usr/lib /usr/lib64
)

if (RT_LIBRARIES)
    list(APPEND Pothos_LIBRARIES ${RT_LIBRARIES})
    list(APPEND PC_LIBS_PRIVATE "-lrt")
endif()

if(WIN32)
    list(APPEND POTHOS_SOURCES Framework/ThreadConfigWindows.cpp)
elseif(UNIX)
    list(APPEND POTHOS_SOURCES Framework/ThreadConfigUnix.cpp)
endif()

########################################################################
# OSX support
########################################################################
if (APPLE)
    #Need to link to CoreFoundation to fix dlopen() issue:
    #http://openradar.appspot.com/7209349
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    list(APPEND Pothos_LIBRARIES ${COREFOUNDATION_LIBRARY})
endif()

########################################################################
# Library sources
########################################################################
list(APPEND POTHOS_SOURCES
    Init.cpp
    Testing.cpp
    Exception.cpp

    System/Logger.cpp
    System/Version.in.cpp
    System/Paths.in.cpp
    System/HostInfo.cpp
    System/NumaInfo.cpp
    System/Exception.cpp

    Object/Object.cpp
    Object/Hash.cpp
    Object/Compare.cpp
    Object/Convert.cpp
    Object/ToString.cpp
    Object/Serialize.cpp
    Object/Exception.cpp

    Object/Builtin/Compare.cpp
    Object/Builtin/ConvertIntermediate.cpp
    Object/Builtin/ConvertIntegers.cpp
    Object/Builtin/ConvertFloats.cpp
    Object/Builtin/ConvertString.cpp
    Object/Builtin/Hash.cpp
    Object/Builtin/Serialize.cpp
    Object/Builtin/Tests.cpp

    Callable/Callable.cpp
    Callable/CallInterface.cpp
    Callable/CallRegistry.cpp
    Callable/Exception.cpp
    Callable/Tests.cpp

    Framework/Packet.cpp
    Framework/DType.cpp
    Framework/Label.cpp
    Framework/InputPort.cpp
    Framework/OutputPort.cpp
    Framework/Connectable.cpp
    Framework/Block.cpp
    Framework/Topology.cpp
    Framework/TopologyCommit.cpp
    Framework/TopologySquashFlows.cpp
    Framework/TopologyNetworkFlows.cpp
    Framework/TopologyDomainFlows.cpp
    Framework/TopologyDotMarkup.cpp
    Framework/TopologyDumpJSON.cpp
    Framework/TopologyMakeJSON.cpp
    Framework/TopologyStatsJSON.cpp
    Framework/WorkInfo.cpp
    Framework/WorkerActor.cpp
    Framework/WorkerActorPortAllocation.cpp
    Framework/ThreadPool.cpp
    Framework/ThreadEnvironment.cpp
    Framework/SharedBuffer.cpp
    Framework/ManagedBuffer.cpp
    Framework/BufferPool.cpp
    Framework/BufferChunk.cpp
    Framework/BufferConvert.cpp
    Framework/BufferManager.cpp
    Framework/BufferAccumulator.cpp
    Framework/BlockRegistry.cpp
    Framework/Exception.cpp

    ConfLoader/ConfLoader.cpp
    ConfLoader/FileRealPath.cpp
    ConfLoader/BlockDescLoader.cpp
    ConfLoader/JSONTopologyLoader.cpp
    ConfLoader/JITCompilerLoader.cpp

    Framework/Builtin/CircularBufferManager.cpp
    Framework/Builtin/TestBufferChunkSerialization.cpp
    Framework/Builtin/TestToString.cpp
    Framework/Builtin/TestBufferConvert.cpp
    Framework/Builtin/TestDType.cpp
    Framework/Builtin/TestAutomaticPorts.cpp
    Framework/Builtin/TestSharedBuffer.cpp
    Framework/Builtin/GenericBufferManager.cpp
    Framework/Builtin/TestCircularBufferManager.cpp
    Framework/Builtin/TestGenericBufferManager.cpp
    Framework/Builtin/TestWorker.cpp
    Framework/Builtin/TestLabel.cpp
    Framework/Builtin/TestThreadPool.cpp
    Framework/Builtin/TestTopology.cpp

    Plugin/Path.cpp
    Plugin/Plugin.cpp
    Plugin/Registry.cpp
    Plugin/Module.cpp
    Plugin/ModuleSafeLoad.cpp
    Plugin/ModulePaths.cpp
    Plugin/Static.cpp
    Plugin/Exception.cpp
    Plugin/Loader.in.cpp
    Plugin/Tests.cpp

    Proxy/Proxy.cpp
    Proxy/Handle.cpp
    Proxy/Convert.cpp
    Proxy/Environment.cpp
    Proxy/Exception.cpp
    Proxy/Builtin/ConvertContainers.cpp

    Remote/RemoteProxyDatagram.cpp
    Remote/RemoteProxy.cpp
    Remote/RemoteProxyHandle.cpp
    Remote/Server.cpp
    Remote/ServerHandler.cpp
    Remote/Client.cpp
    Remote/Exception.cpp
    Remote/Builtin/TestRemote.cpp

    Managed/Class.cpp
    Managed/Registry.cpp
    Managed/Exception.cpp

    Managed/Builtin/ManagedHandle.cpp
    Managed/Builtin/ManagedProxy.cpp
    Managed/Builtin/ManagedProxy.hpp
    Managed/Builtin/TestManaged.cpp
    Managed/Builtin/TestManagedOpaque.cpp
    Managed/Builtin/TestManagedWildcard.cpp
    Managed/Builtin/TestManagedInheritance.cpp

    Util/UID.cpp
    Util/RefHolder.cpp
    Util/TypeInfo.cpp
    Util/Compiler.cpp
    Util/Network.cpp
    Util/EvalEnvironment.cpp
    Util/EvalEnvironmentListParsers.cpp
    Util/BlockDescription.cpp

    Util/Builtin/BlockEval.cpp
    Util/Builtin/DeviceInfoUtils.cpp
    Util/Builtin/DocUtils.cpp
    Util/Builtin/MupOctalValReader.cpp
    Util/Builtin/TestDocUtils.cpp
    Util/Builtin/TestEvalExpression.cpp
    Util/Builtin/TestRingDeque.cpp

    Archive/ArchiveEntry.cpp
    Archive/StreamArchiver.cpp
    Archive/TestBasicTypes.cpp
    Archive/TestCustomType.cpp
    Archive/TestPolymorphicType.cpp
    Archive/Exception.cpp
)

if(WIN32)
    list(APPEND POTHOS_SOURCES WindowsDelayLoadedSymbols.cpp)
    list(APPEND POTHOS_SOURCES Framework/SharedBufferWindows.cpp)
    list(APPEND POTHOS_SOURCES Util/FileLockWindows.cpp)
    list(APPEND POTHOS_SOURCES Util/Builtin/WindowsGetLogicalProcessorInfo.cpp)
elseif(UNIX)
    list(APPEND POTHOS_SOURCES Framework/SharedBufferUnix.cpp)
    list(APPEND POTHOS_SOURCES Util/FileLockUnix.cpp)
endif()

########################################################################
# Compiler support wrapper
########################################################################
list(APPEND POTHOS_SOURCES Util/Builtin/TestCompilerSupport.cpp)

if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND POTHOS_SOURCES Util/Builtin/GccCompilerSupport.cpp)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    list(APPEND POTHOS_SOURCES Util/Builtin/ClangCompilerSupport.cpp)
endif()

if(MSVC)
    get_filename_component(MSVC_INSTALL_PATH ${CMAKE_LINKER} PATH)
    file(TO_NATIVE_PATH ${MSVC_INSTALL_PATH} MSVC_INSTALL_PATH)
    string(REPLACE "\\" "\\\\" MSVC_INSTALL_PATH ${MSVC_INSTALL_PATH})
    list(APPEND POTHOS_SOURCES Util/Builtin/MsvcCompilerSupport.in.cpp)
endif(MSVC)

########################################################################
# Deal with *.in files in the sources list
########################################################################
unset(__POTHOS_SOURCES)
foreach(source ${POTHOS_SOURCES})
    get_filename_component(source_ext ${CMAKE_CURRENT_SOURCE_DIR}/${source} EXT)
    get_filename_component(source_we ${CMAKE_CURRENT_SOURCE_DIR}/${source} NAME_WE)
    get_filename_component(binary_path ${CMAKE_CURRENT_BINARY_DIR}/${source} PATH)
    if ("${source_ext}" STREQUAL ".in.cpp")
        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${binary_path})
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/${source}
            ${CMAKE_CURRENT_BINARY_DIR}/${source_we}.cpp
        @ONLY)
        list(APPEND __POTHOS_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${source_we}.cpp)
    else()
        if (EXISTS ${source})
            list(APPEND __POTHOS_SOURCES ${source})
        else()
            list(APPEND __POTHOS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${source})
        endif()
    endif()
endforeach(source)

set(POTHOS_SOURCES ${__POTHOS_SOURCES})

########################################################################
# Build library
########################################################################
if(MSVC)
    add_definitions(/bigobj) #builtin convert.cpp too large in debug mode
endif(MSVC)

add_library(Pothos SHARED ${POTHOS_SOURCES})
list(REMOVE_ITEM Pothos_LIBRARIES Pothos) #cant link to self
target_link_libraries(Pothos ${Pothos_LIBRARIES})
set_target_properties(Pothos PROPERTIES SOVERSION ${POTHOS_ABI_VERSION})
set_target_properties(Pothos PROPERTIES VERSION ${POTHOS_LIBVER})
set_target_properties(Pothos PROPERTIES DEFINE_SYMBOL "POTHOS_DLL_EXPORTS")

install(TARGETS Pothos
    LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT pothos_runtime # .so file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT pothos_devel   # .lib file
    RUNTIME DESTINATION bin              COMPONENT pothos_runtime # .dll file
)

#get the library name into Paths.cpp for getPothosRuntimeLibraryPath()
add_definitions("-DPOTHOS_LIBRARY_NAME=\"$<TARGET_FILE_NAME:Pothos>\"")

########################################################################
# Build pkg config file
########################################################################
string(REPLACE ";" " " PC_LIBS_PRIVATE "${PC_LIBS_PRIVATE}")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Pothos.in.pc
    ${CMAKE_CURRENT_BINARY_DIR}/Pothos.pc
@ONLY)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/Pothos.pc
    DESTINATION lib${LIB_SUFFIX}/pkgconfig
    COMPONENT pothos_devel
)
