#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <complex>
#include <iostream>

#include <spuce/filters/design_iir.h>
#include <spuce/filters/iir_coeff.h>

using namespace spuce;

/***********************************************************************
 * |PothosDoc IIR Designer
 *
 * Designer for IIR filter taps.
 * This block emits a "tapsChanged" signal upon activations,
 * and when one of the parameters is modified.
 * The "tapsChanged" signal contains two arrays of taps,
 * and can be connected to a IIR filter's setIIR method.
 *
 * |category /Filter
 * |keywords iir filter taps highpass lowpass 
 * |alias /blocks/iir_designer
 *
 * |param type[Filter Type] The type of filter taps to generate.
 * |option [Low Pass] "LOW_PASS"
 * |option [High Pass] "HIGH_PASS"
 *
 * |param iir[IIR Type] The type of IIR filter.
 * |default "butterworth"
 * |option [Butterworth] "butterworth"
 * |option [Chebyshev] "chebyshev"
 * |option [Elliptic] "elliptic"
 * |widget ComboBox(editable=true)
 *
 * |param sampRate[Sample Rate] The sample rate, in samples per second.
 * The transition frequencies must be within the Nyqist frequency of the sampling rate.
 * |default 44100
 * |units Sps
 *
 * |param order[Order] The order of the IIR filter.
 * |default 2
 * |widget SpinBox(minimum=1)
 *
 * |param freqLower[Lower Freq] The lower transition frequency.
 * For low and high pass filters, this is the only transition frequency.
 * For root raised cosine and Gaussian filters, this is the symbol rate.
 * |default 4000
 * |units Hz
 *
 * |param stopBandAtten[Stop Band Attenuation] The stop band attenuation for elliptic filters.
 * |default 60
 * |units dB
 *
 * |param ripple[Ripple] For Chebyshev or Elliptic filters
 * This is pass band ripple in dB
 * |default 0.1
 * |units dB
 *
 * |param freqUpper[Upper Freq] The upper transition frequency.
 * This parameter is only used for elliptic filters
 * |default 8000
 * |units Hz
 *
 * |factory /comms/iir_designer()
 * |setter setFilterType(type)
 * |setter setIIRType(iir)
 * |setter setSampleRate(sampRate)
 * |setter setFrequencyLower(freqLower)
 * |setter setFrequencyUpper(freqUpper)
 * |setter setOrder(order)
 * |setter setRipple(ripple)
 * |setter setStopBandAtten(stopBandAtten)
 **********************************************************************/
class IIRDesigner : public Pothos::Block {
 public:
  static Block *make(void) { return new IIRDesigner(); }

  IIRDesigner(void)
      : _filterType("LOW_PASS"),
        _IIRType("butterworth"),
        _sampRate(1.0),
        _freqLower(0.1),
        _freqUpper(0.2),
        _stopBandAtten(0.1),
        _ripple(0.1),
        _order(4) {
    auto env = Pothos::ProxyEnvironment::make("managed");
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRDesigner, setFilterType));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRDesigner, filterType));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRDesigner, setIIRType));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRDesigner, IIRType));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRDesigner, setSampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRDesigner, sampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRDesigner, setFrequencyLower));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRDesigner, frequencyLower));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRDesigner, setFrequencyUpper));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRDesigner, frequencyUpper));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRDesigner, setOrder));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRDesigner, order));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRDesigner, setRipple));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRDesigner, ripple));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRDesigner, setStopBandAtten));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRDesigner, stopBandAtten));
    this->registerSignal("tapsChanged");
    this->recalculate();
  }

  void setFilterType(const std::string &type) {
    _filterType = type;
    this->recalculate();
  }

  std::string filterType(void) const { return _filterType; }

  void setIIRType(const std::string &type) {
    _IIRType = type;
    this->recalculate();
  }

  std::string IIRType(void) const { return _IIRType; }

  void setSampleRate(const double rate) {
    _sampRate = rate;
    this->recalculate();
  }

  double sampleRate(void) const { return _sampRate; }

  void setFrequencyLower(const double freq) {
    _freqLower = freq;
    this->recalculate();
  }

  double frequencyLower(void) const { return _freqLower; }

  void setFrequencyUpper(const double freq) {
    _freqUpper = freq;
    this->recalculate();
  }

  double frequencyUpper(void) const { return _freqUpper; }

  void setOrder(const size_t num) {
    _order = num;
    this->recalculate();
  }

  size_t order(void) const { return _order; }

  void setRipple(const double rip) {
    _ripple = rip;
    this->recalculate();
  }
  double ripple(void) const { return _ripple; }

  void setStopBandAtten(const double db) {
    _stopBandAtten = db;
    this->recalculate();
  }
  double stopBandAtten(void) const { return _stopBandAtten; }

  void activate(void) { this->recalculate(); }

 private:
  void recalculate(void);

  std::string _filterType;
  std::string _IIRType;
  double _sampRate;
  double _freqLower;
  double _freqUpper;
  double _stopBandAtten;
  double _ripple;
  size_t _order;
};

void IIRDesigner::recalculate(void) {
  if (not this->isActive()) return;

  // check for error
  if (_order == 0) Pothos::Exception("IIRDesigner()", "order must be positive");
  if (_sampRate <= 0) Pothos::Exception("IIRDesigner()", "sample rate must be positive");
  if (_freqLower <= 0) Pothos::Exception("IIRDesigner()", "lower frequency must be positive");
  if (_freqLower >= _sampRate / 2) Pothos::Exception("IIRDesigner()", "lower frequency Nyquist fail");
  if (_filterType == "HIGH_PASS" || _filterType == "BAND_PASS" || _filterType == "BAND_STOP" ||
      _filterType == "COMPLEX_BAND_PASS" || _filterType == "COMPLEX_BAND_STOP") {
    if (_freqUpper <= 0) Pothos::Exception("IIRDesigner()", "upper frequency must be positive");
    if (_freqUpper >= _sampRate / 2) Pothos::Exception("IIRDesigner()", "upper frequency Nyquist fail");
    if (_freqUpper <= _freqLower) Pothos::Exception("IIRDesigner()", "upper frequency <= lower frequency");
  }

  // generate the filter design
  iir_coeff* filt = design_iir(_IIRType, _filterType, _order,
															 _freqLower/_sampRate, _ripple,
															 _stopBandAtten, _freqUpper/_sampRate);
	if (filt == nullptr) {
		throw Pothos::InvalidArgumentException("IIRDesigner(" + _filterType + "," + _IIRType + ")", "unknown filter or band type");
  }

  // get the tap from iir_coeff for iir_filter, incorporating the gain to feedforward taps
	std::vector<double> b = filt->get_b();
	std::vector<double> a = filt->get_a();
	// Group together feed forward and feed back taps into 1 vector for transferring to IIR filter
	for (int i=0;i<a.size();i++) b.push_back(a[i]);

	delete filt;
	this->callVoid("tapsChanged", b);
}

static Pothos::BlockRegistry registerIIRDesigner("/comms/iir_designer", &IIRDesigner::make);
