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
 * |preview disable
 * |tab Streaming
 *
 * |param sampleRate[Sample Rate] The rate of sample stream on each channel.
 * |units Sps
 * |default 1e6
 * |tab Streaming
 *
 * |param clockRate[Clock rate] The master reference clock rate.
 * |default 0.0
 * |units Hz
 * |preview disable
 * |tab Clocking
 *
 * |param clockSource[Clock Source] The source of the reference clock.
 * |default ""
 * |preview disable
 * |tab Clocking
 * |widget StringEntry()
 *
 * |param timeSource[Time Source] The source of the hardware time.
 * |default ""
 * |preview disable
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
        this->registerCall(this, POTHOS_FCN_TUPLE(SDRSource, setupDevice));
        this->registerCall(this, POTHOS_FCN_TUPLE(SDRSource, setupStream));
        this->registerCall(this, POTHOS_FCN_TUPLE(SDRSource, setSampleRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(SDRSource, getSampleRate));
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

    static SoapySDR::Device *makeDevice(const std::map<std::string, std::string> &deviceArgs)
    {
        return SoapySDR::Device::make(deviceArgs);
    }

    void setupDevice(const std::map<std::string, std::string> &deviceArgs)
    {
        _deviceFuture = std::async(std::launch::async, &SDRSource::makeDevice, deviceArgs);
    }

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

    void activate(void)
    {
        if (not this->isReady())
        {
            throw Pothos::Exception("SDRSource::activate()", "device not ready");
        }
        //TODO other arguments
        _device->activateStream(_stream);
    }

    void deactivate(void)
    {
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
