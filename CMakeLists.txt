########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(Pothos)
enable_language(CXX)
enable_testing()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
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
# POCO dependency - build it by default
########################################################################
option(BUILD_POCO "Enable Poco Build" ON)

if(BUILD_POCO)
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
    include(${PROJECT_SOURCE_DIR}/pothos-library/cmake/PothosLibraryConfig.cmake)

    #set top level dirs, go into submodule, and restore
    set(POTHOS_MODULE_PATH ${CMAKE_MODULE_PATH})
    set(CMAKE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/poco)
    set(CMAKE_BINARY_DIR ${PROJECT_BINARY_DIR}/poco)
    unset(CMAKE_MODULE_PATH)
    add_subdirectory(poco)
    set(CMAKE_SOURCE_DIR ${PROJECT_SOURCE_DIR})
    set(CMAKE_BINARY_DIR ${PROJECT_BINARY_DIR})
    set(CMAKE_MODULE_PATH ${POTHOS_MODULE_PATH})

    #set FindPoco variables for submodule build
    set(POCO_FOUND TRUE)

    set(POCO_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/poco/Foundation/include)
    set(POCO_LIBRARY_REL PocoFoundation)

    set(POCO_Util_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/poco/Util/include)
    set(POCO_Util_LIBRARY_REL PocoUtil)

    set(POCO_XML_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/poco/XML/include)
    set(POCO_XML_LIBRARY_REL PocoXML)

    set(POCO_Net_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/poco/Net/include)
    set(POCO_Net_LIBRARY_REL PocoNet)

    set(POCO_JSON_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/poco/JSON/include)
    set(POCO_JSON_LIBRARY_REL PocoJSON)

else(BUILD_POCO)
    find_package(Poco)
    if(NOT POCO_FOUND)
        message(FATAL_ERROR "Cannot find POCO dependency - get it from your package manager")
    endif()

endif(BUILD_POCO)

list(APPEND POCO_INCLUDE_DIRS
    ${POCO_INCLUDE_DIR}
    ${POCO_Util_INCLUDE_DIR}
    ${POCO_XML_INCLUDE_DIR}
    ${POCO_Net_INCLUDE_DIR}
    ${POCO_JSON_INCLUDE_DIR}
)

list(APPEND POCO_LIBRARIES
    ${POCO_LIBRARY_REL}
    ${POCO_Util_LIBRARY_REL}
    ${POCO_XML_LIBRARY_REL}
    ${POCO_Net_LIBRARY_REL}
    ${POCO_JSON_LIBRARY_REL}
)

message(STATUS "POCO_INCLUDE_DIRS: ${POCO_INCLUDE_DIRS}")
message(STATUS "POCO_LIBRARIES: ${POCO_LIBRARIES}")

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
include_directories(${POCO_INCLUDE_DIRS})

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
# use poco's uninstall target, but overwrite the script
#########################################################################
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/poco/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
