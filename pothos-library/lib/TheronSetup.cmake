########################################################################
# Include this file to get a list of include paths and definitions
# Input: THERON_SOURCE_DIR
# Output: THERON_INCLUDE_DIRS
# Output: THERON_LIBRARY_DIRS
# Output: THERON_LIBRARIES
# Output: THERON_DEFINES
# Output: THERON_SOURCES
########################################################################

list(APPEND CMAKE_MODULE_PATH ${THERON_SOURCE_DIR})

########################################################################
# Setup the list of sources
########################################################################
file(GLOB THERON_SOURCES "${THERON_SOURCE_DIR}/Theron/*.cpp")

########################################################################
# Detect the system defines
########################################################################
if(WIN32)
    list(APPEND THERON_DEFINES -DTHERON_WINDOWS=1)
endif()

if(MSVC)
    list(APPEND THERON_DEFINES -DTHERON_MSVC=1)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND THERON_DEFINES -DTHERON_GCC=1)
endif()

include(CheckTypeSize)
enable_language(C)
check_type_size("void*[8]" SIZEOF_CPU BUILTIN_TYPES_ONLY)
if(${SIZEOF_CPU} EQUAL 64)
    list(APPEND THERON_DEFINES -DTHERON_64BIT=1)
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    list(APPEND THERON_DEFINES -DTHERON_DEBUG=1)
endif()

list(APPEND THERON_DEFINES -DTHERON_CPP11=1) #using C++11 everywhere

########################################################################
# Setup the include directories
########################################################################
list(APPEND THERON_INCLUDE_DIRS ${THERON_SOURCE_DIR}/Include)

########################################################################
# Extra linux specific libraries
########################################################################
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    find_library(
        NUMA_LIBRARIES
        NAMES numa
        PATHS /usr/lib /usr/lib64
    )

    if(NUMA_LIBRARIES)
        list(APPEND THERON_LIBRARIES ${NUMA_LIBRARIES})
        list(APPEND THERON_DEFINES -DTHERON_NUMA=1)
    endif()

    find_library(
        RT_LIBRARIES
        NAMES rt
        PATHS /usr/lib /usr/lib64
    )

    if(RT_LIBRARIES)
        list(APPEND THERON_LIBRARIES ${RT_LIBRARIES})
    else()
        message(FATAL_ERROR "librt required to build Theron")
    endif()

endif()

########################################################################
# Print results
########################################################################
message(STATUS "THERON_INCLUDE_DIRS: ${THERON_INCLUDE_DIRS}")
message(STATUS "THERON_LIBRARY_DIRS: ${THERON_LIBRARY_DIRS}")
message(STATUS "THERON_LIBRARIES: ${THERON_LIBRARIES}")
message(STATUS "THERON_DEFINES: ${THERON_DEFINES}")
