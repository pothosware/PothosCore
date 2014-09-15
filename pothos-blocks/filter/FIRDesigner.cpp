// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <fir_filter/filt.h>

/***********************************************************************
 * |PothosDoc FIR Designer
 *
 * Designer for FIR filter taps.
 * This block emits a "tapsChanged" signal upon activations,
 * and when one of the parameters are modified.
 * The "tapsChanged" signal contains an array of FIR taps,
 * and can be connected to a FIR filter's set taps method.
 *
 * |category /Filter
 * |keywords fir filter taps highpass lowpass bandpass
 *
 * |param type[Filter Type] The type of filter taps to generate.
 * |option [Low Pass] "LOW_PASS"
 * |option [High Pass] "HIGH_PASS"
 * |option [Band Pass] "BAND_PASS"
 * |option [Complex Band Pass] "COMPLEX_BAND_PASS"
 *
 * |param sampRate[Sample Rate]
 * |default 1e6
 * |units Sps
 *
 * |param freqLower[Lower Freq]
 * |default 1000
 * |units Hz
 *
 * |param freqUpper[Upper Freq]
 * |default 2000
 * |units Hz
 *
 * |param numTaps[Num Taps]
 * |default 50
 * |widget SpinBox(minimum=1)
 *
 * |factory /blocks/fir_designer()
 * |setter setFilterType(type)
 * |setter setSampleRate(sampRate)
 * |setter setFrequencyLower(freqLower)
 * |setter setFrequencyUpper(freqUpper)
 * |setter setNumTaps(numTaps)
 **********************************************************************/
class FIRDesigner : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new FIRDesigner();
    }

    FIRDesigner(void):
        _filterType("LOW_PASS"),
        _sampRate(1.0),
        _freqLower(0.1),
        _freqUpper(0.2),
        _numTaps(50)
    {
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setFilterType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, filterType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setSampleRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, sampleRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setFrequencyLower));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, frequencyLower));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setFrequencyUpper));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, frequencyUpper));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setNumTaps));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, numTaps));
        this->registerSignal("tapsChanged");
        this->recalculate();
    }

    void setFilterType(const std::string &type)
    {
        _filterType = type;
        this->recalculate();
    }

    std::string filterType(void) const
    {
        return _filterType;
    }

    void setSampleRate(const double rate)
    {
        _sampRate = rate;
        this->recalculate();
    }

    double sampleRate(void) const
    {
        return _sampRate;
    }

    void setFrequencyLower(const double freq)
    {
        _freqLower = freq;
        this->recalculate();
    }

    double frequencyLower(void) const
    {
        return _freqLower;
    }

    void setFrequencyUpper(const double freq)
    {
        _freqUpper = freq;
        this->recalculate();
    }

    double frequencyUpper(void) const
    {
        return _freqUpper;
    }

    void setNumTaps(const size_t num)
    {
        _numTaps = num;
        //odd number of taps to center sync function
        if ((_numTaps & 0x1) == 0) _numTaps++;
        this->recalculate();
    }

    size_t numTaps(void) const
    {
        return _numTaps;
    }

    void activate(void)
    {
        this->recalculate();
    }

private:

    void recalculate(void);

    std::string _filterType;
    double _sampRate;
    double _freqLower;
    double _freqUpper;
    size_t _numTaps;
};

void FIRDesigner::recalculate(void)
{
    std::shared_ptr<Filter> filt;
    if (_filterType == "LOW_PASS") filt.reset(new Filter(LPF, _numTaps, _sampRate, _freqLower));
    else if (_filterType == "HIGH_PASS") filt.reset(new Filter(HPF, _numTaps, _sampRate, _freqLower));
    else if (_filterType == "BAND_PASS") filt.reset(new Filter(BPF, _numTaps, _sampRate, _freqLower, _freqUpper));
    else if (_filterType == "COMPLEX_BAND_PASS")
    {
        //TODO
    }
    else
    {
        throw Pothos::InvalidArgumentException("FIRDesigner::recalculate("+_filterType+")", "unknown filter type");
    }


    std::vector<double> taps(_numTaps);
    filt->get_taps(taps.data());

    //TODO handle convert to complex taps + shift

    //TODO apply window and calculate total gain

    //TODO remove gain

    if (this->isActive()) this->callVoid("tapsChanged", taps);
}

static Pothos::BlockRegistry registerFIRDesigner(
    "/blocks/fir_designer", &FIRDesigner::make);
