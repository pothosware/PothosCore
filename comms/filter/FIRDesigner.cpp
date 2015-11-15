// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Poco/Logger.h>
#include <complex>
#include <algorithm>
#include <iostream>
#include <spuce/filters/design_fir.h>
#include <spuce/filters/design_window.h>
#include "FIRHelper.hpp"

using spuce::design_fir;
using spuce::design_complex_fir;
using spuce::design_window;
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
 * |keywords fir filter taps highpass lowpass bandpass remez
 * |alias /blocks/fir_designer
 *
 * |param type[Filter Type] The type of filter taps to generate.
 * <ul>
 * <li>MAXFLAT is based on a set of equations that only allow some discrete cut-off frequencies and is non-linear phase.</li>
 * <li>BOX-CAR/SINC is a truncated sin(x)/x impulse response</li>
 * <li>ROOT_RAISED_COSINE and RAISED_COSINE are based on ideal (infinite) impulse responses and have an 'alpha' factor for excess bandwidth</li>
 * <li>For GAUSSIAN, Lower Freq specifies the time-bandwidth product.</li>
 * </ul>
 * |option [Root Raised Cosine] "ROOT_RAISED_COSINE"
 * |option [Raised Cosine] "RAISED_COSINE"
 * |option [Box-Car] "SINC"
 * |option [Maxflat] "MAXFLAT"
 * |option [Gaussian] "GAUSSIAN"
 * |option [Remez] "REMEZ"
 * |default "SINC"
 *
 * |param band[Band Type] The band type of filter
 * |option [Low Pass] "LOW_PASS"
 * |option [High Pass] "HIGH_PASS"
 * |option [Band Pass] "BAND_PASS"
 * |option [Band Stop] "BAND_STOP"
 * |option [Complex Band Pass] "COMPLEX_BAND_PASS"
 * |option [Complex Band Stop] "COMPLEX_BAND_STOP"
 *
 * |param window[Window Type] The window function controls passband ripple.
 * |default "hann"
 * |option [Rectangular] "rectangular"
 * |option [Hann] "hann"
 * |option [Hamming] "hamming"
 * |option [Blackman] "blackman"
 * |option [Bartlett] "bartlett"
 * |option [Flat-top] "flattop"
 * |option [Kaiser] "kaiser"
 * |option [Chebyshev] "chebyshev"
 * |tab Window
 *
 * |param windowArgs[Window Args] Optional window arguments (depends on window type).
 * <ul>
 * <li>When using the <i>Kaiser</i> window, specify [beta] to use the parameterized Kaiser window.</li>
 * <li>When using the <i>Chebyshev</i> window, specify [atten] to use the Dolph-Chebyshev window with attenuation in dB.</li>
 * </ul>
 * |default []
 * |preview valid
 * |tab Window
 *
 * |param gain[Gain] The filter gain.
 * |default 1.0
 *
 * |param sampRate[Sample Rate] The sample rate, in samples per second.
 * The transition frequencies must be within the Nyqist frequency of the sampling rate.
 * |default 1e6
 * |units Sps
 *
 * |param freqLower[Lower Freq] The lower transition frequency.
 * For low and high pass filters, this is the only transition frequency.
 * For root raised cosine and Gaussian filters, this is the symbol rate.
 * |default 1000
 * |units Hz
 *
 * |param freqUpper[Upper Freq] The upper transition frequency.
 * This parameter is used for band pass and band reject filters.
 * |default 2000
 * |units Hz
 * |preview when(enum=band, "BAND_PASS", "BAND_STOP", "COMPLEX_BAND_PASS", "COMPLEX_BAND_STOP")
 *
 * |param freqTrans[Transition Freq] The transition bandwidth for Remez filters (only)
 * |default 1000
 * |units Hz
 * |preview when(enum=type, "REMEZ")
 * |tab Remez
 *
 * |param numTaps[Num Taps] The number of filter taps -- or computational complexity of the filter.
 * |default 51
 * |widget SpinBox(minimum=1)
 *
 * |param alpha[Alpha] For the raised and root-raised cosine filter, this is the excess bandwidth factor and can be from 0.0 to 1.0
 * |default 0.5
 * |preview when(enum=type, "RAISED_COSINE", "ROOT_RAISED_COSINE")
 * |tab Cosine
 *
 * |param weight[Weight] For the Remez filter. This is the weight for stopband attenuation vs passband ripple.
 * |default 100.0
 * |preview when(enum=type, "REMEZ")
 * |tab Remez
 *
 * |factory /comms/fir_designer()
 * |setter setFilterType(type)
 * |setter setBandType(band)
 * |setter setWindowType(window)
 * |setter setWindowArgs(windowArgs)
 * |setter setSampleRate(sampRate)
 * |setter setFrequencyLower(freqLower)
 * |setter setFrequencyUpper(freqUpper)
 * |setter setFrequencyTrans(freqTrans)
 * |setter setNumTaps(numTaps)
 * |setter setAlpha(alpha)
 * |setter setWeight(weight)
 * |setter setGain(gain)
 **********************************************************************/
class FIRDesigner : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new FIRDesigner();
    }

    FIRDesigner(void):
        _filterType("GAUSSIAN"),
        _bandType("LOW_PASS"),
        _windowType("hann"),
        _gain(1.0),
        _sampRate(1.0),
        _freqLower(0.1),
        _freqUpper(0.2),
        _freqTrans(0.1),
        _alpha(0.5),
        _weight(100.0),
        _numTaps(50)
    {
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setBandType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, bandType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setFilterType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, filterType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setWindowType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, windowType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setWindowArgs));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, windowArgs));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setSampleRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, sampleRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setFrequencyLower));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, frequencyLower));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setFrequencyUpper));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, frequencyUpper));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setFrequencyTrans));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, frequencyTrans));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setNumTaps));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, numTaps));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setAlpha));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, alpha));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setWeight));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, weight));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setGain));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, gain));
        this->registerSignal("tapsChanged");
        this->recalculate();
    }

    void setFilterType(const std::string &type)
    {
        //---------- BEGIN backwards compatible support --------------//
        if (
            type == "LOW_PASS" or
            type == "HIGH_PASS" or
            type == "BAND_PASS" or
            type == "BAND_STOP" or
            type == "COMPLEX_BAND_PASS" or
            type == "COMPLEX_BAND_STOP"
        )
        {
            poco_warning_f1(Poco::Logger::get("FIRDesigner"),
                "Filter type '%s' should now be used as a band type, with filter type set to 'SINC'", type);
            _filterType = "SINC";
            _bandType = type;
            this->recalculate();
            return;
        }
        //---------- END backwards compatible support --------------//

        _filterType = type;
        this->recalculate();
    }

    std::string filterType(void) const
    {
        return _filterType;
    }

    void setBandType(const std::string &type)
    {
        _bandType = type;
        this->recalculate();
    }

    std::string bandType(void) const
    {
        return _bandType;
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

    void setWindowArgs(const std::vector<double> &args)
    {
        _windowArgs = args;
        this->recalculate();
    }

    std::vector<double> windowArgs(void) const
    {
        return _windowArgs;
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
  
    void setFrequencyTrans(const double freq)
    {
        _freqTrans = freq;
        this->recalculate();
    }

    double frequencyTrans(void) const
    {
        return _freqTrans;
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

    void setAlpha(const double alpha)
    {
        _alpha = alpha;
        this->recalculate();
    }

    double alpha(void) const
    {
        return _alpha;
    }
  
    void setWeight(const double w)
    {
        _weight = w;
        this->recalculate();
    }

    double weight(void) const
    {
        return _weight;
    }

    void setGain(const double gain)
    {
        _gain = gain;
        this->recalculate();
    }

    double gain(void) const
    {
        return _gain;
    }

    void activate(void)
    {
        this->recalculate();
    }

private:

    void recalculate(void);

    std::string _filterType;
    std::string _bandType;
    std::string _windowType;
    std::vector<double> _windowArgs;
    double _gain;
    double _sampRate;
    double _freqLower;
    double _freqUpper;
    double _freqTrans;
    double _alpha;
    double _weight;
    size_t _numTaps;
};

void FIRDesigner::recalculate(void)
{
    if (not this->isActive()) return;

    const bool isComplex = _bandType.find("COMPLEX") != std::string::npos;
    const bool isStop    = _bandType.find("STOP") != std::string::npos;

    //check for error
    if (_numTaps == 0) throw Pothos::Exception("FIRDesigner()", "num taps must be positive");
    if (_sampRate <= 0) throw Pothos::Exception("FIRDesigner()", "sample rate must be positive");
    if (isComplex and _freqLower <= -_sampRate/2) throw Pothos::Exception("FIRDesigner()", "lower frequency below Nyquist range");
    if (not isComplex and _freqLower <= 0) throw Pothos::Exception("FIRDesigner()", "lower frequency must be positive");
    if (_freqLower >= _sampRate/2) throw Pothos::Exception("FIRDesigner()", "lower frequency above Nyquist range");

    //check upper freq only when its used
    if (_bandType == "BAND_PASS" or
        _bandType == "BAND_STOP" or
        _bandType == "COMPLEX_BAND_PASS" or
        _bandType == "COMPLEX_BAND_STOP")
    {
      if ((_numTaps % 2) == 0) throw Pothos::Exception("FIRDesigner()", "Band pass or Band stop FIRs must have an odd number of taps");
      if (isComplex and _freqUpper <= -_sampRate/2) throw Pothos::Exception("FIRDesigner()", "upper frequency below Nyquist range");
      if (not isComplex and _freqUpper <= 0) throw Pothos::Exception("FIRDesigner()", "upper frequency must be positive");
      if (_freqUpper >= _sampRate/2) throw Pothos::Exception("FIRDesigner()", "upper frequency above Nyquist range");
      if (_freqUpper <= _freqLower) throw Pothos::Exception("FIRDesigner()", "upper frequency <= lower frequency");
    }


    if (_filterType == "MAXFLAT") {
      if (isStop) {
        throw Pothos::Exception("FIRDesigner()", "Can not use MAXFLAT as prototype for stop-band filter, please choose another type");
      }
    } else if (_filterType == "REMEZ") {
      _alpha = _freqTrans/_sampRate;
    }

    std::string filt_type = _filterType;
    // Convert to lowercase for design_fir
    std::transform(filt_type.begin(), filt_type.end(), filt_type.begin(), ::tolower);

    //generate the filter taps
    std::vector<double> taps;
    std::vector<std::complex<double>> complexTaps;
    try {
      if ((_bandType == "COMPLEX_BAND_PASS") || (_bandType == "COMPLEX_BAND_STOP")) {
        complexTaps = design_complex_fir(filt_type, _bandType, _numTaps, _freqLower/_sampRate, _freqUpper/_sampRate, _alpha);
      } else {
        taps = design_fir(filt_type, _bandType, _numTaps, _freqLower/_sampRate, _freqUpper/_sampRate, _alpha, _weight);
      }
    }
    catch (const std::runtime_error error) {
      throw Pothos::InvalidArgumentException("Problem with creating taps for FIRDesigner("+_filterType+"/"+_bandType+"):"+error.what(), "problem with input parameters?");
    }
      

    /* apply gain */
    std::transform(complexTaps.begin(), complexTaps.end(), complexTaps.begin(),
                   std::bind1st(std::multiplies<std::complex<double>>(),_gain));
    std::transform(taps.begin(), taps.end(), taps.begin(),
                   std::bind1st(std::multiplies<double>(),_gain));

    //generate the window
    const auto window = design_window(_windowType, _numTaps, _windowArgs.empty()?0.0:_windowArgs.at(0));

    // Apply window & emit taps
    if (not complexTaps.empty()) {
      for (size_t i=0;i<_numTaps;i++) complexTaps[i] *= window[i];
      this->callVoid("tapsChanged", complexTaps);
    } else if (not taps.empty()) {
      for (size_t i=0;i<_numTaps;i++) taps[i] *= window[i];
      this->callVoid("tapsChanged", taps);
    }
}

static Pothos::BlockRegistry registerFIRDesigner(
    "/comms/fir_designer", &FIRDesigner::make);

static Pothos::BlockRegistry registerFIRDesignerOldPath(
    "/blocks/fir_designer", &FIRDesigner::make);
