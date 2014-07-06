set(SOURCES
    EvalEnvironment.cpp
    BlockEval.cpp
    TopologyEngine.cpp
    DeviceInfoUtils.cpp
    DocUtils.cpp
    TestDocUtils.cpp
    TestEvalExpression.cpp
)

POTHOS_MODULE_UTIL(
    TARGET GuiSupport
    SOURCES ${SOURCES}
    DESTINATION gui
)
