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
include(PothosLibraryConfig) #library settings
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
## in-tree build support
########################################################################
if (POTHOS_IN_TREE_SOURCE_DIR)

    set(POTHOS_ROOT ${CMAKE_INSTALL_PREFIX})

    list(APPEND Pothos_LIBRARIES
        Pothos
        PothosSerialization
        ${Poco_LIBRARIES}
    )

    list(APPEND Pothos_INCLUDE_DIRS
        ${POTHOS_IN_TREE_SOURCE_DIR}/pothos-library/include
        ${POTHOS_IN_TREE_SOURCE_DIR}/pothos-serialization/include
        ${Poco_INCLUDE_DIRS}
    )

    #a list of in-tree built libraries to generate a library path script
    set(IN_TREE_LIBRARIES Pothos PothosSerialization)
    if (POCO_IN_TREE)
        list(APPEND IN_TREE_LIBRARIES ${Poco_LIBRARIES})
    endif()

    if(MSVC)
        set(built_dll_paths "%PATH%")
        foreach(target ${IN_TREE_LIBRARIES})
            set(built_dll_paths "$<TARGET_FILE_DIR:${target}>;${built_dll_paths}")
        endforeach(target)
        set(POTHOS_UTIL_EXE ${PROJECT_BINARY_DIR}/PothosUtil.bat)
        add_custom_command(
            OUTPUT ${POTHOS_UTIL_EXE}
            DEPENDS PothosUtil
            COMMAND ${CMAKE_COMMAND} -E echo "set PATH=${built_dll_paths}" > ${POTHOS_UTIL_EXE}
            COMMAND ${CMAKE_COMMAND} -E echo "\"$<TARGET_FILE:PothosUtil>\" %*" >> ${POTHOS_UTIL_EXE}
            VERBATIM
        )
    endif()

    if(UNIX)
        set(built_dll_paths "\${LD_LIBRARY_PATH}")
        foreach(target ${IN_TREE_LIBRARIES})
            set(built_dll_paths "$<TARGET_FILE_DIR:${target}>:${built_dll_paths}")
        endforeach(target)
        find_program(SH_EXE sh)
        find_program(CHMOD_EXE chmod)
        set(POTHOS_UTIL_EXE ${PROJECT_BINARY_DIR}/PothosUtil.sh)
        add_custom_command(
            OUTPUT ${POTHOS_UTIL_EXE}
            DEPENDS PothosUtil
            COMMAND ${CMAKE_COMMAND} -E echo "#!${SH_EXE}" > ${POTHOS_UTIL_EXE}
            COMMAND ${CMAKE_COMMAND} -E echo "export LD_LIBRARY_PATH=${built_dll_paths}" >> ${POTHOS_UTIL_EXE}
            COMMAND ${CMAKE_COMMAND} -E echo "export DYLD_LIBRARY_PATH=\${LD_LIBRARY_PATH}:\${DYLD_LIBRARY_PATH}" >> ${POTHOS_UTIL_EXE}
            COMMAND ${CMAKE_COMMAND} -E echo "\"$<TARGET_FILE:PothosUtil>\" $@" >> ${POTHOS_UTIL_EXE}
            COMMAND ${CHMOD_EXE} +x ${POTHOS_UTIL_EXE}
            VERBATIM
        )
    endif()

    set(__POTHOS_UTIL_TARGET_NAME ${PROJECT_NAME}PothosUtil)
    add_custom_target(${__POTHOS_UTIL_TARGET_NAME} DEPENDS ${POTHOS_UTIL_EXE})

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

if(MSVC)
    file(TO_NATIVE_PATH "${POTHOS_ROOT}/bin" bin_path)
    file(TO_NATIVE_PATH "${POTHOS_UTIL_EXE}" POTHOS_UTIL_EXE)
    file(WRITE ${PROJECT_BINARY_DIR}/PothosUtil.bat
        "set PATH=${bin_path};%PATH%\n"
        "\"${POTHOS_UTIL_EXE}\" %*\n"
    )
    set(POTHOS_UTIL_EXE ${PROJECT_BINARY_DIR}/PothosUtil.bat)
endif()

if(UNIX)
    file(TO_NATIVE_PATH "${POTHOS_ROOT}/lib${LIB_SUFFIX}" lib_path)
    file(TO_NATIVE_PATH "${POTHOS_UTIL_EXE}" POTHOS_UTIL_EXE)
    find_program(SH_EXE sh)
    find_program(CHMOD_EXE chmod)
    file(WRITE ${PROJECT_BINARY_DIR}/PothosUtil.sh
        "#!${SH_EXE}\n"
        "export LD_LIBRARY_PATH=${lib_path}:\${LD_LIBRARY_PATH}\n"
        "export DYLD_LIBRARY_PATH=\${LD_LIBRARY_PATH}:\${DYLD_LIBRARY_PATH}\n"
        "\"${POTHOS_UTIL_EXE}\" $@\n"
    )
    execute_process(COMMAND ${CHMOD_EXE} +x ${PROJECT_BINARY_DIR}/PothosUtil.sh)
    set(POTHOS_UTIL_EXE ${PROJECT_BINARY_DIR}/PothosUtil.sh)
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
    PATH_SUFFIXES ${CMAKE_LIBRARY_ARCHITECTURE}
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
include(SetupPoco) #find poco install

#try to use poco from the system install
if (Poco_FOUND)
    list(APPEND Pothos_LIBRARIES ${Poco_LIBRARIES})

#otherwise use poco from the pothos install
else (Poco_FOUND)
    foreach(lib Foundation JSON XML Util Net)
        find_library(
            POCO_${lib}_LIBRARY Poco${lib} Poco${lib}d
            PATHS ${POTHOS_ROOT}/lib${LIB_SUFFIX}
            NO_DEFAULT_PATH
        )
        if(NOT POCO_${lib}_LIBRARY)
            message(FATAL_ERROR "cannot find POCO_${lib}_LIBRARY library in ${POTHOS_ROOT}/lib${LIB_SUFFIX}")
        endif()
        list(APPEND Pothos_LIBRARIES ${POCO_${lib}_LIBRARY})
    endforeach(lib)
endif (Poco_FOUND)
