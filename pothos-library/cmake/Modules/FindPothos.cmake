if(DEFINED INCLUDED_FIND_POTHOS_CMAKE)
    return()
endif()
set(INCLUDED_FIND_POTHOS_CMAKE TRUE)

########################################################################
## Configure installation variables
########################################################################
include(PothosProjectDefaults) #defines LIB_SUFFIX
set(POTHOS_ROOT ${CMAKE_INSTALL_PREFIX}) #same as install prefix

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
list(APPEND POTHOS_LIBRARIES ${POTHOS_LIBRARY})

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
list(APPEND POTHOS_LIBRARIES ${POTHOS_SERIALIZATION_LIBRARY})

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
list(APPEND POTHOS_INCLUDE_DIRS ${POTHOS_INCLUDE_DIR})

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
    list(APPEND POTHOS_LIBRARIES ${POCO_LIBRARY_${lib}})
endforeach(lib)
