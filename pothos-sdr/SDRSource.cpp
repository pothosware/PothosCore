// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <SoapySDR/Device.hpp>
#include <Poco/Logger.h>
#include <iostream>
#include <future>

/***********************************************************************
 * |PothosDoc SDR Source
 *
 * The SDR source block configures the receiver end of an SDR
 * and forwards baseband samples into 1 or more output streams.
 *
 * |category /Sources
 * |category /SDR
 *
 * |param deviceArgs[Device Args] Device construction arguments.
 * A map of key/value string pairs that identifies a SDR device.
 * |default {"driver":"null", "type":"null"}
 * |preview disable
 * |tab Streaming
 *
 * |param dtype[Data Type] The data type produced by the SDR source.
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float32"
 * |preview disable
 * |tab Streaming
 *
 * |param channels[Channels] A list of RX channels to stream from.
 * Each element maps a port index on the block to a device channel.
 * |default [0]
 * |preview disable
 * |tab Streaming
 *
 * |param streamArgs[Stream Args] Additional stream arguments.
 * A map of key/value string pairs with implementation-specific meaning.
 * |default {}
 * |preview valid
 * |tab Streaming
 *
 * |param sampleRate[Sample Rate] The rate of sample stream on each channel.
 * |units Sps
 * |default 1e6
 * |tab Streaming
 *
 * |param frequency0[Frequency] The center frequency of channel 0.
 * |units Hz
 * |default 1e9
 * |tab Frontend0
 *
 * |param tuneArgs0[Tune Args] Advanced key/value tuning parameters.
 * |default {}
 * |preview valid
 * |tab Frontend0
 *
 * |param gainMode0[Gain Mode] Gain selection mode for channel 0.
 * The possible options are automatic mode or manual mode.
 * In manual mode the user-provided gain value will be used.
 * |default false
 * |option [Automatic] true
 * |option [Manual] false
 * |preview valid
 * |tab Frontend0
 *
 * |param gain0[Gain Value] The overall amplification of channel 0.
 * The user may also provide a dictionary of name to gain values
 * where each name represents an amplification element in the chain.
 * Example: {"BB" : 10.0, "RF" : "5.5"}
 * |units dB
 * |default 0.0
 * |preview valid
 * |tab Frontend0
 *
 * |param antenna0[Antenna] The selected antenna for channel 0.
 * |default ""
 * |tab Frontend0
 * |preview valid
 * |widget StringEntry()
 *
 * |param bandwidth0[Bandwidth] The filter bandwidth for channel 0.
 * |units Hz
 * |default 0.0
 * |preview valid
 * |tab Frontend0
 *
 * |param clockRate[Clock rate] The master reference clock rate.
 * |default 0.0
 * |units Hz
 * |preview valid
 * |tab Clocking
 *
 * |param clockSource[Clock Source] The source of the reference clock.
 * |default ""
 * |preview valid
 * |tab Clocking
 * |widget StringEntry()
 *
 * |param timeSource[Time Source] The source of the hardware time.
 * |default ""
 * |preview valid
 * |tab Clocking
 * |widget StringEntry()
 *
 * |factory /sdr/source(dtype, channels)
 * |initializer setupDevice(deviceArgs)
 * |initializer setupStream(streamArgs)
 * |setter setSampleRate(sampleRate)
 * |setter setClockRate(clockRate)
 * |setter setClockSource(clockSource)
 * |setter setTimeSource(timeSource)
 **********************************************************************/
class SDRSource : public Pothos::Block
{
public:
    static Block *make(const Pothos::DType &dtype, const std::vector<size_t> &channels)
    {
        return new SDRSource(dtype, channels);
    }

    SDRSource(const Pothos::DType &dtype, const std::vector<size_t> &channels):
        _channels(channels),
        _device(nullptr),
        _stream(nullptr)
    {
        for (size_t i = 0; i < channels.size(); i++) this->setupOutput(i, dtype);

        //streaming
        this->registerCall(this, POTHOS_FCN_TUPLE(SDRSource, setupDevice));
        this->registerCall(this, POTHOS_FCN_TUPLE(SDRSource, setupStream));
        this->registerCall(this, POTHOS_FCN_TUPLE(SDRSource, setSampleRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(SDRSource, getSampleRate));

        //frontend
        //this->registerCallable("setFrequency", Pothos::Callable(&SDRSource::setFrequency).bind(std::ref(*this), 0));
        //this->registerCallable("setFrequency", Pothos::Callable(&SDRSource::setFrequency).bind(std::ref(*this), 0).bind(std::map<std::string, std::string>(), 3));

        //clocking
        this->registerCall(this, POTHOS_FCN_TUPLE(SDRSource, setClockRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(SDRSource, getClockRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(SDRSource, setClockSource));
        this->registerCall(this, POTHOS_FCN_TUPLE(SDRSource, getClockSource));
        this->registerCall(this, POTHOS_FCN_TUPLE(SDRSource, setTimeSource));
        this->registerCall(this, POTHOS_FCN_TUPLE(SDRSource, getTimeSource));
    }

    ~SDRSource(void)
    {
        if (_stream != nullptr) _device->closeStream(_stream);
        if (_device != nullptr) SoapySDR::Device::unmake(_device);
    }

    /*******************************************************************
     * Device object creation
     ******************************************************************/
    static SoapySDR::Device *makeDevice(const std::map<std::string, std::string> &deviceArgs)
    {
        return SoapySDR::Device::make(deviceArgs);
    }

    void setupDevice(const std::map<std::string, std::string> &deviceArgs)
    {
        _deviceFuture = std::async(std::launch::async, &SDRSource::makeDevice, deviceArgs);
    }

    /*******************************************************************
     * Delayed method dispatch
     ******************************************************************/
    Pothos::Object opaqueCallHandler(const std::string &name, const Pothos::Object *inputArgs, const size_t numArgs)
    {
        //try to setup the device future first
        if (name == "setupDevice") return Pothos::Block::opaqueCallHandler(name, inputArgs, numArgs);

        //when ready forward the call to the handler
        if (this->isReady()) return Pothos::Block::opaqueCallHandler(name, inputArgs, numArgs);

        //cache attempted settings when not ready
        const bool isSetter = (name.size() > 3 and name.substr(0, 3) == "set");
        if (isSetter) _cachedArgs[name] = std::vector<Pothos::Object>(inputArgs, inputArgs+numArgs);
        else throw Pothos::Exception("SDRSource::"+name+"()", "device not ready");
        return Pothos::Object();
    }

    /*******************************************************************
     * Stream config
     ******************************************************************/
    void setupStream(const std::map<std::string, std::string> &streamArgs)
    {
        //create format string from the dtype
        std::string format;
        const auto dtype = this->output(0)->dtype();
        if (dtype.isComplex()) format += "C";
        if (dtype.isFloat()) format += "F";
        else if (dtype.isInteger() and dtype.isSigned()) format += "S";
        else if (dtype.isInteger() and not dtype.isSigned()) format += "U";
        size_t bits = dtype.elemSize()*8;
        if (dtype.isComplex()) bits /= 2;
        format += std::to_string(bits);

        //create the stream
        _stream = _device->setupStream(SOAPY_SDR_RX, format, _channels, streamArgs);
    }

    void setSampleRate(const double rate)
    {
        for (const size_t chan : _channels)
        {
            _device->setSampleRate(SOAPY_SDR_RX, chan, rate);
        }
    }

    double getSampleRate(void) const
    {
        return _device->getSampleRate(SOAPY_SDR_RX, _channels.front());
    }

    /*******************************************************************
     * Frontend config
     ******************************************************************/
    void setFrequency(const size_t chan, const double freq, const std::map<std::string, std::string> &args)
    {
        return _device->setFrequency(SOAPY_SDR_RX, _channels.at(chan), freq, args);
    }

    void setGain(const size_t chan, const std::string &name, const double gain)
    {
        return _device->setGain(SOAPY_SDR_RX, _channels.at(chan), name, gain);
    }

    void setAntenna(const size_t chan, const std::string &name)
    {
        return _device->setAntenna(SOAPY_SDR_RX, _channels.at(chan), name);
    }

    /*******************************************************************
     * Clocking config
     ******************************************************************/
    void setClockRate(const double rate)
    {
        if (rate == 0.0) return;
        return _device->setMasterClockRate(rate);
    }

    double getClockRate(void) const
    {
        return _device->getMasterClockRate();
    }

    void setClockSource(const std::string &source)
    {
        if (source.empty()) return;
        return _device->setClockSource(source);
    }

    std::string getClockSource(void) const
    {
        return _device->getClockSource();
    }

    void setTimeSource(const std::string &source)
    {
        if (source.empty()) return;
        return _device->setTimeSource(source);
    }

    std::string getTimeSource(void) const
    {
        return _device->getTimeSource();
    }

    /*******************************************************************
     * Streaming implementation
     ******************************************************************/
    void activate(void)
    {
        if (not this->isReady())
        {
            throw Pothos::Exception("SDRSource::activate()", "device not ready");
        }
        //TODO other arguments
        //TODO check result
        _device->activateStream(_stream);

        //TODO emit configuration...
    }

    void deactivate(void)
    {
        //TODO check result
        _device->deactivateStream(_stream);
    }

    void work(void)
    {
        int flags = 0;
        long long timeNs = 0;
        const size_t numElems = this->workInfo().minOutElements;
        const long timeoutUs = this->workInfo().maxTimeoutNs/1000;
        const auto &buffs = this->workInfo().outputPointers;
        const int ret = _device->readStream(_stream, buffs.data(), numElems, flags, timeNs, timeoutUs);

        //TODO labels

        if (ret > 0) for (auto output : this->outputs()) output->produce(numElems);
        else if (ret == SOAPY_SDR_TIMEOUT) return this->yield();
        else poco_error_f1(Poco::Logger::get("SDRSource"), "readStream %d", ret);
    }

private:

    bool isReady(void)
    {
        if (_device != nullptr) return true;
        if (_deviceFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return false;
        _device = _deviceFuture.get();

        //call the cached settings now that the device exists
        for (const auto &pair : _cachedArgs)
        {
            POTHOS_EXCEPTION_TRY
            {
                Pothos::Block::opaqueCallHandler(pair.first, pair.second.data(), pair.second.size());
            }
            POTHOS_EXCEPTION_CATCH (const Pothos::Exception &ex)
            {
                poco_error_f2(Poco::Logger::get("SDRSource"), "call %s threw: %s", pair.first, ex.displayText());
            }
        }

        return true;
    }

    const std::vector<size_t> _channels;
    SoapySDR::Device *_device;
    SoapySDR::Stream *_stream;

    std::map<std::string, std::vector<Pothos::Object>> _cachedArgs;
    std::shared_future<SoapySDR::Device *> _deviceFuture;
};


static Pothos::BlockRegistry registerSDRSource(
    "/sdr/source", &SDRSource::make);
