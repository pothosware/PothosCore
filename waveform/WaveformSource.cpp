// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Util/MathCompat.hpp>
#include <cstdint>
#include <iostream>
#include <complex>

static const size_t defaultWaveTableSize = 4096;
static const size_t maxWaveTableSize = 1024*1024;
static const size_t minimumTableStepSize = 16;

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
 * |param res[Resolution] The resolution of the internal wave table (0.0 for automatic).
 * When unspecified, the wave table size will be configured for the user's requested frequency.
 * Specify a minimum resolution in Hz to fix the size of the wave table.
 * |units Hz
 * |default 0.0
 * |preview valid
 *
 * |factory /blocks/waveform_source(dtype)
 * |setter setSampleRate(rate)
 * |setter setWaveform(wave)
 * |setter setOffset(offset)
 * |setter setAmplitude(ampl)
 * |setter setFrequency(freq)
 * |setter setResolution(res)
 **********************************************************************/
template <typename Type>
class WaveformSource : public Pothos::Block
{
public:
    WaveformSource(void):
        _index(0), _step(0), _mask(0),
        _rate(1.0), _freq(0.0), _res(0.0),
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
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveformSource<Type>, setResolution));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveformSource<Type>, getResolution));
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
            out[i] = _table[_index & _mask];
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

    void setResolution(const double &res)
    {
        _res = res;
        this->updateStep();
    }

    double getResolution(void)
    {
        return _res;
    }

private:
    void updateStep(void)
    {
        //This fraction (of a period) is used to determine table size efficacy.
        //When specified, use the resolution, otherwise the user's frequency.
        const auto frac = ((_res == 0.0)?_freq:_res)/_rate;

        //loop for a table size that meets the minimum step
        size_t numEntries = defaultWaveTableSize;
        while (true)
        {
            const auto delta = std::llround(frac*numEntries);
            if (frac == 0.0) break;
            if (std::abs(delta) >= minimumTableStepSize) break;
            if (numEntries*2 > _table.max_size()) break;
            if (numEntries*2 > maxWaveTableSize) break;
            numEntries *= 2;
        }

        //update mask: assumes power of 2
        _mask = numEntries-1;

        //update step: given ratio and table size
        _step = size_t(std::llround((_freq/_rate)*numEntries));

        //check for table size change and update
        if (numEntries != _table.size())
        {
            _table.resize(numEntries);
            this->updateTable();
        }
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
    size_t _mask;
    double _rate;
    double _freq;
    double _res;
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
