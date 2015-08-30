TEMPLATE = app
TARGET = example
CONFIG += console
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11 -Wall -pedantic

CONFIG(debug, debug|release) {
    DESTDIR = ./debug
    OBJECTS_DIR = ./debug/obj
    MOC_DIR = ./debug/moc
    RCC_DIR = ./debug/rcc
    UI_DIR = ./debug/ui
} else {
    DESTDIR = ./release
    OBJECTS_DIR = ./release/obj
    MOC_DIR = ./release/moc
    RCC_DIR = ./release/rcc
    UI_DIR = ./release/ui
}

SOURCES += \
    ../../parser/mpVariable.cpp \
    ../../parser/mpValueCache.cpp \
    ../../parser/mpValue.cpp \
    ../../parser/mpValReader.cpp \
    ../../parser/mpTokenReader.cpp \
    ../../parser/mpTest.cpp \
    ../../parser/mpScriptTokens.cpp \
    ../../parser/mpRPN.cpp \
    ../../parser/mpParserBase.cpp \
    ../../parser/mpParser.cpp \
    ../../parser/mpParserMessageProvider.cpp \
    ../../parser/mpPackageUnit.cpp \
    ../../parser/mpPackageStr.cpp \
    ../../parser/mpPackageNonCmplx.cpp \
    ../../parser/mpPackageMatrix.cpp \
    ../../parser/mpPackageCommon.cpp \
    ../../parser/mpPackageCmplx.cpp \
    ../../parser/mpOprtNonCmplx.cpp \
    ../../parser/mpOprtMatrix.cpp \
    ../../parser/mpOprtIndex.cpp \
    ../../parser/mpOprtCmplx.cpp \
    ../../parser/mpOprtPostfixCommon.cpp \
    ../../parser/mpOprtBinCommon.cpp \
    ../../parser/mpOprtBinAssign.cpp \
    ../../parser/mpIValue.cpp \
    ../../parser/mpIValReader.cpp \
    ../../parser/mpIToken.cpp \
    ../../parser/mpIPackage.cpp \
    ../../parser/mpIOprt.cpp \
    ../../parser/mpIfThenElse.cpp \
    ../../parser/mpICallback.cpp \
    ../../parser/mpFuncStr.cpp \
    ../../parser/mpFuncNonCmplx.cpp \
    ../../parser/mpFuncMatrix.cpp \
    ../../parser/mpFuncCommon.cpp \
    ../../parser/mpFuncCmplx.cpp \
    ../../parser/mpError.cpp \
    ../../sample/timer.cpp \
    ../../sample/example.cpp

OTHER_FILES +=

HEADERS += \
    ../../parser/utGeneric.h \
    ../../parser/suSortPred.h \
    ../../parser/mpVariable.h \
    ../../parser/mpValueCache.h \
    ../../parser/mpValue.h \
    ../../parser/mpValReader.h \
    ../../parser/mpTypes.h \
    ../../parser/mpTokenReader.h \
    ../../parser/mpTest.h \
    ../../parser/mpStack.h \
    ../../parser/mpScriptTokens.h \
    ../../parser/mpRPN.h \
    ../../parser/mpParserBase.h \
    ../../parser/mpParser.h \
    ../../parser/mpParserMessageProvider.h \
    ../../parser/mpPackageUnit.h \
    ../../parser/mpPackageStr.h \
    ../../parser/mpPackageNonCmplx.h \
    ../../parser/mpPackageMatrix.h \
    ../../parser/mpPackageCommon.h \
    ../../parser/mpPackageCmplx.h \
    ../../parser/mpOprtNonCmplx.h \
    ../../parser/mpOprtMatrix.h \
    ../../parser/mpOprtIndex.h \
    ../../parser/mpOprtCmplx.h \
    ../../parser/mpOprtPostfixCommon.h \
    ../../parser/mpOprtBinCommon.h \
    ../../parser/mpOprtBinAssign.h \
    ../../parser/mpMatrixError.h \
    ../../parser/mpMatrix.h \
    ../../parser/mpIValue.h \
    ../../parser/mpIValReader.h \
    ../../parser/mpIToken.h \
    ../../parser/mpIPrecedence.h \
    ../../parser/mpIPackage.h \
    ../../parser/mpIOprt.h \
    ../../parser/mpIfThenElse.h \
    ../../parser/mpICallback.h \
    ../../parser/mpFwdDecl.h \
    ../../parser/mpFuncStr.h \
    ../../parser/mpFuncNonCmplx.h \
    ../../parser/mpFuncMatrix.h \
    ../../parser/mpFuncCommon.h \
    ../../parser/mpFuncCmplx.h \
    ../../parser/mpError.h \
    ../../parser/mpDefines.h \
    ../../sample/timer.h

INCLUDEPATH += ../../parser
