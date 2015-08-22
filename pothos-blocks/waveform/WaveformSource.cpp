// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Util/MathCompat.hpp>
#include <cstdint>
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
 * |keywords cosine sine ramp square waveform source signal
 *
 * |param dtype[Data Type] The data type produced by the waveform source.
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param wave[Wave Type] The type of the waveform produced.
 * |option [Constant] "CONST"
 * |option [Sinusoid] "SINE"
 * |option [Ramp] "RAMP"
 * |option [Square] "SQUARE"
 * |default "SINE"
 *
 * |param rate[Sample Rate] The sample rate of the waveform.
 * |units samples/sec
 * |default 1.0
 *
 * |param freq[Frequency] The frequency of the waveform (+/- 0.5*rate).
 * |units Hz
 * |default 0.1
 *
 * |param ampl[Amplitude] A constant scalar representing the amplitude.
 * |default 1.0
 *
 * |param offset A constant value added to the waveform after scaling.
 * |default 0.0
 * |preview valid
 *
 * |factory /blocks/waveform_source(dtype)
 * |setter setSampleRate(rate)
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
        _rate(1.0), _freq(0.0),
        _table(waveTableSize),
        _offset(0.0), _scalar(1.0),
        _wave("CONST")
    {
        this->setupOutput(0, typeid(Type));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveformSource<Type>, setWaveform));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveformSource<Type>, getWaveform));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveformSource<Type>, setOffset));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveformSource<Type>, getOffset));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveformSource<Type>, setAmplitude));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveformSource<Type>, getAmplitude));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveformSource<Type>, setFrequency));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveformSource<Type>, getFrequency));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveformSource<Type>, setSampleRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveformSource<Type>, getSampleRate));
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
        _freq = freq;
        this->updateStep();
    }

    double getFrequency(void)
    {
        return _freq;
    }

    void setSampleRate(const double &rate)
    {
        _rate = rate;
        this->updateStep();
    }

    double getSampleRate(void)
    {
        return _rate;
    }

private:
    void updateStep(void)
    {
        _step = size_t(std::llround((_freq/_rate)*_table.size()));
    }

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
                this->setElem(_table[i], std::polar(1.0, 2*M_PI*i/_table.size()));
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
    double _rate;
    double _freq;
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
    ifTypeDeclareFactory(int64_t);
    ifTypeDeclareFactory(int32_t);
    ifTypeDeclareFactory(int16_t);
    ifTypeDeclareFactory(int8_t);
    throw Pothos::InvalidArgumentException("waveformSourceFactory("+dtype.toString()+")", "unsupported type");
}

static Pothos::BlockRegistry registerWaveformSource(
    "/blocks/waveform_source", &waveformSourceFactory);
