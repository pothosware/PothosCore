# We can't use the Pothos dynamic dispatch infrastructure we export for
# blocks, as that involves a call into PothosUtil, which depends on what
# we're building here. To get around this, we'll use the SIMDPP dynamic
# dispatch infrastructure we're cannibalizing.

include(${PROJECT_SOURCE_DIR}/cmake/Modules/PothosConfigSIMD.cmake)

PothosGenerateSIMDSourcesOnly(PothosSIMDSources
    BufferConvert.cpp
    VectorConvert.cpp)

add_library(PothosSIMD STATIC ${PothosSIMDSources})
target_link_libraries(PothosSIMD xsimd)