// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Util/MathCompat.hpp>
#include <Poco/Types.h>
#include <iostream>
#include <complex>
#include "RandomUtils.hpp"

static const size_t waveTableSize = 4096;

/***********************************************************************
 * |PothosDoc Noise Source
 *
 * The noise source produces pseudorandom noise patterns.
 * When a complex data type is chosen, the real and imaginary
 * components are simply treated as two independent channels.
 *
 * |category /Sources
 * |category /Waveforms
 * |keywords noise random source pseudorandom gaussian
 *
 * |param dtype[Data Type] The datatype produced by the noise source.
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
 * |param seed A seed value for the random number generators.
 * |default 42
 *
 * |param wave[Wave Type] The type of the pseudorandom noise produced.
 * |option [Uniform] "UNIFORM"
 * |option [Gaussian] "GAUSSIAN"
 * |option [Laplacian] "LAPLACIAN"
 * |option [Impluse] "IMPULSE"
 * |default "GAUSSIAN"
 *
 * |param ampl[Amplitude] A constant scalar representing the amplitude.
 * |default 1.0
 *
 * |param offset A constant value added to the waveform after scaling.
 * |default 0.0
 *
 * |param factor A factor for the impulse noise configuration.
 * |default 9.0
 *
 * |factory /blocks/noise_source(dtype, seed)
 * |setter setWaveform(wave)
 * |setter setOffset(offset)
 * |setter setAmplitude(ampl)
 * |setter setFactor(factor)
 **********************************************************************/
template <typename Type>
class NoiseSource : public Pothos::Block
{
public:
    NoiseSource(const long seed):
        _index(0),
        _table(waveTableSize),
        _offset(0.0),
        _scalar(1.0),
        _factor(9.0),
        _wave("GAUSSIAN"),
        _random(seed)
    {
        this->setupOutput(0, typeid(Type));
        this->registerCall(POTHOS_FCN_TUPLE(NoiseSource, setWaveform));
        this->registerCall(POTHOS_FCN_TUPLE(NoiseSource, getWaveform));
        this->registerCall(POTHOS_FCN_TUPLE(NoiseSource, setOffset));
        this->registerCall(POTHOS_FCN_TUPLE(NoiseSource, getOffset));
        this->registerCall(POTHOS_FCN_TUPLE(NoiseSource, setAmplitude));
        this->registerCall(POTHOS_FCN_TUPLE(NoiseSource, getAmplitude));
        this->registerCall(POTHOS_FCN_TUPLE(NoiseSource, setFactor));
        this->registerCall(POTHOS_FCN_TUPLE(NoiseSource, getFactor));
    }

    void activate(void)
    {
        this->updateTable();
    }

    void work(void)
    {
        _index += size_t(_random.ran1()*waveTableSize); //lookup into table is random each work()
        auto outPort = this->output(0);
        auto out = outPort->buffer().template as<Type *>();
        for (size_t i = 0; i < outPort->elements(); i++)
        {
            out[i] = _table[_index % waveTableSize];
            _index++;
        }
        outPort->produce(outPort->elements());
    }

    void setWaveform(const std::string &wave)
    {
        _wave = wave;
        this->updateTable();
    }

    std::string getWaveform(void)
    {
        return _wave;
    }

    void setOffset(const std::complex<double> &offset)
    {
        _offset = offset;
        this->updateTable();
    }

    std::complex<double> getOffset(void)
    {
        return _offset;
    }

    void setAmplitude(const std::complex<double> &scalar)
    {
        _scalar = scalar;
        this->updateTable();
    }

    std::complex<double> getAmplitude(void)
    {
        return _scalar;
    }

    void setFactor(const double &factor)
    {
        _factor = factor;
        this->updateTable();
    }

    double getFactor(void)
    {
        return _factor;
    }

private:
    void updateTable(void)
    {
        if (_wave == "UNIFORM")
        {
            for (size_t i = 0; i < _table.size(); i++)
            {
                this->setElem(_table[i], std::complex<double>(2*_random.ran1()-1, 2*_random.ran1()-1));
            }
        }
        else if (_wave == "GAUSSIAN")
        {
            for (size_t i = 0; i < _table.size(); i++)
            {
                this->setElem(_table[i], std::complex<double>(_random.gasdev(), _random.gasdev()));
            }
        }
        else if (_wave == "LAPLACIAN")
        {
            for (size_t i = 0; i < _table.size(); i++)
            {
                this->setElem(_table[i], std::complex<double>(_random.laplacian(), _random.laplacian()));
            }
        }
        else if (_wave == "IMPULSE")
        {
            const float factor = float(_factor);
            for (size_t i = 0; i < _table.size(); i++)
            {
                this->setElem(_table[i], std::complex<double>(_random.impulse(factor), _random.impulse(factor)));
            }
        }
        else throw Pothos::InvalidArgumentException("NoiseSource::setWaveform("+_wave+")", "unknown waveform setting");
    }

    template <typename T>
    void setElem(T &out, const std::complex<double> &val)
    {
        out = Type((_scalar * val + _offset).real());
    }

    template <typename T>
    void setElem(std::complex<T> &out, const std::complex<double> &val)
    {
        out = Type(_scalar * val + _offset);
    }

    size_t _index;
    std::vector<Type> _table;
    std::complex<double> _offset, _scalar;
    double _factor;
    std::string _wave;
    gr_random _random;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *noiseSourceFactory(const Pothos::DType &dtype, const long seed)
{
    #define ifTypeDeclareFactory(type) \
        if (dtype == Pothos::DType(typeid(type))) return new NoiseSource<type>(seed); \
        if (dtype == Pothos::DType(typeid(std::complex<type>))) return new NoiseSource<std::complex<type>>(seed);
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    ifTypeDeclareFactory(Poco::Int64);
    ifTypeDeclareFactory(Poco::Int32);
    ifTypeDeclareFactory(Poco::Int16);
    ifTypeDeclareFactory(Poco::Int8);
    throw Pothos::InvalidArgumentException("noiseSourceFactory("+dtype.toString()+")", "unsupported type");
}

static Pothos::BlockRegistry registerNoiseSource(
    "/blocks/noise_source", &noiseSourceFactory);
