if(DEFINED INCLUDED_POTHOS_PYTHON_UTIL_CMAKE)
    return()
endif()
set(INCLUDED_POTHOS_PYTHON_UTIL_CMAKE TRUE)

include(${POTHOS_MODULE_PATH}/PothosUtil.cmake)

########################################################################
## POTHOS_PYTHON_UTIL - build and install python modules for Pothos
##
## TARGET - the name of the python module to build
##
## All other arguments are passed directly to the POTHOS_MODULE_UTIL()
## See documentation for POTHOS_MODULE_UTIL() in PothosUtil.cmake
########################################################################
function(POTHOS_PYTHON_UTIL)

    include(CMakeParseArguments)
    CMAKE_PARSE_ARGUMENTS(POTHOS_MODULE_UTIL "ENABLE_DOCS" "TARGET;DESTINATION" "SOURCES;LIBRARIES;DOC_SOURCES" ${ARGN})

    #generate block factories

    POTHOS_MODULE_UTIL(${ARGN})

endfunction(POTHOS_PYTHON_UTIL)
