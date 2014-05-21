if(DEFINED INCLUDED_POTHOS_PROJECT_DEFAULTS_CMAKE)
    return()
endif()
set(INCLUDED_POTHOS_PROJECT_DEFAULTS_CMAKE TRUE)

########################################################################
# select the release build type by default to get optimization flags
########################################################################
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

########################################################################
# Linux detection stuff
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

########################################################################
# Automatic LIB_SUFFIX detection + configuration option
########################################################################
if(NOT DEFINED LIB_SUFFIX AND (REDHAT OR SLACKWARE) AND CMAKE_SYSTEM_PROCESSOR MATCHES "64$")
    SET(LIB_SUFFIX 64)
endif()
set(LIB_SUFFIX ${LIB_SUFFIX} CACHE STRING "lib directory suffix")

########################################################################
# Clang required flag because we are using C++11
########################################################################
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    #The stdlib flag setting supports c++11 library features.
    set(CMAKE_CXX_FLAGS "-stdlib=libc++")
endif()
