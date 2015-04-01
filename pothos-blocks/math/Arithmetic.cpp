// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstdint>
#include <iostream>
#include <complex>
#include <vector>
#include <cstring> //memset
#include <algorithm> //min/max

/***********************************************************************
 * |PothosDoc Arithmetic
 *
 * Perform arithmetic operations on elements across multiple input ports to produce a stream of outputs.
 *
 * out[n] = in0[n] $op in1[n] $op ... $op in_last[n]
 *
 * |category /Math
 * |keywords math arithmetic add subtract multiply divide
 *
 * |param dtype[Data Type] The data type used in the arithmetic.
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param operation The mathematical operation to perform
 * |default "ADD"
 * |option [Add] "ADD"
 * |option [Subtract] "SUB"
 * |option [Multiply] "MUL"
 * |option [Divide] "DIV"
 *
 * |param numInputs[Num Inputs] The number of input ports.
 * |default 2
 * |widget SpinBox(minimum=2)
 * |preview disable
 *
 * |param preload The number of elements to preload into each input.
 * The value is an array of integers where each element represents
 * the number of elements to preload the port with.
 * |default []
 * |widget ComboBox(editable=true)
 * |option [Ignored] \[\]
 * |preview disable
 *
 * |factory /blocks/arithmetic(dtype, operation)
 * |initializer setNumInputs(numInputs)
 * |initializer setPreload(preload)
 **********************************************************************/
template <typename Type, void (*Operator)(const Type *, const Type *, Type *, const size_t)>
class Arithmetic : public Pothos::Block
{
public:
    Arithmetic(void):
        _numInlineBuffers(0)
    {
        typedef Arithmetic<Type, Operator> ClassType;
        this->registerCall(this, POTHOS_FCN_TUPLE(ClassType, setNumInputs));
        this->registerCall(this, POTHOS_FCN_TUPLE(ClassType, setPreload));
        this->registerCall(this, POTHOS_FCN_TUPLE(ClassType, preload));
        this->registerCall(this, POTHOS_FCN_TUPLE(ClassType, getNumInlineBuffers));
        this->setupInput(0, typeid(Type));
        this->setupOutput(0, typeid(Type), this->uid()); //unique domain because of inline buffer forwarding

        //read before write optimization
        this->output(0)->setReadBeforeWrite(this->input(0));
    }

    void setNumInputs(const size_t numInputs)
    {
        if (numInputs < 2) throw Pothos::RangeException("Arithmetic::setNumInputs("+std::to_string(numInputs)+")", "require inputs >= 2");
        for (size_t i = this->inputs().size(); i < numInputs; i++)
        {
            this->setupInput(i, this->input(0)->dtype());
        }
    }

    void setPreload(const std::vector<size_t> &preload)
    {
        this->setNumInputs(std::max<size_t>(2, preload.size()));
        _preload = preload;
    }

    const std::vector<size_t> &preload(void) const
    {
        return _preload;
    }

    void activate(void)
    {
        for (size_t i = 0; i < _preload.size(); i++)
        {
            auto bytes = _preload[i]*this->input(i)->dtype().size();
            if (bytes == 0) continue;
            Pothos::BufferChunk buffer(bytes);
            std::memset(buffer.as<void *>(), 0, buffer.length);
            this->input(i)->clear();
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
            auto *inX = inputs[i]->buffer().as<const Type *>();
            Operator(in0, inX, out, elems);
            in0 = out; //operate on output array next loop
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
    std::vector<size_t> _preload;
};

/***********************************************************************
 * templated arithmetic vector operators
 **********************************************************************/
template <typename Type>
void addArray(const Type *in0, const Type *in1, Type *out, const size_t num)
{
    for (size_t i = 0; i < num; i++) out[i] = in0[i] + in1[i];
}

template <typename Type>
void subArray(const Type *in0, const Type *in1, Type *out, const size_t num)
{
    for (size_t i = 0; i < num; i++) out[i] = in0[i] - in1[i];
}

template <typename Type>
void mulArray(const Type *in0, const Type *in1, Type *out, const size_t num)
{
    for (size_t i = 0; i < num; i++) out[i] = in0[i] * in1[i];
}

template <typename Type>
void divArray(const Type *in0, const Type *in1, Type *out, const size_t num)
{
    for (size_t i = 0; i < num; i++) out[i] = in0[i] / in1[i];
}

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *arithmeticFactory(const Pothos::DType &dtype, const std::string &operation)
{
    #define ifTypeDeclareFactory__(type, opKey, opVal) \
        if (dtype == Pothos::DType(typeid(type)) and operation == opKey) return new Arithmetic<type, opVal<type>>();
    #define ifTypeDeclareFactory_(type) \
        ifTypeDeclareFactory__(type, "ADD", addArray) \
        ifTypeDeclareFactory__(type, "SUB", subArray) \
        ifTypeDeclareFactory__(type, "MUL", mulArray) \
        ifTypeDeclareFactory__(type, "DIV", divArray)
    #define ifTypeDeclareFactory(type) \
        ifTypeDeclareFactory_(type) \
        ifTypeDeclareFactory_(std::complex<type>)
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    ifTypeDeclareFactory(int64_t);
    ifTypeDeclareFactory(int32_t);
    ifTypeDeclareFactory(int16_t);
    ifTypeDeclareFactory(int8_t);
    throw Pothos::InvalidArgumentException("arithmeticFactory("+dtype.toString()+", "+operation+")", "unsupported args");
}

static Pothos::BlockRegistry registerArithmetic(
    "/blocks/arithmetic", &arithmeticFactory);
