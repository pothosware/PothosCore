########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_LIBRARY_APPS "Enable Pothos Library.Apps component" ON "ENABLE_LIBRARY" OFF)
add_feature_info("  Apps" ENABLE_LIBRARY_APPS "Utility application for Pothos library")
if (NOT ENABLE_LIBRARY_APPS)
    return()
endif()

########################################################################
# Local includes
########################################################################
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${Pothos_INCLUDE_DIRS})
include_directories(${JSON_HPP_INCLUDE_DIR})

########################################################################
# apps
########################################################################
list(APPEND SOURCES
    PothosUtil.cpp
    PothosUtilModuleInfo.cpp
    PothosUtilSelfTests.cpp
    PothosUtilPluginTree.cpp
    PothosUtilDeviceInfo.cpp
    PothosUtilProxyServer.cpp
    PothosUtilLoadModule.cpp
    PothosUtilDocParse.cpp
    PothosUtilRunTopology.cpp
)
add_executable(PothosUtil ${SOURCES})
target_link_libraries(PothosUtil Pothos ${Pothos_LIBRARIES})
install(TARGETS PothosUtil
    RUNTIME DESTINATION bin COMPONENT pothos_runtime
)
