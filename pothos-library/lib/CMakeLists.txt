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
include_directories(${POTHOS_INCLUDE_DIRS})

########################################################################
# Sigar dependency
########################################################################
include_directories(${SIGAR_INCLUDE_DIRS})
list(APPEND POTHOS_LIBRARIES ${SIGAR_LIBRARIES})

########################################################################
# Theron dependency
########################################################################
set(THERON_SOURCE_DIR ${POTHOS_SOURCE_DIR}/theron)
include(${CMAKE_CURRENT_SOURCE_DIR}/TheronSetup.cmake)
include_directories(${THERON_INCLUDE_DIRS})
link_directories(${THERON_LIBRARY_DIRS})
list(APPEND POTHOS_LIBRARIES ${THERON_LIBRARIES})
add_definitions(${THERON_DEFINES})
list(APPEND POTHOS_SOURCES ${THERON_SOURCES})

########################################################################
# NUMA info support
########################################################################
find_library(
    NUMA_LIBRARIES
    NAMES numa
    PATHS /usr/lib /usr/lib64
)

if(NUMA_LIBRARIES)
    list(APPEND POTHOS_SOURCES System/NumaInfoLinux.cpp)
elseif(WIN32)
    list(APPEND POTHOS_SOURCES System/NumaInfoWindows.cpp)
else()
    list(APPEND POTHOS_SOURCES System/NumaInfoFail.cpp)
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
    System/Info.cpp
    System/NumaInfo.cpp
    System/Exception.cpp

    Object/Object.cpp
    Object/ObjectM.cpp
    Object/Hash.cpp
    Object/Compare.cpp
    Object/Convert.cpp
    Object/Allocator.cpp
    Object/Serialize.cpp
    Object/Exception.cpp

    Object/Builtin/Compare.cpp
    Object/Builtin/ConvertVecs.cpp
    Object/Builtin/ConvertNums.cpp
    Object/Builtin/ConvertString.cpp
    Object/Builtin/ConvertContainers.cpp
    Object/Builtin/Hash.cpp
    Object/Builtin/Serialize.cpp
    Object/Builtin/Tests.cpp

    Callable/Callable.cpp
    Callable/Exception.cpp
    Callable/Tests.cpp

    Framework/DType.cpp
    Framework/Label.cpp
    Framework/InputPort.cpp
    Framework/OutputPort.cpp
    Framework/CallRegistry.cpp
    Framework/SignalEmitter.cpp
    Framework/Connectable.cpp
    Framework/Block.cpp
    Framework/Topology.cpp
    Framework/WorkInfo.cpp
    Framework/WorkerActor.cpp
    Framework/WorkerActorPortAllocation.cpp
    Framework/WorkerActorHandlers.cpp
    Framework/WorkerStats.cpp
    Framework/SharedBuffer.cpp
    Framework/ManagedBuffer.cpp
    Framework/BufferChunk.cpp
    Framework/BufferManager.cpp
    Framework/BufferAccumulator.cpp
    Framework/BlockRegistry.cpp
    Framework/Exception.cpp

    Framework/Builtin/CircularBufferManager.cpp
    Framework/Builtin/TestBufferChunkSerialization.cpp
    Framework/Builtin/TestDType.cpp
    Framework/Builtin/TestSharedBuffer.cpp
    Framework/Builtin/GenericBufferManager.cpp
    Framework/Builtin/TestCircularBufferManager.cpp
    Framework/Builtin/TestGenericBufferManager.cpp
    Framework/Builtin/TestWorker.cpp
    Framework/Builtin/TestLabel.cpp

    Plugin/Path.cpp
    Plugin/Plugin.cpp
    Plugin/Registry.cpp
    Plugin/Module.cpp
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

    Remote/Node.cpp
    Remote/RemoteProxy.cpp
    Remote/RemoteProxyHandle.cpp
    Remote/Server.cpp
    Remote/ServerHandler.cpp
    Remote/Client.cpp
    Remote/Exception.cpp

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
    Util/EvalInterface.cpp
)

if(WIN32)
    list(APPEND POTHOS_SOURCES Framework/SharedBufferWindows.cpp)
elseif(UNIX)
    list(APPEND POTHOS_SOURCES Framework/SharedBufferUnix.cpp)
endif()

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
list(REMOVE_ITEM POTHOS_LIBRARIES Pothos) #cant link to self
target_link_libraries(Pothos ${POTHOS_LIBRARIES})
set_target_properties(Pothos PROPERTIES VERSION ${POTHOS_VERSION})
set_target_properties(Pothos PROPERTIES DEFINE_SYMBOL "POTHOS_DLL_EXPORTS")

install(TARGETS Pothos
    LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT pothos_runtime # .so file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT pothos_devel   # .lib file
    RUNTIME DESTINATION bin              COMPONENT pothos_runtime # .dll file
)

########################################################################
# Determine library name and reconfigure paths with that name
########################################################################
get_target_property(POTHOS_LIBRARY_LOCATION Pothos LOCATION_${CMAKE_BUILD_TYPE})
get_filename_component(POTHOS_LIBRARY_NAME ${POTHOS_LIBRARY_LOCATION} NAME)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/System/Paths.in.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/Paths.cpp
@ONLY)

########################################################################
# Build pkg config file
########################################################################
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Pothos.in.pc
    ${CMAKE_CURRENT_BINARY_DIR}/Pothos.pc
@ONLY)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/Pothos.pc
    DESTINATION lib${LIB_SUFFIX}/pkgconfig
    COMPONENT pothos_devel
)
