# We can't use the Pothos dynamic dispatch infrastructure we export for
# blocks, as that involves a call into PothosUtil, which depends on what
# we're building here.

include(${PROJECT_SOURCE_DIR}/cmake/Modules/PothosConfigSIMD.cmake)

PothosGenerateSIMDSourcesOnly(PothosSIMDSources
    BufferConvert.cpp
    VectorConvert.cpp)

add_library(PothosSIMD STATIC ${PothosSIMDSources})
target_link_libraries(PothosSIMD xsimd)
set_property(TARGET PothosSIMD PROPERTY POSITION_INDEPENDENT_CODE TRUE)

# This library is pure templates, so expect large object files.
if(MSVC)
    set_property(TARGET PothosSIMD PROPERTY COMPILE_FLAGS /bigobj)
endif()
