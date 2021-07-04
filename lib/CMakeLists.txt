########################################################################
## locate the Poco development libraries
########################################################################
enable_language(C) #newer Poco config scrips require c to be enabled
include(SetupPoco)

if (Poco_FOUND)

    message(STATUS "Poco_VERSION: ${Poco_VERSION}")
    message(STATUS "Poco_INCLUDE_DIRS: ${Poco_INCLUDE_DIRS}")
    message(STATUS "Poco_LIBRARIES: ${Poco_LIBRARIES}")

else (Poco_FOUND)
    message(WARNING "Pothos library requires Poco libraries...")
endif (Poco_FOUND)

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

else (muparserx_FOUND)
    message(WARNING "Pothos library requires dependency muparserx...")
endif (muparserx_FOUND)

########################################################################
# json.hpp header
########################################################################
find_path(JSON_HPP_INCLUDE_DIR NAMES json.hpp PATH_SUFFIXES nlohmann)

if (NOT JSON_HPP_INCLUDE_DIR)
    message(WARNING "Pothos library requires dependency json.hpp...")
endif (NOT JSON_HPP_INCLUDE_DIR)

########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_LIBRARY_LIB "Enable Pothos Library.Lib component" ON "ENABLE_LIBRARY;Poco_FOUND;muparserx_FOUND;JSON_HPP_INCLUDE_DIR" OFF)
add_feature_info("  Lib" ENABLE_LIBRARY_LIB "Pothos development and runtime libraries")
if (NOT ENABLE_LIBRARY_LIB)
    return()
endif()

########################################################################
# Library sources
########################################################################
add_library(Pothos SHARED
    Init.cpp
    Exception.cpp

    Testing/Testing.cpp

    System/Logger.cpp
    System/HostInfo.cpp
    System/NumaInfo.cpp
    System/Exception.cpp
    System/SIMD.cpp

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
    Object/Builtin/TestConvertVectors.cpp

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
    Framework/Builtin/TestDoesBlockExist.cpp
    Framework/Builtin/TestDType.cpp
    Framework/Builtin/TestAutomaticPorts.cpp
    Framework/Builtin/TestSharedBuffer.cpp
    Framework/Builtin/GenericBufferManager.cpp
    Framework/Builtin/TestCircularBufferManager.cpp
    Framework/Builtin/TestGenericBufferManager.cpp
    Framework/Builtin/TestBufferManagerWithCustomAllocation.cpp
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
    Util/Builtin/SIMDDispatcherUtils.cpp
    Util/Builtin/TestDocUtils.cpp
    Util/Builtin/TestErrorCodeHandling.cpp
    Util/Builtin/TestEvalExpression.cpp
    Util/Builtin/TestRingDeque.cpp
    Util/Builtin/TestSIMDDispatcherUtils.cpp

    Archive/ArchiveEntry.cpp
    Archive/StreamArchiver.cpp
    Archive/TestBasicTypes.cpp
    Archive/TestCustomType.cpp
    Archive/TestPolymorphicType.cpp
    Archive/Exception.cpp
)

if(WIN32)
    target_sources(Pothos PRIVATE WindowsDelayLoadedSymbols.cpp)
    target_sources(Pothos PRIVATE Framework/SharedBufferWindows.cpp)
    target_sources(Pothos PRIVATE Util/FileLockWindows.cpp)
    target_sources(Pothos PRIVATE Util/Builtin/WindowsGetLogicalProcessorInfo.cpp)
elseif(UNIX)
    target_sources(Pothos PRIVATE Framework/SharedBufferUnix.cpp)
    target_sources(Pothos PRIVATE Util/FileLockUnix.cpp)
endif()

target_link_libraries(Pothos PUBLIC ${Pothos_LINKER_FLAGS})
target_include_directories(Pothos PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Pothos PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
target_compile_options(Pothos PUBLIC ${Pothos_COMPILE_OPTIONS})
target_compile_definitions(Pothos PUBLIC ${Pothos_COMPILE_DEFINITIONS})
set_target_properties(Pothos PROPERTIES SOVERSION ${POTHOS_ABI_VERSION})
set_target_properties(Pothos PROPERTIES VERSION ${POTHOS_LIBVER})
set_target_properties(Pothos PROPERTIES DEFINE_SYMBOL "POTHOS_DLL_EXPORTS")

if(MSVC)
    target_compile_options(Pothos PRIVATE /bigobj) #builtin convert.cpp too large in debug mode
endif(MSVC)

########################################################################
# Link feature dependencies
########################################################################
if (Poco_INCLUDE_DIRS)
    target_include_directories(Pothos PUBLIC ${Poco_INCLUDE_DIRS})
endif (Poco_INCLUDE_DIRS)
target_link_libraries(Pothos PUBLIC ${Poco_LIBRARIES})

target_include_directories(Pothos PRIVATE ${muparserx_INCLUDE_DIRS})
target_link_libraries(Pothos PRIVATE ${muparserx_LIBRARIES})
list(APPEND PC_LIBS_PRIVATE "-lmuparserx")

target_include_directories(Pothos PRIVATE ${JSON_HPP_INCLUDE_DIR})

########################################################################
# Compiler flags that are generally always a good idea to set
# or sensible to use because of the project's constraints.
########################################################################

if(CMAKE_COMPILER_IS_GNUCXX OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))

    #these warnings are caused by static warnings used throughout the code
    target_compile_options(Pothos PRIVATE -Wno-unused-local-typedefs)

    #common warnings to help encourage good coding practices
    target_compile_options(Pothos PUBLIC -Wall)
    target_compile_options(Pothos PUBLIC -Wextra)
    target_compile_options(Pothos PUBLIC -Wnon-virtual-dtor)

    #symbols are only exported from libraries/modules explicitly
    target_compile_options(Pothos PRIVATE -fvisibility=hidden)
    target_compile_options(Pothos PRIVATE -fvisibility-inlines-hidden)
endif()

if(APPLE)
    #fixes issue with duplicate module registry when using application bundle
    target_link_libraries(Pothos PUBLIC "-flat_namespace")
endif()

if(MSVC)
    #we always want to use multiple cores for compilation
    target_compile_options(Pothos PUBLIC /MP)

    #projects should be cross-platform and standard stl functions should work
    target_compile_definitions(Pothos PUBLIC -DNOMINMAX) #enables std::min and std::max

    #suppress the following warnings which are commonly caused by project headers
    target_compile_options(Pothos PUBLIC /wd4251) #disable 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
    target_compile_options(Pothos PUBLIC /wd4275) #disable non – DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
    target_compile_options(Pothos PUBLIC /wd4503) #'identifier' : decorated name length exceeded, name was truncated
endif(MSVC)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
    target_compile_options(Pothos PUBLIC -stdlib=libc++)
endif()

########################################################################
# Configure sources
########################################################################

#get the library name into Paths.cpp for getPothosRuntimeLibraryPath()
target_compile_definitions(Pothos PRIVATE "-DPOTHOS_LIBRARY_NAME=\"$<TARGET_FILE_NAME:Pothos>\"")

if (WIN32)
    set(MODULE_EXT "dll")
elseif (UNIX)
    set(MODULE_EXT "so")
else()
    message(FATAL_ERROR "not win32 or unix")
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/System/Paths.in.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/Paths.cpp @ONLY)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/System/Version.in.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/Version.cpp @ONLY)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Plugin/Loader.in.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/Loader.cpp @ONLY)

target_sources(Pothos PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/Version.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/Paths.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/Loader.cpp
)

########################################################################
# NUMA info support
########################################################################
find_library(
    NUMA_LIBRARIES
    NAMES numa
    PATHS /usr/lib /usr/lib64
)

if(NUMA_LIBRARIES)
    target_compile_definitions(Pothos PRIVATE -DHAVE_LIBNUMA)
    target_sources(Pothos PRIVATE  System/NumaInfoLinux.cpp)
    target_link_libraries(Pothos PRIVATE ${NUMA_LIBRARIES})
    list(APPEND PC_LIBS_PRIVATE "-lnuma")
elseif(WIN32)
    target_sources(Pothos PRIVATE System/NumaInfoWindows.cpp)
elseif(APPLE)
    target_sources(Pothos PRIVATE System/NumaInfoOSX.cpp)
else()
    target_sources(Pothos PRIVATE System/NumaInfoOther.cpp)
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
    target_link_libraries(Pothos PRIVATE ${ATOMIC_LIBRARIES})
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
    target_link_libraries(Pothos PRIVATE ${RT_LIBRARIES})
    list(APPEND PC_LIBS_PRIVATE "-lrt")
endif()

if(WIN32)
    target_sources(Pothos PRIVATE Framework/ThreadConfigWindows.cpp)
elseif(UNIX)
    target_sources(Pothos PRIVATE Framework/ThreadConfigUnix.cpp)
endif()

########################################################################
# OSX support
########################################################################
if (APPLE)
    #Need to link to CoreFoundation to fix dlopen() issue:
    #http://openradar.appspot.com/7209349
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    target_link_libraries(Pothos PRIVATE ${COREFOUNDATION_LIBRARY})
endif()

########################################################################
# Compiler support wrapper
########################################################################
target_sources(Pothos PRIVATE Util/Builtin/TestCompilerSupport.cpp)

if(CMAKE_COMPILER_IS_GNUCXX)
    target_sources(Pothos PRIVATE Util/Builtin/GccCompilerSupport.cpp)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_sources(Pothos PRIVATE Util/Builtin/ClangCompilerSupport.cpp)
endif()

if(MSVC)
    get_filename_component(MSVC_INSTALL_PATH ${CMAKE_LINKER} PATH)
    file(TO_NATIVE_PATH ${MSVC_INSTALL_PATH} MSVC_INSTALL_PATH)
    string(REPLACE "\\" "\\\\" MSVC_INSTALL_PATH ${MSVC_INSTALL_PATH})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Util/Builtin/MsvcCompilerSupport.in.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/MsvcCompilerSupport.cpp @ONLY)
    target_sources(Pothos PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/MsvcCompilerSupport.cpp)
endif(MSVC)

########################################################################
# Generate dynamic dispatch files for SIMD code
########################################################################
target_include_directories(Pothos PRIVATE ${PROJECT_SOURCE_DIR}/libsimdpp)

include(${CMAKE_CURRENT_SOURCE_DIR}/../libsimdpp/cmake/SimdppMultiarch.cmake)

set(simd_sources
    SIMD/SIMDConvert.cpp)

simdpp_get_runnable_archs(simdpp_runnable_archs)
simdpp_multiarch(
    GeneratedSIMDFiles
    ${simd_sources}
    ${simdpp_runnable_archs})
target_sources(Pothos PRIVATE ${GeneratedSIMDFiles})

########################################################################
# Install the library
########################################################################
install(TARGETS Pothos
    EXPORT PothosExport
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT pothos_runtime # .so file
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT pothos_devel   # .lib file
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT pothos_runtime # .dll file
)

#export target to Pothos project config
install(EXPORT PothosExport DESTINATION ${POTHOS_CMAKE_DIRECTORY})

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
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    COMPONENT pothos_devel
)
