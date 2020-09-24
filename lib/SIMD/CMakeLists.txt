# We can't use the Pothos dynamic dispatch infrastructure we export for
# blocks, as that involves a call into PothosUtil, which depends on what
# we're building here. To get around this, we'll use the SIMDPP dynamic
# dispatch infrastructure we're cannibalizing.

include(${PROJECT_SOURCE_DIR}/libsimdpp/cmake/SimdppMultiarch.cmake)

# See https://github.com/p12tic/libsimdpp/blob/master/cmake/SimdppMultiarch.cmake
simdpp_get_compilable_archs(SIMDBuildArchs)
set(DISPATCHER_CXX_FLAGS "-DSIMDPP_EMIT_DISPATCHER=1")
set(DISPATCH_ARCH_IDX "1")
foreach(arch ${SIMDBuildArchs})
    simdpp_get_arch_info(CXX_FLAGS DEFINES_LIST SUFFIX ${arch})
    if(NOT "${SUFFIX}" STREQUAL "")
        set(DISPATCHER_CXX_FLAGS "${DISPATCHER_CXX_FLAGS} -DSIMDPP_DISPATCH_ARCH${DISPATCH_ARCH_IDX}=${DEFINES_LIST}")
        math(EXPR DISPATCH_ARCH_IDX "${DISPATCH_ARCH_IDX}+1")
    endif()
endforeach()

include(${PROJECT_SOURCE_DIR}/cmake/Modules/PothosConfigSIMD.cmake)

PothosGenerateSIMDSourcesOnly(PothosSIMDSources
    BufferConvert.cpp
    VectorConvert.cpp)
    
list(APPEND PothosSIMDSources BufferConversions.cpp)
set_property(SOURCE BufferConversions.cpp APPEND_STRING PROPERTY COMPILE_FLAGS "${DISPATCHER_CXX_FLAGS}")

add_library(PothosSIMD STATIC ${PothosSIMDSources})
target_link_libraries(PothosSIMD xsimd)