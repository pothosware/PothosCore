# We can't use the Pothos dynamic dispatch infrastructure we export for
# blocks, as that involves a call into PothosUtil, which depends on what
# we're building here.
include(${PROJECT_SOURCE_DIR}/cmake/Modules/PothosConfigSIMD.cmake)

PothosGenerateSIMDSourcesOnly(PothosSIMDSources
    BufferConvert.cpp
    VectorConvert.cpp)
    
set(PothosSIMDSources
    ${CMAKE_CURRENT_SOURCE_DIR}/BufferConversions.cpp
    ${PothosSIMDSources}
PARENT_SCOPE)