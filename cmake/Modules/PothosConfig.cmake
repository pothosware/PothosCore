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
include(PothosUtil) #utility functions

########################################################################
# select the release build type by default to get optimization flags
########################################################################
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

########################################################################
# Use GNU Install Dirs, but support LIB_SUFFIX if specified
########################################################################
include(GNUInstallDirs)
if(LIB_SUFFIX)
    set(CMAKE_INSTALL_LIBDIR "lib${LIB_SUFFIX}") #support old lib suffix
endif(LIB_SUFFIX)

########################################################################
# extract the ABI version string from the Version.hpp header
########################################################################
function(_POTHOS_GET_ABI_VERSION VERSION INCLUDE_DIR)
    file(READ "${INCLUDE_DIR}/Pothos/System/Version.hpp" version_hpp)
    string(REGEX MATCH "\\#define POTHOS_ABI_VERSION \"([0-9]+\\.[0-9]+(-[A-Za-z0-9]+)?)\"" POTHOS_ABI_VERSION_MATCHES "${version_hpp}")
    if(NOT POTHOS_ABI_VERSION_MATCHES)
        message(FATAL_ERROR "Failed to extract version number from Version.hpp")
    endif(NOT POTHOS_ABI_VERSION_MATCHES)
    set(${VERSION} "${CMAKE_MATCH_1}" CACHE INTERNAL "")
endfunction(_POTHOS_GET_ABI_VERSION)

########################################################################
# install directory for cmake files
# used publicly in PothosPython
########################################################################
if (UNIX)
    set(POTHOS_CMAKE_DIRECTORY ${CMAKE_INSTALL_DATADIR}/cmake/Pothos)
elseif (WIN32)
    set(POTHOS_CMAKE_DIRECTORY cmake)
endif ()

########################################################################
## in-tree build support
########################################################################
if (POTHOS_IN_TREE_SOURCE_DIR)

    if(NOT POTHOS_ROOT)
        set(POTHOS_ROOT ${CMAKE_INSTALL_PREFIX})
    endif(NOT POTHOS_ROOT)

    _POTHOS_GET_ABI_VERSION(POTHOS_ABI_VERSION ${POTHOS_IN_TREE_SOURCE_DIR}/include)

    #a list of in-tree built libraries to generate a library path script
    set(IN_TREE_LIBRARIES Pothos)
    if (POCO_IN_TREE)
        list(APPEND IN_TREE_LIBRARIES ${Poco_LIBRARIES})
    endif()
    if (MUPARSERX_IN_TREE)
        list(APPEND IN_TREE_LIBRARIES ${muparserx_LIBRARIES})
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

    #set old-style variables
    unset(Pothos_INCLUDE_DIRS)
    set(Pothos_LIBRARIES Pothos)
    return()
endif ()

########################################################################
## Determine root installation path
########################################################################
file(RELATIVE_PATH POTHOS_ROOT_RELDIR "/${POTHOS_CMAKE_DIRECTORY}" "/")
get_filename_component(POTHOS_ROOT "${CMAKE_CURRENT_LIST_DIR}/${POTHOS_ROOT_RELDIR}" ABSOLUTE)

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
    file(TO_NATIVE_PATH "${POTHOS_ROOT}/${CMAKE_INSTALL_LIBDIR}" lib_path)
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
## create import library target
########################################################################
enable_language(C) #newer Poco config scrips require c to be enabled
include(SetupPoco) #Poco is a Pothos library public dependency

include(PothosExport)

#set old-style variables
get_target_property(POTHOS_INCLUDE_DIR Pothos INTERFACE_INCLUDE_DIRECTORIES)
set(Pothos_INCLUDE_DIRS ${POTHOS_INCLUDE_DIR})
set(Pothos_LIBRARIES Pothos)
_POTHOS_GET_ABI_VERSION(POTHOS_ABI_VERSION ${POTHOS_INCLUDE_DIR})
