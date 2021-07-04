########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_LIBRARY_APPS "Enable Pothos Library.Apps component" ON "ENABLE_LIBRARY" OFF)
add_feature_info("  Apps" ENABLE_LIBRARY_APPS "Utility application for Pothos library")
if (NOT ENABLE_LIBRARY_APPS)
    return()
endif()

########################################################################
# build PothosUtil
########################################################################
add_executable(PothosUtil
    PothosUtil.cpp
    PothosUtilSystemInfo.cpp
    PothosUtilModuleInfo.cpp
    PothosUtilSelfTests.cpp
    PothosUtilPluginTree.cpp
    PothosUtilDeviceInfo.cpp
    PothosUtilProxyServer.cpp
    PothosUtilLoadModule.cpp
    PothosUtilDocParse.cpp
    PothosUtilRunTopology.cpp
    PothosUtilListModules.cpp
    PothosUtilProxyEnvironmentInfo.cpp
    PothosUtilSIMDFeatures.cpp
    PothosUtilGenerateSIMDDispatchers.cpp
    PothosUtilListTypeConversions.cpp
)
target_include_directories(PothosUtil PRIVATE ${JSON_HPP_INCLUDE_DIR})
target_link_libraries(PothosUtil PRIVATE Pothos)
install(TARGETS PothosUtil
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT pothos_runtime
)
