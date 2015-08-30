########################################################################
# install include files
########################################################################
install(
    DIRECTORY Pothos
    DESTINATION include
    COMPONENT pothos_devel
    PATTERN "*.tmpl.*" EXCLUDE
)

########################################################################
# generated headers
########################################################################
find_package(PythonInterp)

#convenience macro to generate from a templated file
function(gen_tmpl inFile)
    string(REPLACE ".tmpl." "." outFile ${inFile})
    set(exp_tmpl_py ${CMAKE_CURRENT_SOURCE_DIR}/ExpandTmpl.py)
    add_custom_command(
        OUTPUT ${outFile}
        COMMAND ${PYTHON_EXECUTABLE} ${exp_tmpl_py} ${inFile} ${outFile}
        DEPENDS ${inFile} ${exp_tmpl_py}
    )
    list(APPEND gen_tmpl_outputs ${outFile})
    set(gen_tmpl_outputs ${gen_tmpl_outputs} PARENT_SCOPE)
endfunction(gen_tmpl)

#declare generation rules for the templated sources
#developers must manually configure with -DGEN_ENABLED for generation
if(GEN_ENABLED AND PYTHON_EXECUTABLE)
    gen_tmpl(${CMAKE_CURRENT_SOURCE_DIR}/Pothos/Callable/Callable.tmpl.hpp)
    gen_tmpl(${CMAKE_CURRENT_SOURCE_DIR}/Pothos/Callable/CallableImpl.tmpl.hpp)
    gen_tmpl(${CMAKE_CURRENT_SOURCE_DIR}/Pothos/Callable/CallInterface.tmpl.hpp)
    gen_tmpl(${CMAKE_CURRENT_SOURCE_DIR}/Pothos/Callable/CallInterfaceImpl.tmpl.hpp)
    gen_tmpl(${CMAKE_CURRENT_SOURCE_DIR}/Pothos/Callable/CallRegistry.tmpl.hpp)
    gen_tmpl(${CMAKE_CURRENT_SOURCE_DIR}/Pothos/Callable/CallRegistryImpl.tmpl.hpp)
    gen_tmpl(${CMAKE_CURRENT_SOURCE_DIR}/Pothos/Proxy/Proxy.tmpl.hpp)
    gen_tmpl(${CMAKE_CURRENT_SOURCE_DIR}/Pothos/Proxy/ProxyImpl.tmpl.hpp)
    gen_tmpl(${CMAKE_CURRENT_SOURCE_DIR}/Pothos/Managed/Class.tmpl.hpp)
    gen_tmpl(${CMAKE_CURRENT_SOURCE_DIR}/Pothos/Managed/ClassImpl.tmpl.hpp)
    add_custom_target(gen_tmpl ALL DEPENDS ${gen_tmpl_outputs})
endif()
