if(DEFINED INCLUDED_POTHOS_CONFIG_CMAKE)
    return()
endif()
set(INCLUDED_POTHOS_CONFIG_CMAKE TRUE)

########################################################################
# PothosConfig - sets Pothos installation variables
#
# POTHOS_UTIL_EXE - the path to PothosUtil executable
# Pothos_INCLUDE_DIRS - Pothos development includes
# Pothos_LIBRARIES - Pothos development libraries
# POTHOS_CMAKE_DIRECTORY - where to install CMake files
########################################################################
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_LIST_DIR})
include(PothosStandardFlags) #compiler settings
include(PothosUtil) #utility functions

########################################################################
# install directory for cmake files
########################################################################
if (UNIX)
    set(POTHOS_CMAKE_DIRECTORY share/cmake/Pothos)
elseif (WIN32)
    set(POTHOS_CMAKE_DIRECTORY cmake)
endif ()

########################################################################
# select the release build type by default to get optimization flags
########################################################################
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

########################################################################
# Automatic LIB_SUFFIX detection + configuration option
########################################################################
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
endif()

if(LINUX AND EXISTS "/etc/debian_version")
    set(DEBIAN TRUE)
endif()

if(LINUX AND EXISTS "/etc/redhat-release")
    set(REDHAT TRUE)
endif()

if(LINUX AND EXISTS "/etc/slackware-version")
    set(SLACKWARE TRUE)
endif()

if(NOT DEFINED LIB_SUFFIX AND (REDHAT OR SLACKWARE) AND CMAKE_SYSTEM_PROCESSOR MATCHES "64$")
    SET(LIB_SUFFIX 64)
endif()
set(LIB_SUFFIX ${LIB_SUFFIX} CACHE STRING "lib directory suffix")

########################################################################
## it-tree build support
########################################################################
if (POTHOS_IN_TREE_SOURCE_DIR)

    list(APPEND Pothos_LIBRARIES
        Pothos
        PothosSerialization
        ${POCO_LIBRARIES}
    )

    list(APPEND Pothos_INCLUDE_DIRS
        ${POTHOS_IN_TREE_SOURCE_DIR}/pothos-library/include
        ${POTHOS_IN_TREE_SOURCE_DIR}/pothos-serialization/include
        ${POCO_INCLUDE_DIRS}
    )

    get_target_property(POTHOS_UTIL_EXE PothosUtil LOCATION_${CMAKE_BUILD_TYPE})

    if(MSVC AND POTHOS_UTIL_EXE)
        set(built_dll_paths "%PATH%")
        foreach(target ${Pothos_LIBRARIES})
            get_target_property(library_location ${target} LOCATION_${CMAKE_BUILD_TYPE})
            get_filename_component(library_location ${library_location} PATH)
            file(TO_NATIVE_PATH ${library_location} library_location)
            set(built_dll_paths "${library_location};${built_dll_paths}")
        endforeach(target)
        file(WRITE ${PROJECT_BINARY_DIR}/PothosUtil.bat
            "set PATH=${built_dll_paths}\n"
            "${POTHOS_UTIL_EXE} %*\n"
        )
        set(POTHOS_UTIL_EXE ${PROJECT_BINARY_DIR}/PothosUtil.bat)
    endif()

    return()
endif ()

########################################################################
## Determine root installation path
########################################################################
if (UNIX)
    get_filename_component(POTHOS_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
elseif (WIN32)
    get_filename_component(POTHOS_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif ()

########################################################################
## locate the PothosUtil application
########################################################################
find_program(
    POTHOS_UTIL_EXE PothosUtil
    PATHS ${POTHOS_ROOT}/bin
    NO_DEFAULT_PATH
)
if(NOT POTHOS_UTIL_EXE)
    message(FATAL_ERROR "cannot find PothosUtil in ${POTHOS_ROOT}/bin")
endif()

set(__success_code "200")
execute_process(
    COMMAND ${POTHOS_UTIL_EXE} --success-code ${__success_code}
    RESULT_VARIABLE __result_code
)
if(NOT __result_code STREQUAL __success_code)
    message(FATAL_ERROR "PothosUtil execution test failed")
endif()

########################################################################
## locate the Pothos library
########################################################################
find_library(
    POTHOS_LIBRARY Pothos Pothosd
    PATHS ${POTHOS_ROOT}/lib${LIB_SUFFIX}
    NO_DEFAULT_PATH
)
if(NOT POTHOS_LIBRARY)
    message(FATAL_ERROR "cannot find Pothos library in ${POTHOS_ROOT}/lib${LIB_SUFFIX}")
endif()
list(APPEND Pothos_LIBRARIES ${POTHOS_LIBRARY})

########################################################################
## locate the Pothos Serialization library
########################################################################
find_library(
    POTHOS_SERIALIZATION_LIBRARY PothosSerialization PothosSerializationd
    PATHS ${POTHOS_ROOT}/lib${LIB_SUFFIX}
    NO_DEFAULT_PATH
)
if(NOT POTHOS_LIBRARY)
    message(FATAL_ERROR "cannot find PothosSerialization library in ${POTHOS_ROOT}/lib${LIB_SUFFIX}")
endif()
list(APPEND Pothos_LIBRARIES ${POTHOS_SERIALIZATION_LIBRARY})

########################################################################
## locate the Pothos includes
########################################################################
find_path(
    POTHOS_INCLUDE_DIR Pothos/Config.hpp
    PATHS ${POTHOS_ROOT}/include
    NO_DEFAULT_PATH
)
if(NOT POTHOS_LIBRARY)
    message(FATAL_ERROR "cannot find Pothos includes in ${POTHOS_ROOT}/include")
endif()
list(APPEND Pothos_INCLUDE_DIRS ${POTHOS_INCLUDE_DIR})

########################################################################
## locate the Poco libraries
########################################################################
foreach(lib Foundation Data JSON XML Util Zip Net PDF MongoDB DataSQLite)
    find_library(
        POCO_LIBRARY_${lib} Poco${lib} Poco${lib}d
        PATHS ${POTHOS_ROOT}/lib${LIB_SUFFIX}
        NO_DEFAULT_PATH
    )
    if(NOT POCO_LIBRARY_${lib})
        message(FATAL_ERROR "cannot find POCO_LIBRARY_${lib} library in ${POTHOS_ROOT}/lib${LIB_SUFFIX}")
    endif()
    list(APPEND Pothos_LIBRARIES ${POCO_LIBRARY_${lib}})
endforeach(lib)
