// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Util/MathCompat.hpp>
#include <cstdint>
#include <iostream>
#include <complex>
#include <vector>
#include <cmath> //log2
#include <algorithm> //min/max

/***********************************************************************
 * |PothosDoc Symbol Mapper
 *
 * Map a stream of symbols to a user-specified value.
 * Each input element is a byte that contains an offset into the symbol map.
 *
 * out[n] = map[in0[n]]
 *
 * |category /Digital
 * |category /Symbol
 * |keywords map symbol mapper
 * |alias /blocks/symbol_mapper
 *
 * |param dtype[Data Type] The output data type produced by the mapper.
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param map[Symbol Map] The symbol map is a list of possible output values
 * which can be anything supported by the output data type.
 * Example: a list of integers to encode/decode the input stream,
 * or complex constellation points for MPSK modulations.
 * This must be a power-of-two in length; e.g. 2, 4, 8...
 * |default [-1, 1]
 * |option [BPSK] \[-1, 1\]
 * |option [QPSK] \[-1.0-1.0*j, -1.0+1.0*j, 1.0+1.0*j, 1.0-1.0*j\]
 * |option [2-bit Gray Code] \[0, 1, 3, 2\]
 * |option [3-bit Gray Code] \[0, 1, 3, 2, 6, 7, 5, 4\]
 * |option [4-bit Gray Code] \[0, 1, 3, 2, 6, 7, 5, 4, 12, 13, 15, 14, 10, 11, 9, 8\]
 * |widget ComboBox(editable=true)
 *
 * |factory /comms/symbol_mapper(dtype)
 * |setter setMap(map)
 **********************************************************************/
template <typename OutType>
class SymbolMapper : public Pothos::Block
{
public:
    SymbolMapper(void)
    {
        _map = std::vector<OutType>();
        _nbits = 0;
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(OutType));
        this->registerCall(this, POTHOS_FCN_TUPLE(SymbolMapper, getMap));
        this->registerCall(this, POTHOS_FCN_TUPLE(SymbolMapper, setMap));
        this->setMap(std::vector<OutType>(1, OutType(1))); //prob unnecessary
    }

    std::vector<OutType> getMap(void) const
    {
        return _map;
    }

    void setMap(const std::vector<OutType> &map)
    {
        if(map.size() == 0) throw Pothos::InvalidArgumentException("SymbolMapper::setMap()", "Map must be nonzero size");
        auto nbits = std::log2(map.size());
        if(nbits != int(nbits))
        {
            throw Pothos::InvalidArgumentException("SymbolMapper::setMap()", "Map must be a power of two in length");
        }
        _map = map;
        _nbits = nbits;
        _mask = (1<<_nbits)-1;
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        auto in = inPort->buffer().template as<const unsigned char *>();
        auto out = outPort->buffer().template as<OutType *>();

        unsigned int N = std::min(inPort->elements(), outPort->elements());

        for(unsigned int i=0; i<N; i++) {
            out[i] = _map[in[i]&_mask];
        }

        inPort->consume(N);
        outPort->produce(N);
    }

private:
    std::vector<OutType> _map;
    unsigned int _nbits;
    unsigned char _mask;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *SymbolMapperFactory(const Pothos::DType &dtype)
{
    #define ifTypeDeclareFactory(type) \
        if (dtype == Pothos::DType(typeid(type))) \
            return new SymbolMapper<type>(); \
        if (dtype == Pothos::DType(typeid(std::complex<type>))) \
            return new SymbolMapper<std::complex<type>>();
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    ifTypeDeclareFactory(int64_t);
    ifTypeDeclareFactory(int32_t);
    ifTypeDeclareFactory(int16_t);
    ifTypeDeclareFactory(int8_t);
    throw Pothos::InvalidArgumentException("SymbolMapperFactory("+dtype.toString()+")", "unsupported type");
}

static Pothos::BlockRegistry registerSymbolMapper(
    "/comms/symbol_mapper", &SymbolMapperFactory);

static Pothos::BlockRegistry registerSymbolMapperOldPath(
    "/blocks/symbol_mapper", &SymbolMapperFactory);

