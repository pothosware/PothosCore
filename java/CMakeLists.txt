########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosJava CXX)

find_package(Pothos CONFIG REQUIRED)

########################################################################
## Find Java
########################################################################
find_package(Java)
message(STATUS "Java_VERSION: ${Java_VERSION}")

if(Java_VERSION)
    if ("${Java_VERSION}" VERSION_LESS "1.7")
        message(WARNING "Java bindings require Java v1.7 and up")
    else()
        set(Java_VERSION_OK TRUE)
    endif()
endif(Java_VERSION)

########################################################################
## Find JNI
########################################################################
find_package(JNI)

message(STATUS "JAVA_HOME: $ENV{JAVA_HOME}")
message(STATUS "JNI_FOUND: ${JNI_FOUND}")
message(STATUS "JNI_INCLUDE_DIRS: ${JNI_INCLUDE_DIRS}")
message(STATUS "JNI_LIBRARIES: ${JNI_LIBRARIES}")

if(NOT JNI_FOUND AND "$ENV{JAVA_HOME}" STREQUAL "")
    message(WARNING "JNI not found, you may have to set JAVA_HOME\n"
    "Example: export JAVA_HOME=/usr/lib/jvm/java-7-openjdk-amd64")
endif()

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_JAVA "Enable Pothos Java component" ON "Pothos_FOUND;Java_VERSION_OK;JNI_FOUND" OFF)
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
