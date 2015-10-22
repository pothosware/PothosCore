#include <Pothos/Framework.hpp>
#include <cstdint>
#include <complex>
#include <cassert>
#include <cstring>  //memset, memcpy
#include <iostream>
#include <algorithm>  //min/max

#include <spuce/filters/iir_df.h>
using namespace spuce;
/***********************************************************************
 * |PothosDoc IIR Filter
 *
 * The IIR filter convolves an input element stream from port 0 with
 * the filter taps to produce an output element stream on port 0.
 *
 * <a href="https://en.wikipedia.org/wiki/Infinite_impulse_response">
 * https://en.wikipedia.org/wiki/Infinite_impulse_response</a>
 *
 * |category /Filter
 * |keywords iir filter taps highpass lowpass
 * |alias /blocks/iir_filter
 *
 * |param dtype[Data Type] The data type of the input and output element stream.
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param taps The IIR filter taps used in convolution.
 * Manually enter or paste in IIR filter taps or leave this entry blank
 * and use the IIR Designer taps signal to configure the filter taps at runtime.
 * |default [0.0676, 0.135, 0.0676, 1, -1.142, 0.412]
 *
 * |param waitTaps[Wait Taps] Wait for the taps to be set before allowing operation.
 * Use this mode when taps are set exclusively at runtime by the setTaps() slot.
 * |default false
 * |preview valid
 * |option [Enabled] true
 * |option [Disabled] false
 *
 * |factory /comms/iir_filter(dtype)
 * |setter setTaps(taps, taps)
 * |setter setWaitTaps(waitTaps)
 **********************************************************************/
template <typename Type> class IIRFilter : public Pothos::Block {
 public:
  IIRFilter(void) : _waitTapsMode(false), _waitTapsArmed(false) {
    this->setupInput(0, typeid(Type));
    this->setupOutput(0, typeid(Type));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRFilter, setTaps));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRFilter, setWaitTaps));
    this->registerCall(this, POTHOS_FCN_TUPLE(IIRFilter, getWaitTaps));
		std::vector<double> taps = {0.0676, 0.135, 0.0676, 1, -1.142, 0.412};
		this->setTaps(taps);  // initial update
  }

  void setWaitTaps(const bool waitTaps) { _waitTapsMode = waitTaps; }

  bool getWaitTaps(void) const { return _waitTapsMode; }

  void setTaps(const std::vector<double>& taps) {
    if (taps.size() == 0) throw Pothos::InvalidArgumentException("IIRFilter::setTaps()", "Order cannot 0");
#ifdef DEBUG
		std::cout << "Called SetIIR with taps A :";
		for (int i=0;i<taps.size()/2;i++) std::cout << taps[i] << " ";
		std::cout <<", B :";
		for (int i=0;i<taps.size()/2;i++) std::cout << taps[i+taps.size()/2] << " ";
		std::cout <<"\n";
#endif
		IIR.set_taps(taps);
		IIR.reset();
    _waitTapsArmed = false;  // got taps
  }

	void reset(void)
	{
		IIR.reset();
	}
	
  void activate(void) {
		this->reset();
    _waitTapsArmed = _waitTapsMode;
  }

	void work(void)
	{
		if (_waitTapsArmed) return;
		auto inPort = this->input(0);
		auto outPort = this->output(0);

		const size_t N = this->workInfo().minElements;

		//cast the input and output buffers
		const auto in = inPort->buffer().template as<const Type *>();
		const auto out = outPort->buffer().template as<Type *>();

    for (size_t n = 0; n < N; n++) {
			out[n] = IIR.clock(in[n]);
		}
		inPort->consume(N);
		outPort->produce(N);
	}
	
 private:
	iir_df<Type, double> IIR;
  bool _waitTapsMode;
  bool _waitTapsArmed;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *IIRFilterFactory(const Pothos::DType &dtype) {
#define ifTypeDeclareFactory__(Type) \
  if (dtype == Pothos::DType(typeid(Type))) return new IIRFilter<Type>();
#define ifTypeDeclareFactory(type) ifTypeDeclareFactory__(type) ifTypeDeclareFactory__(std::complex<type>)
  ifTypeDeclareFactory(double);
	ifTypeDeclareFactory(float);
	ifTypeDeclareFactory(int64_t);
	ifTypeDeclareFactory(int32_t);
  ifTypeDeclareFactory(int16_t);
  ifTypeDeclareFactory(int8_t);
  throw Pothos::InvalidArgumentException("IIRFilterFactory(" + dtype.toString() + ")", "unsupported types");
}
static Pothos::BlockRegistry registerIIRFilter("/comms/iir_filter", &IIRFilterFactory);
