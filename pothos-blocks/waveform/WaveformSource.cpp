// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Util/MathCompat.hpp>
#include <Poco/Types.h>
#include <iostream>
#include <complex>

static const size_t waveTableSize = 4096;

/***********************************************************************
 * |PothosDoc Waveform Source
 *
 * The waveform source produces simple cyclical waveforms.
 * When a complex data type is chosen, the real and imaginary
 * components of the outputs will be 90 degrees out of phase.
 *
 * |category /Sources
 * |category /Waveforms
 * |keywords cosine sine ramp square waveform source
 *
 * |param dtype[Data Type] The datatype produced by the waveform source.
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
 * |param wave[Wave Type] The type of the waveform produced.
 * |option [Constant] "CONST"
 * |option [Sinusoid] "SINE"
 * |option [Ramp] "RAMP"
 * |option [Square] "SQUARE"
 * |default "SINE"
 *
 * |param freq[Frequency] The frequency of the waveform (+/- 0.5).
 * |units cycles/sample
 * |default 0.1
 *
 * |param ampl[Amplitude] A constant scalar representing the amplitude.
 * |default 1.0
 *
 * |param offset A constant value added to the waveform after scaling.
 * |default 0.0
 *
 * |factory /blocks/waveform_source(dtype)
 * |setter setWaveform(wave)
 * |setter setOffset(offset)
 * |setter setAmplitude(ampl)
 * |setter setFrequency(freq)
 **********************************************************************/
template <typename Type>
class WaveformSource : public Pothos::Block
{
public:
    WaveformSource(void):
        _index(0), _step(0),
        _table(waveTableSize),
        _offset(0.0), _scalar(1.0),
        _wave("CONST")
    {
        this->setupOutput(0, typeid(Type));
        this->registerCall(POTHOS_FCN_TUPLE(WaveformSource<Type>, setWaveform));
        this->registerCall(POTHOS_FCN_TUPLE(WaveformSource<Type>, getWaveform));
        this->registerCall(POTHOS_FCN_TUPLE(WaveformSource<Type>, setOffset));
        this->registerCall(POTHOS_FCN_TUPLE(WaveformSource<Type>, getOffset));
        this->registerCall(POTHOS_FCN_TUPLE(WaveformSource<Type>, setAmplitude));
        this->registerCall(POTHOS_FCN_TUPLE(WaveformSource<Type>, getAmplitude));
        this->registerCall(POTHOS_FCN_TUPLE(WaveformSource<Type>, setFrequency));
        this->registerCall(POTHOS_FCN_TUPLE(WaveformSource<Type>, getFrequency));
    }

    void activate(void)
    {
        this->updateTable();
    }

    void work(void)
    {
        auto outPort = this->output(0);
        auto out = outPort->buffer().template as<Type *>();
        for (size_t i = 0; i < outPort->elements(); i++)
        {
            out[i] = _table[_index % waveTableSize];
            _index += _step;
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

    void setFrequency(const double &freq)
    {
        _step = size_t(std::llround(freq*_table.size()));
    }

    double getFrequency(void)
    {
        return double(_step)/_table.size();
    }

private:
    void updateTable(void)
    {
        if (_wave == "CONST")
        {
            for (size_t i = 0; i < _table.size(); i++)
            {
                this->setElem(_table[i], 1.0);
            }
        }
        else if (_wave == "SINE")
        {
            for (size_t i = 0; i < _table.size(); i++){
                this->setElem(_table[i], std::pow(M_E, std::complex<double>(0, 2*M_PI*i/_table.size())));
            }
        }
        else if (_wave == "RAMP")
        {
            for (size_t i = 0; i < _table.size(); i++)
            {
                const size_t q = (i+(3*_table.size())/4)%_table.size();
                this->setElem(_table[i], std::complex<double>(
                    2.0*i/(_table.size()-1) - 1.0,
                    2.0*q/(_table.size()-1) - 1.0
                ));
            }
        }
        else if (_wave == "SQUARE")
        {
            for (size_t i = 0; i < _table.size(); i++)
            {
                const size_t q = (i+(3*_table.size())/4)%_table.size();
                this->setElem(_table[i], std::complex<double>(
                    (i < _table.size()/2)? 0.0 : 1.0,
                    (q < _table.size()/2)? 0.0 : 1.0
                ));
            }
        }
        else throw Pothos::InvalidArgumentException("WaveformSource::setWaveform("+_wave+")", "unknown waveform setting");
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
    size_t _step;
    std::vector<Type> _table;
    std::complex<double> _offset, _scalar;
    std::string _wave;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *waveformSourceFactory(const Pothos::DType &dtype)
{
    #define ifTypeDeclareFactory(type) \
        if (dtype == Pothos::DType(typeid(type))) return new WaveformSource<type>(); \
        if (dtype == Pothos::DType(typeid(std::complex<type>))) return new WaveformSource<std::complex<type>>();
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    ifTypeDeclareFactory(Poco::Int64);
    ifTypeDeclareFactory(Poco::Int32);
    ifTypeDeclareFactory(Poco::Int16);
    ifTypeDeclareFactory(Poco::Int8);
    throw Pothos::InvalidArgumentException("waveformSourceFactory("+dtype.toString()+")", "unsupported type");
}

static Pothos::BlockRegistry registerWaveformSource(
    "/blocks/waveform_source", &waveformSourceFactory);
