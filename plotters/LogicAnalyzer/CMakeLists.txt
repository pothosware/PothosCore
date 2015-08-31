########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_PLOTTERS_LOGICANALYZER "Enable Pothos Plotters.LogicAnalyzer component" ON "ENABLE_PLOTTERS" OFF)
add_feature_info("  Logic Analyzer" ENABLE_PLOTTERS_LOGICANALYZER "Logic analyzer list-view debug display")
if (NOT ENABLE_PLOTTERS_LOGICANALYZER)
    return()
endif()

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
    DESTINATION plotters
)
