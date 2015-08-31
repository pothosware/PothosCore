########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosJava CXX)

find_package(Pothos CONFIG REQUIRED)

########################################################################
## Find JNI
########################################################################
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
find_package(JNI)

message(STATUS "JNI_FOUND: ${JNI_FOUND}")
message(STATUS "JNI_INCLUDE_DIRS: ${JNI_INCLUDE_DIRS}")
message(STATUS "JNI_LIBRARIES: ${JNI_LIBRARIES}")

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_JAVA "Enable Pothos Java component" ON "ENABLE_LIBRARY;JNI_FOUND" OFF)
add_feature_info(Java ENABLE_JAVA "Java bindings for the Pothos framework")
if (NOT ENABLE_JAVA)
    return()
endif()

########################################################################
## Build the module
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
