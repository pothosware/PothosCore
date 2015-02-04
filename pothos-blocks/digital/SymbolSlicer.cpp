// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstdint>
#include <iostream>
#include <complex>
#include <vector>
#include <cfloat> //FLT_MAX

/***********************************************************************
 * |PothosDoc Symbol Slicer
 *
 * Slice an incoming stream of elements into binary symbols using Euclidean distance.
 * The output is the symbol index of the closest value in the map.
 *
 * This slicer is O(len(map)) and suboptimal for simple (BPSK, QPSK) constellations, but
 * has the advantage that it will work for any arbitrary constellation.
 *
 * |category /Digital
 * |category /Symbol
 * |keywords symbol slicer
 *
 * |param dtype[Data Type] The input data type consumed by the slicer.
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param map[Symbol Map] The symbol map is a list of arbitrary slicer values
 * which can be anything supported by the input data type.
 * |default [-1, 1]
 *
 * |factory /blocks/symbol_slicer(dtype)
 * |setter setMap(map)
 **********************************************************************/

template <typename T>
float euclidDist(T inA, T inB)
{
    return std::abs(inB - inA);
}

template <typename T>
float euclidDist(std::complex<T> inA, std::complex<T> inB)
{
    return powf(inB.real()-inA.real(), 2) + powf(inB.imag()-inA.imag(), 2);
}

template <typename InType>
class SymbolSlicer : public Pothos::Block
{
public:
    SymbolSlicer(void)
    {
        this->setupInput(0, typeid(InType));
        this->setupOutput(0, typeid(unsigned char));
        this->registerCall(this, POTHOS_FCN_TUPLE(SymbolSlicer, getMap));
        this->registerCall(this, POTHOS_FCN_TUPLE(SymbolSlicer, setMap));
        this->setMap(std::vector<InType>(1, InType(1))); //prob unnecessary
    }

    std::vector<InType> getMap(void) const
    {
        return _map;
    }

    void setMap(const std::vector<InType> &map)
    {
        if(map.size() == 0) throw Pothos::InvalidArgumentException("SymbolSlicer::setMap()", "Map must be nonzero size");
        _map = map;
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        auto in  = inPort-> buffer().template as<const InType *>();
        auto out = outPort->buffer().template as<unsigned char *>();

        unsigned int N = std::min(inPort->elements(), outPort->elements());

        for(unsigned int i=0; i<N; i++) {
            std::pair<unsigned char, float> mindist = std::make_pair(0, FLT_MAX);
            for(unsigned int j=0; j<_map.size(); j++) {
                float dist = euclidDist(in[i], _map[j]);
                if(dist < mindist.second)
                {
                    mindist = std::make_pair(j, dist);
                }
            }
            out[i] = mindist.first;
        }

        inPort->consume(N);
        outPort->produce(N);
    }

private:
    std::vector<InType> _map;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *SymbolSlicerFactory(const Pothos::DType &dtype)
{
    #define ifTypeDeclareFactory(type) \
        if (dtype == Pothos::DType(typeid(type))) \
            return new SymbolSlicer<type>(); \
        if (dtype == Pothos::DType(typeid(std::complex<type>))) \
            return new SymbolSlicer<std::complex<type>>();
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    ifTypeDeclareFactory(int64_t);
    ifTypeDeclareFactory(int32_t);
    ifTypeDeclareFactory(int16_t);
    ifTypeDeclareFactory(int8_t);
    throw Pothos::InvalidArgumentException("SymbolSlicerFactory("+dtype.toString()+")", "unsupported type");
}

static Pothos::BlockRegistry registerSymbolSlicer(
    "/blocks/symbol_slicer", &SymbolSlicerFactory);

