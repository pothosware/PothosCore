if(POTHOS_IN_TREE_SOURCE_DIR)
    set(SPUCE_INCLUDE_DIR ${POTHOS_IN_TREE_SOURCE_DIR}/comms)
    set(SPUCE_LIBRARY spuce)
else()
    find_path(
        SPUCE_INCLUDE_DIR spuce/typedefs.h
        PATHS ${POTHOS_ROOT}/include
        NO_DEFAULT_PATH
    )
    find_library(
        SPUCE_LIBRARY spuce spucelibd
        PATHS ${POTHOS_ROOT}/lib${LIB_SUFFIX}
        NO_DEFAULT_PATH
    )
endif()

set (SPUCE_INCLUDE_DIRS ${SPUCE_INCLUDE_DIR})
set (SPUCE_LIBRARIES ${SPUCE_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SPUCE DEFAULT_MSG SPUCE_LIBRARY SPUCE_INCLUDE_DIR)
