########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(Pothos)
enable_language(CXX)
enable_testing()

########################################################################
# gather version information
# packagers may specify -DPOTHOS_EXTVER="foo" to replace the git hash
########################################################################
set(POTHOS_MAJOR_VERSION 0)
set(POTHOS_MINOR_VERSION 3)
set(POTHOS_PATCH_VERSION 0)
set(POTHOS_LIBVER ${POTHOS_MAJOR_VERSION}.${POTHOS_MINOR_VERSION}.${POTHOS_PATCH_VERSION})

if (NOT POTHOS_EXTVER)
    include(${PROJECT_SOURCE_DIR}/cmake/GetGitRevisionDescription.cmake)
    get_git_head_revision(GITREFSPEC GITHASH)
    if (GITHASH)
        string(SUBSTRING "${GITHASH}" 0 8 GITHASH)
        set(POTHOS_EXTVER "g${GITHASH}")
    else (GITHASH)
        set(POTHOS_EXTVER "unknown")
    endif (GITHASH)
endif()

set(POTHOS_VERSION "${POTHOS_LIBVER}-${POTHOS_EXTVER}")

#set in-tree variables for PothosConfig.cmake
set(Pothos_DIR ${PROJECT_SOURCE_DIR}/cmake)
set(POTHOS_IN_TREE_SOURCE_DIR ${PROJECT_SOURCE_DIR})

#POTHOS_ROOT is compiled into the library to locate the install base.
#By default, the POTHOS_ROOT is set to the CMAKE_INSTALL_PREFIX.
#However users may overload this by specifying -DPOTHOS_ROOT=<path>.
if(NOT POTHOS_ROOT)
    file(TO_CMAKE_PATH ${CMAKE_INSTALL_PREFIX} POTHOS_ROOT)
endif(NOT POTHOS_ROOT)

# Include the Pothos library config into the top level.
# This brings in LIB_SUFFIX and RPATH settings for submodules.
include(${PROJECT_SOURCE_DIR}/cmake/PothosLibraryConfig.cmake)

########################################################################
# POCO C++ utility libraries
########################################################################
include(${PROJECT_SOURCE_DIR}/cmake/SetupPoco.cmake)

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
# Spuce filter design library
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
# muparserx expression parser
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
# Pothos serialization library
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
# Build the framework library
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LIBRARY "Enable Pothos Library component" ON "TRUE" OFF)
add_feature_info(Library ENABLE_LIBRARY "The main library for all Pothos projects")

if(ENABLE_LIBRARY)
    find_package(Pothos CONFIG REQUIRED)
    add_subdirectory(include)
    add_subdirectory(lib)
    add_subdirectory(cmake)
    add_subdirectory(apps)
    add_subdirectory(docs)
endif(ENABLE_LIBRARY)

########################################################################
## Build submodule toolkits
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_TOOLKITS "Enable Pothos Toolkits components" ON "ENABLE_LIBRARY" OFF)
add_feature_info(Toolkits ENABLE_TOOLKITS "Pothos data-flow software suite toolkits")

if(ENABLE_TOOLKITS)
    add_subdirectory(blocks)
    add_subdirectory(comms)
    add_subdirectory(python)
    add_subdirectory(sdr)
    add_subdirectory(audio)
    add_subdirectory(gui)
    add_subdirectory(widgets)
    add_subdirectory(plotters)
endif(ENABLE_TOOLKITS)

########################################################################
# uninstall target
########################################################################
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

########################################################################
# print features summary
########################################################################
include(FeatureSummary)
message(STATUS "")
message(STATUS "######################################################")
message(STATUS "## Enabled features")
message(STATUS "######################################################")
feature_summary(WHAT ENABLED_FEATURES)
message(STATUS "######################################################")
message(STATUS "## Disabled features")
message(STATUS "######################################################")
feature_summary(WHAT DISABLED_FEATURES)
message(STATUS "Pothos version: v${POTHOS_VERSION}")
message(STATUS "ABI/so version: v${POTHOS_SOVER}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
