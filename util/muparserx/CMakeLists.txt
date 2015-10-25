########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.0)
project(muparserx CXX)

########################################################################
# Extract version
########################################################################
set(MUPARSERX_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/parser)
file(READ "${MUPARSERX_SOURCE_DIR}/mpDefines.h" mpDefines_h)
string(REGEX MATCH "\\#define MUP_PARSER_VERSION _T\\(\"([0-9]+\\.[0-9]+\\.[0-9]+) \\(" MUPARSERX_VERSION_MATCHES "${mpDefines_h}")
if(NOT MUPARSERX_VERSION_MATCHES)
    message(FATAL_ERROR "Failed to extract version number from mpDefines.h")
endif(NOT MUPARSERX_VERSION_MATCHES)
set(MUPARSERX_VERSION ${CMAKE_MATCH_1})

########################################################################
# Compiler specific flags
########################################################################
if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-pedantic)
    add_compile_options(-Wall)
    add_compile_options(-Wextra)
endif(CMAKE_COMPILER_IS_GNUCXX)

if(MSVC)
    add_compile_options(/MP) #multi-core build
endif(MSVC)

########################################################################
# Build library
# Defaults to static, set BUILD_SHARED_LIBS=ON for shared
########################################################################
file(GLOB MUPARSERX_SOURCES "${MUPARSERX_SOURCE_DIR}/*.cpp")
include_directories(${MUPARSERX_SOURCE_DIR})
add_library(muparserx ${MUPARSERX_SOURCES})
set_target_properties(muparserx PROPERTIES VERSION ${MUPARSERX_VERSION})

#link with lib math when found
find_library(
    M_LIBRARY NAMES m
    PATHS /usr/lib /usr/lib64
)
if(M_LIBRARY)
    target_link_libraries(muparserx ${M_LIBRARY})
endif(M_LIBRARY)

install(TARGETS muparserx
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)

########################################################################
# Build pkg config file
########################################################################
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/muparserx.in.pc
    ${CMAKE_CURRENT_BINARY_DIR}/muparserx.pc
@ONLY)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/muparserx.pc
    DESTINATION lib${LIB_SUFFIX}/pkgconfig
)

########################################################################
# Install headers
########################################################################
file(GLOB MUPARSERX_HEADERS "${MUPARSERX_SOURCE_DIR}/*.h")
install(
    FILES ${MUPARSERX_HEADERS}
    DESTINATION include
)

########################################################################
# Example application
########################################################################
option(BUILD_EXAMPLES "enable building example applications" ON)
if(BUILD_EXAMPLES)
    add_executable(example sample/example.cpp sample/timer.cpp)
    target_link_libraries(example muparserx)
endif(BUILD_EXAMPLES)

########################################################################
# Print summary
########################################################################
message(STATUS "Building muparserx version: ${MUPARSERX_VERSION}")
message(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")
