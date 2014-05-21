########################################################################
## Build Pothos python module
########################################################################
set(MODULE_SOURCES
    PothosModule.cpp
    ProxyEnvironmentType.cpp
    ProxyType.cpp
    ProxyCallType.cpp
    Converters.cpp
)

#field initializer warnings are unavoidable with PyTypeObject initialization
if (CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wno-missing-field-initializers)
endif()

include_directories(${POTHOS_INCLUDE_DIRS})
add_library(PothosModule MODULE ${MODULE_SOURCES})
target_link_libraries(PothosModule ${POTHOS_LIBRARIES} ${PYTHON_LIBRARIES})

#remove lib prefix and possible debug (d) postfix for importable module name
set_target_properties(PothosModule PROPERTIES
    PREFIX ""
    DEBUG_POSTFIX "${PYTHON_DEBUG_POSTFIX}"
)

#windows python expects pyd file extensions, dlls will not import
if (WIN32)
    set_target_properties(PothosModule PROPERTIES
        SUFFIX ".pyd"
    )
endif()

#install the module and __init__.py importer script
set(PYDIR lib${LIB_SUFFIX}/Pothos/python)
install(TARGETS PothosModule DESTINATION ${PYDIR}/Pothos)
install(FILES
    __init__.py
    Block.py
    Buffer.py
    Label.py
    InputPort.py
    OutputPort.py
    DESTINATION ${PYDIR}/Pothos)
install(FILES
    TestPothos.py
    TestBlocks.py
    DESTINATION ${PYDIR})
