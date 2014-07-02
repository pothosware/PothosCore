// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Poco/Types.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>
#include <iostream>
#include <complex>
#include <cstring> //memset

/***********************************************************************
 * |PothosDoc Add
 *
 * Add elements across multiple input ports to produce a stream of outputs.
 *
 * out[n] = in0[n] + in1[n] + ... + in_last[n]
 *
 * |category /Arithmetic
 * |keywords math add arithmetic
 *
 * |param dtype[Data Type] The datatype used in the arithmetic.
 * |option [Complex128] "complex128"
 * |option [Float64] "float64"
 * |option [Complex64] "complex64"
 * |option [Float32] "float32"
 * |option [Complex Int64] "complex_int64"
 * |option [Int64] "int64"
 * |option [Complex Int32] "complex_int32"
 * |option [Int32] "int32"
 * |option [Complex Int16] "complex_int16"
 * |option [Int16] "int16"
 * |option [Complex Int8] "complex_int8"
 * |option [Int8] "int8"
 * |preview disable
 *
 * |param preload The number of elements to preload into each input.
 * The value is a JSON array of integers where each element represents
 * the number of elements to preload the port with.
 * |default "[0, 0]"
 * |preview disable
 *
 * |factory /blocks/arithmetic/add(dtype)
 * |setter setPreload(preload)
 **********************************************************************/
template <typename Type>
class Add : public Pothos::Block
{
public:
    Add(void):
        _numInlineBuffers(0)
    {
        this->registerCall(POTHOS_FCN_TUPLE(Add<Type>, setPreload));
        this->registerCall(POTHOS_FCN_TUPLE(Add<Type>, getNumInlineBuffers));
        this->setupInput(0, typeid(Type));
        this->setupInput(1, typeid(Type)); //TODO FIXME remove this line after: https://github.com/pothosware/pothos-library/issues/42
        this->setupOutput(0, typeid(Type));

        //read before write optimization
        this->output(0)->setReadBeforeWrite(this->input(0));
    }

    void setPreload(const std::string &preloadStr)
    {
        Poco::JSON::Parser p; p.parse(preloadStr);
        const auto ports = p.getHandler()->asVar().extract<Poco::JSON::Array::Ptr>();
        for (size_t i = 0; i < ports->size(); i++)
        {
            auto bytes = ports->getElement<int>(i)*this->input(i)->dtype().size();
            Pothos::BufferChunk buffer(bytes);
            std::memset(buffer.as<void *>(), 0, buffer.length);
            this->input(i)->pushBuffer(buffer);
        }
    }

    void work(void)
    {
        //number of elements to work with
        auto elems = this->workInfo().minElements;
        if (elems == 0) return;

        //access to input ports and output port
        const std::vector<Pothos::InputPort *> &inputs = this->inputs();
        Pothos::OutputPort *output = this->output(0);

        //establish pointers to buffers
        auto out = Pothos::BufferChunk(output->buffer()).as<Type *>();
        auto in0 = inputs[0]->buffer().as<const Type *>();
        if (out == in0) _numInlineBuffers++; //track buffer inlining

        //loop through available ports
        for (size_t i = 1; i < inputs.size(); i++)
        {
            auto *inX = Pothos::BufferChunk(inputs[i]->buffer()).as<const Type *>();
            for (size_t n = 0; n < elems; n++) //loop through elements
            {
                out[n] = inX[n] + in0[n];
            }
            in0 = out; //adding to output array next loop
            inputs[i]->consume(elems); //consume on ith input port
        }

        //produce and consume on 0th ports
        inputs[0]->consume(elems);
        output->produce(elems);
    }

    size_t getNumInlineBuffers(void) const
    {
        return _numInlineBuffers;
    }

private:
    size_t _numInlineBuffers;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *addFactory(const Pothos::DType &dtype)
{
    #define ifTypeDeclareFactory(type) \
        if (dtype == Pothos::DType(typeid(type))) return new Add<type>(); \
        if (dtype == Pothos::DType(typeid(std::complex<type>))) return new Add<std::complex<type>>();
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    ifTypeDeclareFactory(Poco::Int64);
    ifTypeDeclareFactory(Poco::Int32);
    ifTypeDeclareFactory(Poco::Int16);
    ifTypeDeclareFactory(Poco::Int8);
    throw Pothos::InvalidArgumentException("addFactory("+dtype.toString()+")", "unsupported type");
}

static Pothos::BlockRegistry registerAdd(
    "/blocks/arithmetic/add", &addFactory);
