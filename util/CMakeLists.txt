########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosUtil CXX)

find_package(Pothos CONFIG REQUIRED)

########################################################################
# Serialization library
########################################################################
if(NOT PothosSerialization_IN_TREE)
    find_package(PothosSerialization CONFIG)
endif(NOT PothosSerialization_IN_TREE)
message(STATUS "PothosSerialization_FOUND: ${PothosSerialization_FOUND} - ${PothosSerialization_VERSION}")
message(STATUS "PothosSerialization_INCLUDE_DIRS: ${PothosSerialization_INCLUDE_DIRS}")
message(STATUS "PothosSerialization_LIBRARIES: ${PothosSerialization_LIBRARIES}")

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_UTILITY "Enable Pothos Util component" ON "ENABLE_LIBRARY;PothosSerialization_FOUND" OFF)
add_feature_info(Utility ENABLE_UTILITY "Common utility modules shared by components")
if (NOT ENABLE_UTILITY)
    return()
endif()

########################################################################
# Compiler support
########################################################################
add_subdirectory(Compiler)

########################################################################
# Eval support
########################################################################
get_filename_component(MUPARSERX_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/muparserx/parser" ABSOLUTE)
include_directories(${MUPARSERX_ROOT})
file(GLOB MUPARSERX_SOURCES "${MUPARSERX_ROOT}/*.cpp")

POTHOS_MODULE_UTIL(
    TARGET EvalSupport
    SOURCES
        EvalEnvironment.cpp
        EvalEnvironmentListParsers.cpp
        TestEvalExpression.cpp
        BlockEval.cpp
        ${MUPARSERX_SOURCES}
    DESTINATION util
)

########################################################################
# JSON support (using POCO JSON)
########################################################################
include_directories(${PothosSerialization_INCLUDE_DIRS})

POTHOS_MODULE_UTIL(
    TARGET JSONSupport
    SOURCES JSONSerialization.cpp TestJSONSerialization.cpp
    DESTINATION util
    LIBRARIES ${PothosSerialization_LIBRARIES}
)

########################################################################
# Misc support (used by GUI)
# This module used heavily by the GUI but is not depedent on the GUI
# so it can be deployed on remote machines without the GUI installed.
########################################################################
set(SOURCES
    ConvertContainers.cpp
    DeviceInfoUtils.cpp
    DocUtils.cpp
    TestDocUtils.cpp
)

POTHOS_MODULE_UTIL(
    TARGET MiscSupport
    SOURCES ${SOURCES}
    DESTINATION util
)

########################################################################
# CPU device info for windows
########################################################################
if (WIN32)
POTHOS_MODULE_UTIL(
    TARGET WindowsGetLogicalProcessorInfo
    SOURCES WindowsGetLogicalProcessorInfo.cpp
    DESTINATION util
)
endif()
