########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(Pothos)
enable_language(CXX)
enable_testing()

set(POTHOS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(POTHOS_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(POTHOS_MODULE_PATH ${POTHOS_SOURCE_DIR}/pothos-library/cmake/Modules)
set(CMAKE_MODULE_PATH ${POTHOS_MODULE_PATH})
include(PothosProjectDefaults)

########################################################################
# POCO dependency - always build it
########################################################################
set(BUILD_POCO TRUE) #always build -- want version 1.5

if(BUILD_POCO)
    #set top level dirs, go into submodule, and restore
    set(CMAKE_SOURCE_DIR ${POTHOS_SOURCE_DIR}/poco)
    set(CMAKE_BINARY_DIR ${POTHOS_BINARY_DIR}/poco)
    unset(CMAKE_MODULE_PATH)
    add_subdirectory(poco)
    set(CMAKE_SOURCE_DIR ${POTHOS_SOURCE_DIR})
    set(CMAKE_BINARY_DIR ${POTHOS_BINARY_DIR})
    set(CMAKE_MODULE_PATH ${POTHOS_MODULE_PATH})

    #set FindPoco variables for submodule build
    set(POCO_FOUND TRUE)

    set(POCO_INCLUDE_DIR ${POTHOS_SOURCE_DIR}/poco/Foundation/include)
    set(POCO_LIBRARY_REL PocoFoundation)

    set(POCO_Util_INCLUDE_DIR ${POTHOS_SOURCE_DIR}/poco/Util/include)
    set(POCO_Util_LIBRARY_REL PocoUtil)

    set(POCO_XML_INCLUDE_DIR ${POTHOS_SOURCE_DIR}/poco/XML/include)
    set(POCO_XML_LIBRARY_REL PocoXML)

    set(POCO_Net_INCLUDE_DIR ${POTHOS_SOURCE_DIR}/poco/Net/include)
    set(POCO_Net_LIBRARY_REL PocoNet)

    set(POCO_Data_INCLUDE_DIR ${POTHOS_SOURCE_DIR}/poco/Data/include)
    set(POCO_Data_LIBRARY_REL PocoData)

    set(POCO_DataSQLite_INCLUDE_DIR ${POTHOS_SOURCE_DIR}/poco/Data/SQLite/include)
    set(POCO_DataSQLite_LIBRARY_REL PocoDataSQLite)

    set(POCO_MongoDB_INCLUDE_DIR ${POTHOS_SOURCE_DIR}/poco/MongoDB/include)
    set(POCO_MongoDB_LIBRARY_REL PocoMongoDB)

    set(POCO_JSON_INCLUDE_DIR ${POTHOS_SOURCE_DIR}/poco/JSON/include)
    set(POCO_JSON_LIBRARY_REL PocoJSON)

else(BUILD_POCO)
    find_package(Poco)
    if(NOT POCO_FOUND)
        message(FATAL_ERROR "Cannot find POCO dependency - get it from your package manager")
    endif()

endif(BUILD_POCO)

list(APPEND POCO_INCLUDE_DIRS
    ${POCO_INCLUDE_DIR}
    ${POCO_Util_INCLUDE_DIR}
    ${POCO_XML_INCLUDE_DIR}
    ${POCO_Net_INCLUDE_DIR}
    ${POCO_Data_INCLUDE_DIR}
    ${POCO_DataSQLite_INCLUDE_DIR}
    ${POCO_MongoDB_INCLUDE_DIR}
    ${POCO_JSON_INCLUDE_DIR}
)

list(APPEND POCO_LIBRARIES
    ${POCO_LIBRARY_REL}
    ${POCO_Util_LIBRARY_REL}
    ${POCO_XML_LIBRARY_REL}
    ${POCO_Net_LIBRARY_REL}
    ${POCO_Data_LIBRARY_REL}
    ${POCO_DataSQLite_LIBRARY_REL}
    ${POCO_MongoDB_LIBRARY_REL}
    ${POCO_JSON_LIBRARY_REL}
)

message(STATUS "POCO_INCLUDE_DIRS: ${POCO_INCLUDE_DIRS}")
message(STATUS "POCO_LIBRARIES: ${POCO_LIBRARIES}")

########################################################################
# Sigar dependency - always build it
########################################################################
#set top level dirs, go into submodule, and restore
set(CMAKE_SOURCE_DIR ${POTHOS_SOURCE_DIR}/sigar)
set(CMAKE_BINARY_DIR ${POTHOS_BINARY_DIR}/sigar)
unset(CMAKE_MODULE_PATH)
add_subdirectory(sigar)
set(CMAKE_SOURCE_DIR ${POTHOS_SOURCE_DIR})
set(CMAKE_BINARY_DIR ${POTHOS_BINARY_DIR})
set(CMAKE_MODULE_PATH ${POTHOS_MODULE_PATH})

#set FindSigar variables for submodule build
set(SIGAR_FOUND TRUE)
set(SIGAR_INCLUDE_DIRS ${POTHOS_SOURCE_DIR}/sigar/include)
set(SIGAR_LIBRARIES sigar)

message(STATUS "SIGAR_INCLUDE_DIRS: ${SIGAR_INCLUDE_DIRS}")
message(STATUS "SIGAR_LIBRARIES: ${SIGAR_LIBRARIES}")

########################################################################
# Build serialization library
########################################################################
add_subdirectory(pothos-serialization)

########################################################################
# Build main pothos library
########################################################################
list(APPEND POTHOS_LIBRARIES
    Pothos
    PothosSerialization
    ${POCO_LIBRARIES}
)

list(APPEND POTHOS_INCLUDE_DIRS
    ${POTHOS_SOURCE_DIR}/pothos-library/include
    ${POTHOS_SOURCE_DIR}/pothos-serialization/include
    ${POCO_INCLUDE_DIRS}
)

add_subdirectory(pothos-library)

#########################################################################
## Modules use PothosUtil and FindPothos --
## lets trick it into working in a pre-installed environment
#########################################################################
set(INCLUDED_FIND_POTHOS_CMAKE TRUE) #avoids FindPothos

get_target_property(POTHOS_UTIL_EXE PothosUtil LOCATION_${CMAKE_BUILD_TYPE})

if(MSVC)
    unset(built_dll_paths)
    foreach(target sigar ${POTHOS_LIBRARIES})
        get_target_property(library_location ${target} LOCATION_${CMAKE_BUILD_TYPE})
        get_filename_component(library_location ${library_location} PATH)
        file(TO_NATIVE_PATH ${library_location} library_location)
        set(built_dll_paths "${built_dll_paths}\\;${library_location}")
    endforeach(target)
    file(WRITE ${POTHOS_BINARY_DIR}/PothosUtil.bat
        "set PATH=%PATH%${built_dll_paths}\n"
        "${POTHOS_UTIL_EXE} %*\n"
    )
    set(POTHOS_UTIL_EXE ${POTHOS_BINARY_DIR}/PothosUtil.bat)
endif(MSVC)

add_subdirectory(pothos-gui)
add_subdirectory(pothos-blocks)
add_subdirectory(pothos-python)
add_subdirectory(pothos-java)
add_subdirectory(pothos-util)
add_subdirectory(pothos-opencl)
