
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Modules/PothosPythonUtil.cmake)

########################################################################
## Build and install tester blocks for python bindings
########################################################################
POTHOS_PYTHON_UTIL(
    TARGET PythonTestBlocks
    SOURCES
        __init__.py
        Forwarder.py
        SimpleSigSlots.py
    FACTORIES
        "/python/forwarder:PythonTestBlocks.Forwarder"
        "/python/simple_signal_emitter:PythonTestBlocks.SimpleSignalEmitter"
        "/python/simple_slot_acceptor:PythonTestBlocks.SimpleSlotAcceptor"
    DESTINATION PythonTestBlocks
    ENABLE_DOCS
)
