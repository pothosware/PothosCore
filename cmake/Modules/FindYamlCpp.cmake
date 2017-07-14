# - Try to find yaml-cpp
# Once done this will define
#  YAMLCPP_FOUND - System has yaml-cpp
#  YAMLCPP_VERSION - Version of yaml-cpp
#  YAMLCPP_INCLUDE_DIRS - The yaml-cpp include directories
#  YAMLCPP_LIBRARIES - The libraries needed to use yaml-cpp

find_package(PkgConfig)
pkg_check_modules(PC_YAMLCPP QUIET yaml-cpp)

if (PC_YAMLCPP_VERSION)
    set(YAMLCPP_VERSION ${PC_YAMLCPP_VERSION})
endif (PC_YAMLCPP_VERSION)

find_path(YAMLCPP_INCLUDE_DIR
    NAMES yaml-cpp/yaml.h
    HINTS
        $ENV{YAMLCPP_DIR}/include
        ${PC_YAMLCPP_INCLUDEDIR}
        ${PC_YAMLCPP_INCLUDE_DIRS}
)

find_library(YAMLCPP_LIBRARY
    NAMES yaml-cpp
    HINTS
        $ENV{YAMLCPP_DIR}/lib
        ${PC_YAMLCPP_LIBDIR}
        ${PC_YAMLCPP_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(YAMLCPP DEFAULT_MSG YAMLCPP_LIBRARY YAMLCPP_INCLUDE_DIR)

mark_as_advanced(YAMLCPP_INCLUDE_DIR YAMLCPP_LIBRARY)

set(YAMLCPP_INCLUDE_DIRS ${YAMLCPP_INCLUDE_DIR})
set(YAMLCPP_LIBRARIES ${YAMLCPP_LIBRARY})
