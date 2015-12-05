// Copyright (c) 2014-2015 Tony Kirke
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstdint>
#include <iostream>
#include <complex>
#include <algorithm> //min/max

/***********************************************************************
 * |PothosDoc Comparator
 *
 * Perform a comparision between 2 inputs and outputs a char value of 1 or 0
 *
 * out[n] = (in0[n] $op in1[n]) ? 1 : 0;
 *
 * |category /Math
 * |keywords math logic comparator
 *
 * |param dtype[Data Type] The data type used in the arithmetic.
 * |widget DTypeChooser(float=1,int=1)
 * |default "float64"
 * |preview disable
 *
 * |param comparator The comparison operation to perform
 * |default ">"
 * |option [>] ">"
 * |option [<] "<"
 * |option [>=] ">="
 * |option [<=] "<="
 * |option [==] "=="
 * |option [!=] "!="
 *
 * |factory /comms/comparator(dtype,comparator)
 **********************************************************************/
template <typename Type, void (*Operator)(const Type *, const Type *, char *, const size_t)>
class Comparator : public Pothos::Block
{
public:
  Comparator(void) {
    typedef Comparator<Type, Operator> ClassType;
    this->setupInput(0, typeid(Type));
    this->setupInput(1, typeid(Type));
    this->setupOutput(0, typeid(char));
  }

    void work(void)
    {
        //number of elements to work with
        auto elems = this->workInfo().minElements;
        if (elems == 0) return;

        //get pointers to in and out buffer
        auto inPort = this->input(0);
        auto outPort = this->output(0);
        const std::vector<Pothos::InputPort *> &inputs = this->inputs();
        auto in0 = inputs[0]->buffer().as<const Type *>();
        auto in1 = inputs[1]->buffer().as<const Type *>();
        auto out = outPort->buffer().template as<char *>();

        //perform operation
        Operator(in0, in1, out, elems);

        //produce and consume on 0th ports
        inPort->consume(elems);
        outPort->produce(elems);
    }
};
/**********************************************************************/
template <typename Type>
void greaterThan(const Type *in0, const Type *in1, char *out, const size_t num)
{
    for (size_t i = 0; i < num; i++) out[i] = in0[i] > in1[i];
}

template <typename Type>
void lessThan(const Type *in0, const Type *in1, char *out, const size_t num)
{
    for (size_t i = 0; i < num; i++) out[i] = in0[i] < in1[i];
}

template <typename Type>
void greaterOrEqualTo(const Type *in0, const Type *in1, char *out, const size_t num)
{
    for (size_t i = 0; i < num; i++) out[i] = in0[i] >= in1[i];
}

template <typename Type>
void lessThanOrEqualTo(const Type *in0, const Type *in1, char *out, const size_t num)
{
    for (size_t i = 0; i < num; i++) out[i] = in0[i] <= in1[i];
}
template <typename Type>
void equalTo(const Type *in0, const Type *in1, char *out, const size_t num)
{
    for (size_t i = 0; i < num; i++) out[i] = in0[i] == in1[i];
}
template <typename Type>
void notEqualTo(const Type *in0, const Type *in1, char *out, const size_t num)
{
    for (size_t i = 0; i < num; i++) out[i] = in0[i] != in1[i];
}

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *comparatorFactory(const Pothos::DType &dtype, const std::string &operation)
{
    #define ifTypeDeclareFactory__(type, opKey, opVal) \
        if (dtype == Pothos::DType(typeid(type)) and operation == opKey) return new Comparator<type, opVal<type>>();
    #define ifTypeDeclareFactory(type) \
        ifTypeDeclareFactory__(type, ">", greaterThan) \
        ifTypeDeclareFactory__(type, "<", lessThan) \
        ifTypeDeclareFactory__(type, ">=", greaterOrEqualTo) \
        ifTypeDeclareFactory__(type, "<=", lessThanOrEqualTo) \
        ifTypeDeclareFactory__(type, "==", equalTo) \
        ifTypeDeclareFactory__(type, "!=", notEqualTo)
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    ifTypeDeclareFactory(int64_t);
    ifTypeDeclareFactory(int32_t);
    ifTypeDeclareFactory(int16_t);
    ifTypeDeclareFactory(int8_t);
    throw Pothos::InvalidArgumentException("Comparator("+dtype.toString()+", "+operation+")", "unsupported args");
}

static Pothos::BlockRegistry registerComparator(
    "/comms/comparator", &comparatorFactory);

