########################################################################
# Build parser support module
########################################################################
get_filename_component(MUPARSERX_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/muparserx/parser" ABSOLUTE)
include_directories(${MUPARSERX_ROOT})
file(GLOB SOURCES "${MUPARSERX_ROOT}/*.cpp")

list(APPEND SOURCES TestParser.cpp)

POTHOS_MODULE_UTIL(
    TARGET ParserSupport
    SOURCES ${SOURCES}
    DESTINATION util
)
