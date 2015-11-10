// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/Logger.h>
#include <complex>
#include <algorithm>
#include <iostream>
#include <spuce/filters/remez_fir.h>
#include <spuce/filters/design_fir.h>
#include <spuce/filters/transform_fir.h>

using spuce::design_fir;
using spuce::transform_fir;
using spuce::transform_complex_fir;
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
 * MAXFLAT is based on a set of equations that only allow some discrete cut-off frequencies and is non-linear phase.
 * BOX-CAR/SINC is a truncated sin(x)/x impulse response
 * ROOT_RAISED_COSINE and RAISED_COSINE are based on ideal (infinite) impulse responses and have an 'alpha' factor for excess bandwidth
 * For GAUSSIAN, frequency lower specifies the time-bandwidth product.
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
 * Enter "kaiser(beta)" to use the parameterized Kaiser window.
 * Enter "chebyshev(atten)" to use the Dolph-Chebyshev window with attenuation in dB in parenthesis
 * |default "hann"
 * |option [Rectangular] "rectangular"
 * |option [Hann] "hann"
 * |option [Hamming] "hamming"
 * |option [Blackman] "blackman"
 * |option [Bartlett] "bartlett"
 * |option [Flat-top] "flattop"
 * |widget ComboBox(editable=true)
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
 *
 * |param numTaps[Num Taps] The number of filter taps -- or computational complexity of the filter.
 * |default 51
 * |widget SpinBox(minimum=1)
 *
 * |param beta[Beta] For the raised and root-raised cosine filter, this is the excess bandwidth factor.
 * |default 0.5
 * |preview when(enum=type, "RAISED_COSINE", "ROOT_RAISED_COSINE")
 *
 * |param weight[Weight] For the Remez filter. This is the weight for stopband attenuation vs passband ripple.
 * |default 100.0
 * |preview when(enum=type, "REMEZ")
 *
 * |factory /comms/fir_designer()
 * |setter setFilterType(type)
 * |setter setBandType(band)
 * |setter setWindowType(window)
 * |setter setSampleRate(sampRate)
 * |setter setFrequencyLower(freqLower)
 * |setter setFrequencyUpper(freqUpper)
 * |setter setFrequencyTrans(freqTrans)
 * |setter setNumTaps(numTaps)
 * |setter setBeta(beta)
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
        _beta(0.5),
        _weight(100.0),
        _numTaps(50)
    {
        auto env = Pothos::ProxyEnvironment::make("managed");
        _window = env->findProxy("Pothos/Comms/WindowFunction").callProxy("new");
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setBandType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, bandType));
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
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setFrequencyTrans));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, frequencyTrans));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setNumTaps));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, numTaps));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, setBeta));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRDesigner, beta));
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

    void setBeta(const double beta)
    {
        _beta = beta;
        this->recalculate();
    }

    double beta(void) const
    {
        return _beta;
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
    double _gain;
    double _sampRate;
    double _freqLower;
    double _freqUpper;
    double _freqTrans;
    double _beta;
    double _weight;
    size_t _numTaps;
    Pothos::Proxy _window;
};

void FIRDesigner::recalculate(void)
{
    if (not this->isActive()) return;

    const bool isComplex = _bandType.find("COMPLEX") != std::string::npos;
    double center_frequency = 0.0;

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
        if (isComplex and _freqUpper <= -_sampRate/2) throw Pothos::Exception("FIRDesigner()", "upper frequency below Nyquist range");
        if (not isComplex and _freqUpper <= 0) throw Pothos::Exception("FIRDesigner()", "upper frequency must be positive");
        if (_freqUpper >= _sampRate/2) throw Pothos::Exception("FIRDesigner()", "upper frequency above Nyquist range");
        if (_freqUpper <= _freqLower) throw Pothos::Exception("FIRDesigner()", "upper frequency <= lower frequency");
    }

    double filt_bw;

    if (_bandType == "BAND_PASS" or
        _bandType == "BAND_STOP" or
        _bandType == "COMPLEX_BAND_PASS" or
        _bandType == "COMPLEX_BAND_STOP")
    {
      center_frequency = 0.5*(_freqUpper + _freqLower)/_sampRate;
      // Since bandpass/stop, 1/2 the band-pass filter bandwidth since double sided
      // Also bandwidth is absolute value
      filt_bw = std::abs(0.5*(_freqUpper - _freqLower)/_sampRate);
    } else {
      filt_bw = _freqLower/_sampRate;
    }

    //generate the filter taps
    std::vector<double> taps;
    if (_filterType == "ROOT_RAISED_COSINE") taps = design_fir("rootraisedcosine", _numTaps, filt_bw, _beta);
    else if (_filterType == "RAISED_COSINE") taps = design_fir("raisedcosine", _numTaps, filt_bw, _beta);
    else if (_filterType == "GAUSSIAN") taps = design_fir("gaussian", _numTaps, filt_bw);
    else if (_filterType == "SINC") taps = design_fir("sinc", _numTaps, filt_bw);
    else if (_filterType == "MAXFLAT") {
      filt_bw = std::max(filt_bw,0.02);
      taps = design_fir("butterworth", _numTaps, filt_bw);
    }
    else if (_filterType == "REMEZ") {
      double stop_freq = filt_bw + (_freqTrans/_sampRate);
      if (stop_freq >= _sampRate/2) {
        throw Pothos::Exception("FIRDesigner()", "Remez stopband frequency above Nyquist range");
      }
      try {
        // for Band_stop filter, flip the weight since using low-pass prototype
        if (_bandType == "BAND_STOP" or _bandType == "COMPLEX_BAND_STOP") {
          taps = design_fir("remez", _numTaps, filt_bw, stop_freq, 1.0/_weight);
        } else {
          taps = design_fir("remez", _numTaps, filt_bw, stop_freq, _weight);
        }
      }
      catch (const std::runtime_error& error) {
        throw Pothos::InvalidArgumentException("FIRDesigner("+_filterType+"):"+error.what(), "problem with input parameters");
      }
    }
    else throw Pothos::InvalidArgumentException("FIRDesigner("+_filterType+")", "unknown filter type");
     
    //generate the window
    _window.callVoid("setType", _windowType);
    _window.callVoid("setSize", _numTaps);
    auto window = _window.call<std::vector<double>>("window");

    // Apply window
    for (size_t i=0;i<_numTaps;i++) {
      taps[i] *= window[i];
    }

    // Transform Taps!!
    std::vector<std::complex<double>> complexTaps;
    if (_bandType == "HIGH_PASS") taps = transform_fir("HIGH_PASS", taps, center_frequency); 
    else if (_bandType == "BAND_PASS") taps = transform_fir("BAND_PASS", taps, center_frequency);
    else if (_bandType == "BAND_STOP") taps = transform_fir("BAND_STOP", taps, center_frequency);
    else if (_bandType == "COMPLEX_BAND_PASS") complexTaps = transform_complex_fir("COMPLEX_BAND_PASS", taps, center_frequency);
    else if (_bandType == "COMPLEX_BAND_STOP") complexTaps = transform_complex_fir("COMPLEX_BAND_STOP", taps, center_frequency);

    /* apply gain */
    std::transform(complexTaps.begin(), complexTaps.end(), complexTaps.begin(),
                   std::bind1st(std::multiplies<std::complex<double>>(),_gain));
    std::transform(taps.begin(), taps.end(), taps.begin(),
                   std::bind1st(std::multiplies<double>(),_gain));

    //emit the taps
    if (not complexTaps.empty()) this->callVoid("tapsChanged", complexTaps);
    else if (not taps.empty()) this->callVoid("tapsChanged", taps);
}

static Pothos::BlockRegistry registerFIRDesigner(
    "/comms/fir_designer", &FIRDesigner::make);

static Pothos::BlockRegistry registerFIRDesignerOldPath(
    "/blocks/fir_designer", &FIRDesigner::make);
