// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "FIRHelper.hpp"
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <complex>
#include <iostream>

/***********************************************************************
 * |PothosDoc FIR Designer
 *
 * Designer for FIR filter taps.
 * This block emits a "tapsChanged" signal upon activations,
 * and when one of the parameters is modified.
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
 * |option [Band Stop] "BAND_STOP"
 * |option [Complex Band Pass] "COMPLEX_BAND_PASS"
 * |option [Complex Band Stop] "COMPLEX_BAND_STOP"
 *
 * |param window[Window Type] The window function controls passband ripple.
 * Enter "Kaiser(beta)" to use the parameterized Kaiser window.
 * |default "hann"
 * |option [Rectangular] "rectangular"
 * |option [Hann] "hann"
 * |option [Hamming] "hamming"
 * |option [Blackman] "blackman"
 * |option [Bartlett] "bartlett"
 * |option [Flat-top] "flattop"
 * |widget ComboBox(editable=true)
 *
 * |param sampRate[Sample Rate] The rate of samples per second.
 * The transition frequencies must be within the Nyqist frequency of the sampling rate.
 * |default 1e6
 * |units Sps
 *
 * |param freqLower[Lower Freq] The lower transition frequency.
 * For low and high pass filters, this is the only transition frequency.
 * |default 1000
 * |units Hz
 *
 * |param freqUpper[Upper Freq] The upper transition frequency.
 * This parameter is only used for band pass and band reject filters.
 * |default 2000
 * |units Hz
 *
 * |param numTaps[Num Taps] The number of filter taps -- or computational complexity of the filter.
 * |default 51
 * |widget SpinBox(minimum=1)
 *
 * |factory /blocks/fir_designer()
 * |setter setFilterType(type)
 * |setter setWindowType(window)
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
        _windowType("hann"),
        _sampRate(1.0),
        _freqLower(0.1),
        _freqUpper(0.2),
        _numTaps(50)
    {
        auto env = Pothos::ProxyEnvironment::make("managed");
        _window = env->findProxy("Pothos/Util/WindowFunction").callProxy("new");
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setFilterType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, filterType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setWindowType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, windowType));
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

    void setWindowType(const std::string &type)
    {
        _windowType = type;
        this->recalculate();
    }

    std::string windowType(void) const
    {
        return _windowType;
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
    std::string _windowType;
    double _sampRate;
    double _freqLower;
    double _freqUpper;
    size_t _numTaps;
    Pothos::Proxy _window;
};

void FIRDesigner::recalculate(void)
{
    if (not this->isActive()) return;

    //check for error
    if (_numTaps == 0) Pothos::Exception("FIRDesigner()", "num taps must be positive");
    if (_sampRate <= 0) Pothos::Exception("FIRDesigner()", "sample rate must be positive");
    if (_freqLower <= 0) Pothos::Exception("FIRDesigner()", "lower frequency must be positive");
    if (_freqLower >= _sampRate/2) Pothos::Exception("FIRDesigner()", "lower frequency Nyquist fail");
    if (_freqUpper <= 0) Pothos::Exception("FIRDesigner()", "upper frequency must be positive");
    if (_freqUpper >= _sampRate/2) Pothos::Exception("FIRDesigner()", "upper frequency Nyquist fail");
    if (_freqUpper <= _freqLower) Pothos::Exception("FIRDesigner()", "upper frequency <= lower frequency");

    //generate the window
    _window.callVoid("setType", _windowType);
    _window.callVoid("setSize", _numTaps);
    auto window = _window.call<std::vector<double>>("window");

    //generate the filter taps
    std::vector<double> taps;
    std::vector<std::complex<double>> complexTaps;
    if (_filterType == "LOW_PASS") taps = designLPF(_numTaps, _sampRate, _freqLower, window);
    else if (_filterType == "HIGH_PASS") taps = designHPF(_numTaps, _sampRate, _freqLower, window);
    else if (_filterType == "BAND_PASS") taps = designBPF(_numTaps, _sampRate, _freqLower, _freqUpper, window);
    else if (_filterType == "BAND_STOP") taps = designBSF(_numTaps, _sampRate, _freqLower, _freqUpper, window);
    else if (_filterType == "COMPLEX_BAND_PASS") complexTaps = designCBPF(_numTaps, _sampRate, _freqLower, _freqUpper, window);
    else if (_filterType == "COMPLEX_BAND_STOP") complexTaps = designCBSF(_numTaps, _sampRate, _freqLower, _freqUpper, window);
    else throw Pothos::InvalidArgumentException("FIRDesigner("+_filterType+")", "unknown filter type");

    //emit the taps
    if (not complexTaps.empty()) this->callVoid("tapsChanged", complexTaps);
    if (not taps.empty()) this->callVoid("tapsChanged", taps);
}

static Pothos::BlockRegistry registerFIRDesigner(
    "/blocks/fir_designer", &FIRDesigner::make);
