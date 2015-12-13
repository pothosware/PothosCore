########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(Pothos)
enable_language(CXX)
enable_testing()

#args to set the CMAKE_PREFIX_PATH environment variable
if (CMAKE_PREFIX_PATH)
    set(ENV{CMAKE_PREFIX_PATH} ${CMAKE_PREFIX_PATH})
endif (CMAKE_PREFIX_PATH)

########################################################################
# POCO dependency - search for Poco or build if not found
########################################################################
include(${PROJECT_SOURCE_DIR}/library/cmake/SetupPoco.cmake)

set(POCO_DIR poco154)

if (NOT Poco_FOUND)

    message(STATUS "Poco >= 1.5.4 not found - using built-in Poco")

    if (NOT EXISTS ${PROJECT_SOURCE_DIR}/${POCO_DIR}/CMakeLists.txt)
        message(FATAL_ERROR "poco submodule not configured (git submodule init && git submodule update)")
    endif ()

    #mark that the in-tree poco build is used
    #used by uninstall target and PothosConfig
    set(POCO_IN_TREE TRUE)

    #when in-tree poco is selected, default to a minimum build
    #users can specify -DPOCO_MIN_BUILD=TRUE to enable full poco
    option(POCO_MIN_BUILD "Minimum build for in-tree Poco" TRUE)
    if(POCO_MIN_BUILD)
        set(ENABLE_MONGODB OFF CACHE INTERNAL "")
        set(ENABLE_PDF OFF CACHE INTERNAL "")
        set(ENABLE_NETSSL OFF CACHE INTERNAL "")
        set(ENABLE_CRYPTO OFF CACHE INTERNAL "")
        set(ENABLE_DATA OFF CACHE INTERNAL "")
        set(ENABLE_SEVENZIP OFF CACHE INTERNAL "")
        set(ENABLE_ZIP OFF CACHE INTERNAL "")
        set(ENABLE_APACHECONNECTOR OFF CACHE INTERNAL "")
    endif()

    # Include the Pothos library config into the top level.
    # This brings in LIB_SUFFIX and RPATH settings for Poco.
    include(${PROJECT_SOURCE_DIR}/library/cmake/PothosLibraryConfig.cmake)

    #set top level dirs, go into submodule, and restore
    set(POTHOS_MODULE_PATH ${CMAKE_MODULE_PATH})
    set(CMAKE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/${POCO_DIR})
    set(CMAKE_BINARY_DIR ${PROJECT_BINARY_DIR}/${POCO_DIR})
    unset(CMAKE_MODULE_PATH)
    add_subdirectory(${POCO_DIR})
    set(CMAKE_SOURCE_DIR ${PROJECT_SOURCE_DIR})
    set(CMAKE_BINARY_DIR ${PROJECT_BINARY_DIR})
    set(CMAKE_MODULE_PATH ${POTHOS_MODULE_PATH})

    set(Poco_INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/${POCO_DIR}/Foundation/include
        ${PROJECT_SOURCE_DIR}/${POCO_DIR}/Util/include
        ${PROJECT_SOURCE_DIR}/${POCO_DIR}/XML/include
        ${PROJECT_SOURCE_DIR}/${POCO_DIR}/Net/include
        ${PROJECT_SOURCE_DIR}/${POCO_DIR}/JSON/include
    )

    set(Poco_LIBRARIES
        PocoFoundation
        PocoUtil
        PocoXML
        PocoNet
        PocoJSON
    )

    #ensure that local headers get precedent over installed headers
    include_directories(${Poco_INCLUDE_DIRS})

endif()

########################################################################
# Spuce dependency - use system spuce or submodule
########################################################################
find_package(Spuce CONFIG)

if (NOT Spuce_FOUND)
    message(STATUS "Spuce not found - using built-in Spuce")

    if (NOT EXISTS ${PROJECT_SOURCE_DIR}/spuce/CMakeLists.txt)
        message(FATAL_ERROR "spuce submodule not configured (git submodule init && git submodule update)")
    endif ()

    add_subdirectory(spuce)
    set(SPUCE_IN_TREE TRUE)
    set(Spuce_FOUND TRUE)
    set(Spuce_VERSION "[submodule]")
    set(Spuce_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/spuce)
    set(Spuce_LIBRARIES spuce)
endif()

########################################################################
# muparserx dependency - use system muparserx or submodule
########################################################################
find_package(muparserx CONFIG)

if (NOT muparserx_FOUND)
    message(STATUS "muparserx not found - using built-in muparserx")

    if (NOT EXISTS ${PROJECT_SOURCE_DIR}/muparserx/CMakeLists.txt)
        message(FATAL_ERROR "muparserx submodule not configured (git submodule init && git submodule update)")
    endif ()

    add_subdirectory(muparserx)
    set(MUPARSERX_IN_TREE TRUE)
    set(muparserx_FOUND TRUE)
    set(muparserx_VERSION "[submodule]")
    set(muparserx_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/muparserx/parser)
    set(muparserx_LIBRARIES muparserx)
endif()

########################################################################
# Build serialization library
########################################################################
find_package(PothosSerialization CONFIG)

if (NOT PothosSerialization_FOUND)
    message(STATUS "PothosSerialization not found - using built-in PothosSerialization")

    if (NOT EXISTS ${PROJECT_SOURCE_DIR}/serialization/CMakeLists.txt)
        message(FATAL_ERROR "PothosSerialization submodule not configured (git submodule init && git submodule update)")
    endif ()

    add_subdirectory(serialization)
    set(PothosSerialization_IN_TREE TRUE)
    set(PothosSerialization_FOUND TRUE)
    set(PothosSerialization_VERSION "[submodule]")
    set(PothosSerialization_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/serialization/include)
    set(PothosSerialization_LIBRARIES PothosSerialization)
endif()

########################################################################
# Build main pothos library
########################################################################
add_subdirectory(library)

#########################################################################
## Build in-tree modules
#########################################################################
add_subdirectory(blocks)
add_subdirectory(comms)
add_subdirectory(python)
add_subdirectory(java)
add_subdirectory(util)
add_subdirectory(opencl)
add_subdirectory(sdr)
add_subdirectory(audio)
add_subdirectory(gui)
add_subdirectory(widgets)
add_subdirectory(plotters)

#########################################################################
# uninstall target
#########################################################################
if (POCO_IN_TREE)
    # use poco's uninstall target, but overwrite the script
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${POCO_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

else (POCO_IN_TREE)
    add_custom_target(uninstall
        "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

endif (POCO_IN_TREE)

#########################################################################
# print features summary
#########################################################################
include(FeatureSummary)
message(STATUS "######################################################")
message(STATUS "## Enabled features")
message(STATUS "######################################################")
feature_summary(WHAT ENABLED_FEATURES)
message(STATUS "######################################################")
message(STATUS "## Disabled features")
message(STATUS "######################################################")
feature_summary(WHAT DISABLED_FEATURES)
