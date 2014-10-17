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
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(PothosProjectDefaults) #defines LIB_SUFFIX
include(PothosUtil) #utility functions

if (UNIX)
    set(POTHOS_CMAKE_DIRECTORY share/cmake/Pothos)
elseif (WIN32)
    set(POTHOS_CMAKE_DIRECTORY cmake)
endif ()

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

    if(MSVC)
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
    endif(MSVC)

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
