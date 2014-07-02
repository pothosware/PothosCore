set(SOURCES
    EvalEnvironment.cpp
    BlockEval.cpp
    DeviceInfoUtils.cpp
    DocUtils.cpp
    TestDocUtils.cpp
    EvalExpression.cpp
    TestEvalExpression.cpp
)

POTHOS_MODULE_UTIL(
    TARGET GuiSupport
    SOURCES ${SOURCES}
    DESTINATION gui
)
