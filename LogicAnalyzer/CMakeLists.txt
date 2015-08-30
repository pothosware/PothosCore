########################################################################
# Build logic analyzer plot module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET LogicAnalyzer
    SOURCES
        LogicAnalyzer.cpp
        LogicAnalyzerDisplay.cpp
    DOC_SOURCES LogicAnalyzer.cpp
    LIBRARIES ${Qt5_LIBRARIES}
    DESTINATION widgets
)
