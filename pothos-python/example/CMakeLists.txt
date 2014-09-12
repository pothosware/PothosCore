
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Modules/PothosPythonUtil.cmake)

########################################################################
## Build and install tester blocks for python bindings
########################################################################
POTHOS_PYTHON_UTIL(
    TARGET PythonTestBlocks
    SOURCES TestBlocks.py
    DESTINATION PythonTestBlocks
    ENABLE_DOCS
)
