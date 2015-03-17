########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
option(ENABLE_SERIALIZATION "Enable Pothos Serialization component" ON)
add_feature_info(Serialization ENABLE_SERIALIZATION "The Pothos-Serialization C++ library")
if (NOT ENABLE_SERIALIZATION)
    return()
endif()

########################################################################
# Build Pothos modification of boost serialization
########################################################################
file(GLOB serialization_sources "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-fpermissive)
    add_definitions(-Wno-reorder)
    add_definitions(-Wno-unused-local-typedefs)
    add_definitions(-Wno-unused-but-set-variable)
endif()

#cause symbols to be exported to the dynamic library
add_definitions(-DPOTHOS_ALL_DYN_LINK)

add_library(PothosSerialization SHARED ${serialization_sources})

install(TARGETS PothosSerialization
    LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT pothos_runtime # .so file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT pothos_devel   # .lib file
    RUNTIME DESTINATION bin              COMPONENT pothos_runtime # .dll file
)
