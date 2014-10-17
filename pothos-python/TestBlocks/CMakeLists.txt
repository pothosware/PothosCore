########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosPythonTestBlocks)
enable_language(CXX)

find_package(Pothos NO_MODULE REQUIRED)
include(PothosPythonUtil)

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
        "/python/forwarder:Forwarder"
        "/python/simple_signal_emitter:SimpleSignalEmitter"
        "/python/simple_slot_acceptor:SimpleSlotAcceptor"
    DESTINATION TestBlocks
    ENABLE_DOCS
)
