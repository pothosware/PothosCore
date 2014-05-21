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
    gen_tmpl(${POTHOS_SOURCE_DIR}/include/Pothos/Callable/Callable.tmpl.hpp)
    gen_tmpl(${POTHOS_SOURCE_DIR}/include/Pothos/Callable/CallableImpl.tmpl.hpp)
    gen_tmpl(${POTHOS_SOURCE_DIR}/include/Pothos/Proxy/Proxy.tmpl.hpp)
    gen_tmpl(${POTHOS_SOURCE_DIR}/include/Pothos/Proxy/ProxyImpl.tmpl.hpp)
    gen_tmpl(${POTHOS_SOURCE_DIR}/include/Pothos/Managed/Class.tmpl.hpp)
    gen_tmpl(${POTHOS_SOURCE_DIR}/include/Pothos/Managed/ClassImpl.tmpl.hpp)
    gen_tmpl(${POTHOS_SOURCE_DIR}/include/Pothos/Framework/CallRegistry.tmpl.hpp)
    gen_tmpl(${POTHOS_SOURCE_DIR}/include/Pothos/Framework/CallRegistryImpl.tmpl.hpp)
    add_custom_target(gen_tmpl ALL DEPENDS ${gen_tmpl_outputs})
endif()

########################################################################
# precompiled headers - top level headers only
########################################################################
file(GLOB_RECURSE all_include_files
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${POTHOS_SOURCE_DIR}/include/Pothos/*.hpp)
list(APPEND all_include_files ${gen_tmpl_outputs})

file(GLOB include_files
    RELATIVE ${POTHOS_SOURCE_DIR}
    ${POTHOS_SOURCE_DIR}/include/Pothos/*.hpp)

foreach(include_file ${include_files})
    get_filename_component(include_path ${include_file} PATH)
    file(MAKE_DIRECTORY ${POTHOS_BINARY_DIR}/${include_path})
    set(pch_source ${POTHOS_BINARY_DIR}/${include_file}.cpp)
    file(WRITE ${pch_source} "#include <${POTHOS_SOURCE_DIR}/${include_file}>\n")
    if(CMAKE_COMPILER_IS_GNUCXX)
        set(pch_file ${POTHOS_BINARY_DIR}/${include_file}.gch)
        add_custom_command(
            DEPENDS ${all_include_files} OUTPUT ${pch_file}
            COMMAND ${CMAKE_CXX_COMPILER} -std=c++11
                -I ${POTHOS_SOURCE_DIR}/include
                -x c++-header ${pch_source} -o ${pch_file}
        )
    elseif(MSVC)
        set(pch_file ${POTHOS_BINARY_DIR}/${include_file}.pch)
        add_custom_command(
            DEPENDS ${all_include_files} OUTPUT ${pch_file}
            COMMAND ${CMAKE_CXX_COMPILER} /c /EHsc
                /I ${POTHOS_SOURCE_DIR}/include /Yc ${pch_source}
            WORKING_DIRECTORY ${POTHOS_BINARY_DIR}/${include_path}
        )
    endif()
    if(pch_file)
        list(APPEND pch_files ${pch_file})
        install(
            FILES ${pch_file}
            DESTINATION ${include_path}
            COMPONENT pothos_devel
        )
    endif()
endforeach(include_file)

if (pch_files)
    add_custom_target(pch_build ALL DEPENDS ${pch_files})
endif (pch_files)
