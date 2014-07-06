########################################################################
# Compiler support module
########################################################################
set(SOURCES
    TestCompilerSupport.cpp
)

if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND SOURCES GccCompilerSupport.cpp)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    list(APPEND SOURCES ClangCompilerSupport.cpp)
endif()

if(MSVC)
    get_filename_component(MSVC_INSTALL_PATH ${CMAKE_LINKER} PATH)
    file(TO_NATIVE_PATH ${MSVC_INSTALL_PATH} MSVC_INSTALL_PATH)
    string(REPLACE "\\" "\\\\" MSVC_INSTALL_PATH ${MSVC_INSTALL_PATH})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/MsvcCompilerSupport.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/MsvcCompilerSupport.cpp @ONLY)
    list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/MsvcCompilerSupport.cpp)
endif(MSVC)

POTHOS_MODULE_UTIL(
    TARGET CompilerSupport
    SOURCES ${SOURCES}
    DESTINATION util
)
