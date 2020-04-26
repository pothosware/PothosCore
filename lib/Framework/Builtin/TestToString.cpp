// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Object.hpp>
#include <Pothos/Framework.hpp>

#include <Poco/Format.h>
#include <Poco/NumberFormatter.h>

#include <complex>
#include <sstream>
#include <vector>

struct BufferChunkTestParams
{
    std::string dtype;
    std::string expectedDTypeStr;
    size_t elems;
};

static void testTypedBufferChunkToString(const BufferChunkTestParams& testParams)
{
    Pothos::BufferChunk bufferChunk(Pothos::DType(testParams.dtype), testParams.elems);

    const auto expectedToString = Poco::format("Pothos::BufferChunk (dtype: %s, elements: %s)",
                                               testParams.expectedDTypeStr,
                                               Poco::NumberFormatter::format(testParams.elems));

    POTHOS_TEST_EQUAL(expectedToString, Pothos::Object(bufferChunk).toString());
}

POTHOS_TEST_BLOCK("/framework/tests", test_bufferchunk_to_string)
{
    POTHOS_TEST_EQUAL(
        "Pothos::BufferChunk (dtype: unspecified, elements: 0)",
        Pothos::Object(Pothos::BufferChunk()).toString());

    const std::vector<BufferChunkTestParams> allTestParams =
    {
        {"int8",   "int8",   10},
        {"int16",  "int16",  20},
        {"int32",  "int32",  30},
        {"int64",  "int64",  40},
        {"uint8",  "uint8",  50},
        {"uint16", "uint16", 60},
        {"uint32", "uint32", 70},
        {"uint64", "uint64", 80},

        {"complex_int8",   "complex_int8",   90},
        {"complex_int16",  "complex_int16",  100},
        {"complex_int32",  "complex_int32",  110},
        {"complex_int64",  "complex_int64",  120},
        {"complex_uint8",  "complex_uint8",  130},
        {"complex_uint16", "complex_uint16", 140},
        {"complex_uint32", "complex_uint32", 150},
        {"complex_uint64", "complex_uint64", 160},

        {"int8, 2",   "int8[2]",   10},
        {"int16, 2",  "int16[2]",  20},
        {"int32, 2",  "int32[2]",  30},
        {"int64, 2",  "int64[2]",  40},
        {"uint8, 2",  "uint8[2]",  50},
        {"uint16, 2", "uint16[2]", 60},
        {"uint32, 2", "uint32[2]", 70},
        {"uint64, 2", "uint64[2]", 80},

        {"complex_int8, 2",   "complex_int8[2]",   90},
        {"complex_int16, 2",  "complex_int16[2]",  100},
        {"complex_int32, 2",  "complex_int32[2]",  110},
        {"complex_int64, 2",  "complex_int64[2]",  120},
        {"complex_uint8, 2",  "complex_uint8[2]",  130},
        {"complex_uint16, 2", "complex_uint16[2]", 140},
        {"complex_uint32, 2", "complex_uint32[2]", 150},
        {"complex_uint64, 2", "complex_uint64[2]", 160},

        {"custom", "custom[1]", 170}
    };
    for(const auto& testParams: allTestParams)
    {
        testTypedBufferChunkToString(testParams);
    }
}

static inline void testDTypeToString(const std::string& expectedStr, const Pothos::DType& dtype)
{
    POTHOS_TEST_EQUAL(expectedStr, Pothos::Object(dtype).toString());
}

static inline void testDTypeToString(const std::string& dtypeStr, size_t dimension)
{
    const std::string expectedStr = dtypeStr + "[" + std::to_string(dimension) + "]";
    testDTypeToString(expectedStr, Pothos::DType(dtypeStr, dimension));
}

static inline void testDTypeToString(const std::string& dtypeStr)
{
    testDTypeToString(dtypeStr, Pothos::DType(dtypeStr));
}

POTHOS_TEST_BLOCK("/framework/tests", test_dtype_to_string)
{
    testDTypeToString("unspecified", Pothos::DType());
    testDTypeToString("custom[1]", Pothos::DType("custom"));

    testDTypeToString("int8");
    testDTypeToString("int16");
    testDTypeToString("int32");
    testDTypeToString("int64");
    testDTypeToString("uint8");
    testDTypeToString("uint16");
    testDTypeToString("uint32");
    testDTypeToString("uint64");
    testDTypeToString("float32");
    testDTypeToString("float64");
    testDTypeToString("complex_int8");
    testDTypeToString("complex_int16");
    testDTypeToString("complex_int32");
    testDTypeToString("complex_int64");
    testDTypeToString("complex_uint8");
    testDTypeToString("complex_uint16");
    testDTypeToString("complex_uint32");
    testDTypeToString("complex_uint64");
    testDTypeToString("complex_float32");
    testDTypeToString("complex_float64");

    testDTypeToString("int8", 2);
    testDTypeToString("int16", 2);
    testDTypeToString("int32", 2);
    testDTypeToString("int64", 2);
    testDTypeToString("uint8", 2);
    testDTypeToString("uint16", 2);
    testDTypeToString("uint32", 2);
    testDTypeToString("uint64", 2);
    testDTypeToString("float32", 2);
    testDTypeToString("float64", 2);
    testDTypeToString("complex_int8", 2);
    testDTypeToString("complex_int16", 2);
    testDTypeToString("complex_int32", 2);
    testDTypeToString("complex_int64", 2);
    testDTypeToString("complex_uint8", 2);
    testDTypeToString("complex_uint16", 2);
    testDTypeToString("complex_uint32", 2);
    testDTypeToString("complex_uint64", 2);
    testDTypeToString("complex_float32", 2);
    testDTypeToString("complex_float64", 2);
}

static void testPortBufferToString(
    const Pothos::Proxy& blockProxy,
    const std::string& getPortFunc)
{
    auto portProxy = blockProxy.call(getPortFunc, 0);

    const auto elements = portProxy.call<size_t>("elements");
    POTHOS_TEST_EQUAL(
        Poco::format(
            "Pothos::BufferChunk (dtype: %s, elements: %s)",
            portProxy.call("dtype").call<std::string>("name"),
            Poco::NumberFormatter::format(elements)),
        portProxy.call("buffer").toString());
}

POTHOS_TEST_BLOCK("/framework/tests", test_topology_classes_to_string)
{
    auto topologyObj = Pothos::Object::emplace<Pothos::Topology>();
    auto& topologyRef = topologyObj.ref<Pothos::Topology>();
    topologyRef.setName("Test Topology");

    POTHOS_TEST_EQUAL(
        "Pothos::Topology (name: Test Topology)",
        topologyObj.toString());

    // A block that has both an input port and output port
    const auto blockPath = "/blocks/sporadic_labeler";
    auto blockProxy = Pothos::BlockRegistry::make(blockPath);

    // Note: the managed proxy handle calls into Pothos::Object::toString(), so
    // testing against Pothos::Proxy::toString() for topology types is equivalent.
    POTHOS_TEST_EQUAL(
        std::string("Shared pointer: Pothos::Block (name: ") + blockPath + ")",
        blockProxy.toString());
    POTHOS_TEST_EQUAL(
        "Pointer: Pothos::InputPort (alias: 0, dtype: unspecified)",
        blockProxy.call("input", 0).toString());
    POTHOS_TEST_EQUAL(
        "Pointer: Pothos::OutputPort (alias: 0, dtype: unspecified)",
        blockProxy.call("output", 0).toString());
    testPortBufferToString(blockProxy, "input");
    testPortBufferToString(blockProxy, "output");
}
