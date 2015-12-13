########################################################################
# Optional doxygen generation
########################################################################
find_package(Doxygen)

if (DOXYGEN_DOT_FOUND)
    set(HAVE_DOT YES)
else()
    set(HAVE_DOT NO)
endif()

########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_LIBRARY_DOCS "Enable Pothos Library.Docs component" ON "ENABLE_LIBRARY;DOXYGEN_FOUND" OFF)
add_feature_info("  Docs" ENABLE_LIBRARY_DOCS "Doxygen generated docs for Pothos library")
if (NOT ENABLE_LIBRARY_DOCS)
    return()
endif()

#header sources used in the generate rule
file(GLOB_RECURSE POTHOS_INCLUDES ${PROJECT_SOURCE_DIR}/include/*.hpp)
#TODO remove *.tmpl.* files from this list

#formated list of sources to parse
string(REPLACE ";" " " DOXYGEN_SOURCES "${POTHOS_INCLUDES}")

#Configure doxygen config file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
@ONLY)

#Create doxygen generation build rules
if (DOXYGEN_FOUND)
    set(CMAKE_CURRENT_BINARY_DIR_DOXYGEN ${CMAKE_CURRENT_BINARY_DIR}/html)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR_DOXYGEN}
        DEPENDS ${POTHOS_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        COMMENT "Generating documentation with doxygen"
    )
    add_custom_target(docs ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR_DOXYGEN})
endif()
