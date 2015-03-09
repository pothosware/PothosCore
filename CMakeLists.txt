########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(Pothos)
enable_language(CXX)
enable_testing()

set(Pothos_DIR ${PROJECT_SOURCE_DIR}/pothos-library/cmake)
set(POTHOS_IN_TREE_SOURCE_DIR ${PROJECT_SOURCE_DIR})

########################################################################
# Clang required flag because only libc++ supports C++11.
# Poco must be built with this flag when using clang.
# Mixing versions of the stdlib would cause ABI errors.
########################################################################
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    #The stdlib flag setting supports c++11 library features.
    set(CMAKE_CXX_FLAGS "-stdlib=libc++")
endif()

########################################################################
# POCO dependency - search for Poco or build if not found
########################################################################
include(${Pothos_DIR}/SetupPoco.cmake)

if (NOT Poco_FOUND)

    message(STATUS "Poco not found - using built-in Poco")

    #mark that the in-tree poco build is used
    #used by uninstall target and PothosConfig
    set(POCO_IN_TREE TRUE)

    set(ENABLE_MONGODB OFF CACHE INTERNAL "")
    set(ENABLE_PDF OFF CACHE INTERNAL "")
    set(ENABLE_NETSSL OFF CACHE INTERNAL "")
    set(ENABLE_CRYPTO OFF CACHE INTERNAL "")
    set(ENABLE_DATA OFF CACHE INTERNAL "")
    set(ENABLE_SEVENZIP OFF CACHE INTERNAL "")
    set(ENABLE_ZIP OFF CACHE INTERNAL "")
    set(ENABLE_APACHECONNECTOR OFF CACHE INTERNAL "")

    # Include the Pothos library config into the top level.
    # This brings in LIB_SUFFIX and RPATH settings for Poco.
    include(${Pothos_DIR}/PothosLibraryConfig.cmake)

    #set top level dirs, go into submodule, and restore
    set(POTHOS_MODULE_PATH ${CMAKE_MODULE_PATH})
    set(CMAKE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/poco)
    set(CMAKE_BINARY_DIR ${PROJECT_BINARY_DIR}/poco)
    unset(CMAKE_MODULE_PATH)
    add_subdirectory(poco)
    set(CMAKE_SOURCE_DIR ${PROJECT_SOURCE_DIR})
    set(CMAKE_BINARY_DIR ${PROJECT_BINARY_DIR})
    set(CMAKE_MODULE_PATH ${POTHOS_MODULE_PATH})

    list(APPEND Poco_INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/poco/Foundation/include
        ${PROJECT_SOURCE_DIR}/poco/Util/include
        ${PROJECT_SOURCE_DIR}/poco/XML/include
        ${PROJECT_SOURCE_DIR}/poco/Net/include
        ${PROJECT_SOURCE_DIR}/poco/JSON/include
    )

    list(APPEND Poco_LIBRARIES
        PocoFoundation
        PocoUtil
        PocoXML
        PocoNet
        PocoJSON
    )

endif()

########################################################################
# Build serialization library
########################################################################
add_subdirectory(pothos-serialization)

########################################################################
# Build main pothos library
########################################################################
add_subdirectory(pothos-library)

#########################################################################
## Build in-tree modules
#########################################################################

#ensure that local headers get precedent over installed headers
include_directories(pothos-library/include)
include_directories(${Poco_INCLUDE_DIRS})

add_subdirectory(pothos-gui)
add_subdirectory(pothos-widgets)
add_subdirectory(pothos-blocks)
add_subdirectory(pothos-python)
add_subdirectory(pothos-java)
add_subdirectory(pothos-util)
add_subdirectory(pothos-opencl)
add_subdirectory(pothos-sdr)
add_subdirectory(pothos-audio)

#########################################################################
# uninstall target
#########################################################################
if (POCO_IN_TREE)
    # use poco's uninstall target, but overwrite the script
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/poco/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

else (POCO_IN_TREE)
    add_custom_target(uninstall
        "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

endif (POCO_IN_TREE)
