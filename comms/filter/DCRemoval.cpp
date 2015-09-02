// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MovingAverage.hpp"
#include <Pothos/Framework.hpp>
#include <cstdint>
#include <complex>
#include <iostream>

/***********************************************************************
 * |PothosDoc DC Removal
 *
 * The DC removal block calculates the DC level of input port 0
 * and outputs the stream minus the DC level to output port 0.
 *
 * The DC level is calculated using a moving average algorithm:<br />
 * <a href="http://www.digitalsignallabs.com/dcblock.pdf">
 * http://www.digitalsignallabs.com/dcblock.pdf</a>
 *
 * |category /Filter
 * |keywords filter dc blocker bias
 *
 * |param dtype[Data Type] The data type of the input and output element stream.
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param averageSize[Average Size] The depth of the moving average in number of samples.
 * This parameter configures the depth of the history used in the moving average delay-line.
 * |default 512
 * |units samples
 *
 * |param cascadeSize[Cascade Size] The number of moving average filters to cascade.
 * Increasing this parameter will narrow the transition region at the expense of increased computation.
 * |default 2
 * |units filters
 *
 * |factory /comms/dc_removal(dtype)
 * |setter setAverageSize(averageSize)
 * |setter setCascadeSize(cascadeSize)
 **********************************************************************/
template <typename Type, typename AccType>
class DCRemoval : public Pothos::Block
{
public:
    DCRemoval(void):
        _averageSize(0),
        _cascadeSize(0)
    {
        this->setupInput(0, typeid(Type));
        this->setupOutput(0, typeid(Type));
        this->registerCall(this, POTHOS_FCN_TUPLE(DCRemoval, setAverageSize));
        this->registerCall(this, POTHOS_FCN_TUPLE(DCRemoval, getAverageSize));
        this->registerCall(this, POTHOS_FCN_TUPLE(DCRemoval, setCascadeSize));
        this->registerCall(this, POTHOS_FCN_TUPLE(DCRemoval, getCascadeSize));
        this->setAverageSize(512); //initial state
        this->setCascadeSize(2); //initial state
    }

    void setAverageSize(const size_t num)
    {
        if (num == 0) throw Pothos::InvalidArgumentException("DCRemoval::setAverageSize()", "average size cannot be zero");
        _averageSize = num;
        this->resetFilters();
    }

    size_t getAverageSize(void) const
    {
        return _averageSize;
    }

    void setCascadeSize(const size_t num)
    {
        if (num == 0) throw Pothos::InvalidArgumentException("DCRemoval::setCascadeSize()", "cascade size cannot be zero");
        _cascadeSize = num;
        this->resetFilters();
    }

    size_t getCascadeSize(void) const
    {
        return _cascadeSize;
    }

    void activate(void)
    {
        this->resetFilters();
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        const size_t N = this->workInfo().minElements;

        //cast the input and output buffers
        const auto in = inPort->buffer().template as<const Type *>();
        const auto out = outPort->buffer().template as<Type *>();

        //remove DC from each point
        for (size_t i = 0; i < N; i++)
        {
            auto x_i = in[i];

            //advance the filters
            for (auto &f : _filters) x_i = Type(f(x_i));

            //output delayed input minus the dc level
            out[i] = Type(_filters[0].front()) - x_i;
        }

        inPort->consume(N);
        outPort->produce(N);
    }

private:

    void resetFilters(void)
    {
        _filters.resize(_cascadeSize);
        for (auto &f : _filters) f.resize(_averageSize);
    }

    size_t _averageSize;
    size_t _cascadeSize;
    std::vector<MovingAverage<AccType>> _filters;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *DCRemovalFactory(const Pothos::DType &dtype)
{
    #define ifTypeDeclareFactory__(Type, AccType) \
        if (dtype == Pothos::DType(typeid(Type))) return new DCRemoval<Type, AccType>();
    #define ifTypeDeclareFactory(Type, AccType) \
        ifTypeDeclareFactory__(Type, AccType) \
        ifTypeDeclareFactory__(std::complex<Type>, std::complex<AccType>)
    ifTypeDeclareFactory(double, double);
    ifTypeDeclareFactory(float, float);
    ifTypeDeclareFactory(int64_t, int64_t);
    ifTypeDeclareFactory(int32_t, int64_t);
    ifTypeDeclareFactory(int16_t, int32_t);
    ifTypeDeclareFactory(int8_t, int16_t);
    throw Pothos::InvalidArgumentException("DCRemovalFactory("+dtype.toString()+")", "unsupported type");
}
static Pothos::BlockRegistry registerDCRemoval(
    "/comms/dc_removal", &DCRemovalFactory);
