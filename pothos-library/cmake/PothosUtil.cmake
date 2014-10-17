if(DEFINED INCLUDED_POTHOS_UTIL_CMAKE)
    return()
endif()
set(INCLUDED_POTHOS_UTIL_CMAKE TRUE)

########################################################################
## POTHOS_MODULE_UTIL - build and install modules for Pothos
##
## This utility can handle the build and installation operations
## for most Pothos user projects. Otherwise, please copy and modify.
##
## Arguments:
##
## TARGET - the name of the module to build
## The target name should be unique per project.
## If this target shares a destination with other modules,
## Then the target should also be unique among those modules.
##
## SOURCES - a list of c++ source files
## This list will primarily consist of c++ source files and not headers.
## However, if header files contain documentation markup for the GUI,
## then these header files should be included for markup parsing.
## See ENABLE_DOCS which is required to enable SOURCES for scanning.
##
## LIBRARIES - a list of libraries to link the module to
## The module will automatically link to Pothos and Poco libraries.
## This argument should be used to specify additional libraries.
##
## DESTINATION - relative destination path
## The destination path is relative to the module directory.
## Try to use destination paths that reflect the plugin registry path.
## Example: Suppose you are building a module for /blocks/foo/bar,
## Then the destination should be blocks/foo and the TARGET bar.
## However, this is simply a recomendation.
##
## DOC_SOURCES - an alternative list of sources to scan for docs
## This allows the user to explicitly specity the list of sources
## that are used exclusively for documentation markup for the GUI.
##
## ENABLE_DOCS - enable scanning of SOURCES for documentation markup.
## Pass this flag to the util function to enable scanning of SOURCES.
## This is required to scan SOURCES but not when DOC_SOURCES are used.
##
########################################################################
function(POTHOS_MODULE_UTIL)

    include(CMakeParseArguments)
    CMAKE_PARSE_ARGUMENTS(POTHOS_MODULE_UTIL "ENABLE_DOCS" "TARGET;DESTINATION" "SOURCES;LIBRARIES;DOC_SOURCES" ${ARGN})

    #always enable docs if user specifies doc sources
    if (POTHOS_MODULE_UTIL_DOC_SOURCES)
        set(POTHOS_MODULE_UTIL_ENABLE_DOCS TRUE)
    #otherwise doc sources come from the regular sources
    else()
        set(POTHOS_MODULE_UTIL_DOC_SOURCES ${POTHOS_MODULE_UTIL_SOURCES})
    endif()

    #setup json doc file generation and install
    if (POTHOS_MODULE_UTIL_ENABLE_DOCS)

        #turn sources into an absolute path
        unset(__POTHOS_SOURCES)
        foreach(source ${POTHOS_MODULE_UTIL_DOC_SOURCES})
            if (EXISTS ${source})
                list(APPEND __POTHOS_SOURCES ${source})
            else()
                list(APPEND __POTHOS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${source})
            endif()
        endforeach(source)
        set(POTHOS_MODULE_UTIL_DOC_SOURCES ${__POTHOS_SOURCES})

        set(cpp_doc_file ${CMAKE_CURRENT_BINARY_DIR}/${POTHOS_MODULE_UTIL_TARGET}Docs.cpp)
        add_custom_command(
            OUTPUT ${cpp_doc_file}
            COMMAND ${POTHOS_UTIL_EXE}
                --doc-parse ${POTHOS_MODULE_UTIL_DOC_SOURCES}
                --output ${cpp_doc_file}
            DEPENDS ${POTHOS_MODULE_UTIL_DOC_SOURCES}
            DEPENDS ${POTHOS_UTIL_EXE}
        )
        list(APPEND POTHOS_MODULE_UTIL_SOURCES ${cpp_doc_file})
    endif()

    #setup module build and install rules
    include_directories(${Pothos_INCLUDE_DIRS})
    add_library(${POTHOS_MODULE_UTIL_TARGET} MODULE ${POTHOS_MODULE_UTIL_SOURCES})
    target_link_libraries(${POTHOS_MODULE_UTIL_TARGET} ${Pothos_LIBRARIES} ${POTHOS_MODULE_UTIL_LIBRARIES})
    set_target_properties(${POTHOS_MODULE_UTIL_TARGET} PROPERTIES DEBUG_POSTFIX "") #same name in debug mode
    install(
        TARGETS ${POTHOS_MODULE_UTIL_TARGET}
        DESTINATION lib${LIB_SUFFIX}/Pothos/modules/${POTHOS_MODULE_UTIL_DESTINATION}
    )

endfunction(POTHOS_MODULE_UTIL)
