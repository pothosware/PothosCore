########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosJava)
enable_language(CXX)

if (NOT POTHOS_MODULE_PATH)
    set(POTHOS_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/share/Pothos/cmake/Modules")
endif()

include(${POTHOS_MODULE_PATH}/PothosUtil.cmake)

########################################################################
## Find JNI
########################################################################
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
find_package(JNI)

message(STATUS "JNI_FOUND: ${JNI_FOUND}")
message(STATUS "JNI_INCLUDE_DIRS: ${JNI_INCLUDE_DIRS}")
message(STATUS "JNI_LIBRARIES: ${JNI_LIBRARIES}")

if(NOT JNI_FOUND)
    return()
endif()

########################################################################
include_directories(${JNI_INCLUDE_DIRS})

set(SOURCES
    JavaProxy.cpp
    JavaConvert.cpp
    JavaHelper.cpp
    JavaHandle.cpp
    TestJava.cpp
    Containers.cpp
)

POTHOS_MODULE_UTIL(
    TARGET JavaSupport
    SOURCES ${SOURCES}
    LIBRARIES ${JNI_LIBRARIES}
    DESTINATION proxy/environment
)
