if(DEFINED INCLUDED_POTHOS_LIBRARY_CONFIG_CMAKE)
    return()
endif()
set(INCLUDED_POTHOS_LIBRARY_CONFIG_CMAKE TRUE)

########################################################################
# Automatic LIB_SUFFIX detection + configuration option
########################################################################
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
endif()

if(LINUX AND EXISTS "/etc/debian_version")
    set(DEBIAN TRUE)
endif()

if(LINUX AND EXISTS "/etc/redhat-release")
    set(REDHAT TRUE)
endif()

if(LINUX AND EXISTS "/etc/slackware-version")
    set(SLACKWARE TRUE)
endif()

if(NOT DEFINED LIB_SUFFIX AND (REDHAT OR SLACKWARE) AND CMAKE_SYSTEM_PROCESSOR MATCHES "64$")
    SET(LIB_SUFFIX 64)
endif()
set(LIB_SUFFIX ${LIB_SUFFIX} CACHE STRING "lib directory suffix")

########################################################################
# rpath setup - http://www.cmake.org/Wiki/CMake_RPATH_handling
########################################################################
# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
