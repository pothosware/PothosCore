if(DEFINED INCLUDED_POTHOS_PYTHON_UTIL_CMAKE)
    return()
endif()
set(INCLUDED_POTHOS_PYTHON_UTIL_CMAKE TRUE)

include(${POTHOS_MODULE_PATH}/PothosUtil.cmake)

########################################################################
## POTHOS_PYTHON_UTIL - build and install python modules for Pothos
##
## TARGET - the name of the python module to build
## SOURCES - 
## DESTINATION - 
## DOC_SOURCES - 
## ENABLE_DOCS - 
##
## All other arguments are passed directly to the POTHOS_MODULE_UTIL()
## See documentation for POTHOS_MODULE_UTIL() in PothosUtil.cmake
########################################################################
function(POTHOS_PYTHON_UTIL)

    include(CMakeParseArguments)
    CMAKE_PARSE_ARGUMENTS(POTHOS_PYTHON_UTIL "ENABLE_DOCS" "TARGET;DESTINATION" "SOURCES;DOC_SOURCES" ${ARGN})
    
    #install python sources
    if (POTHOS_PYTHON_UTIL_SOURCES)
        install(
            FILES ${${POTHOS_PYTHON_UTIL_SOURCES}}
            DESTINATION ${POTHOS_PYTHON_UTIL_DESTINATION}
        )
    endif()

    #build the module
    if (POTHOS_PYTHON_UTIL_ENABLE_DOCS)
        set(POTHOS_PYTHON_UTIL_ENABLE_DOCS "ENABLE_DOCS")
    else()
        unset(POTHOS_PYTHON_UTIL_ENABLE_DOCS)
    endif()
    POTHOS_MODULE_UTIL(
        TARGET ${POTHOS_PYTHON_UTIL_TARGET}
        DESTINATION ${POTHOS_PYTHON_UTIL_DESTINATION}
        SOURCES ${POTHOS_PYTHON_UTIL_SOURCES}
        DOC_SOURCES ${POTHOS_PYTHON_UTIL_DOC_SOURCES}
        ${POTHOS_PYTHON_UTIL_ENABLE_DOCS}
    )

endfunction(POTHOS_PYTHON_UTIL)
