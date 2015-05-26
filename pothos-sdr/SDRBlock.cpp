// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SDRBlock.hpp"
#include <SoapySDR/Version.hpp>
#include <Poco/SingletonHolder.h>
#include <mutex>
#include <cassert>

SDRBlock::SDRBlock(const int direction, const Pothos::DType &dtype, const std::vector<size_t> &chs):
    _autoActivate(true),
    _direction(direction),
    _dtype(dtype),
    _channels(chs.empty()?std::vector<size_t>(1, 0):chs),
    _device(nullptr),
    _stream(nullptr),
    _pendingLabels(_channels.size())
{
    assert(not _channels.empty());
    if (SoapySDR::getABIVersion() != SOAPY_SDR_ABI_VERSION) throw Pothos::Exception("SDRBlock::make()",
        Poco::format("Failed ABI check. Pothos SDR %s. Soapy SDR %s. Rebuild the module.",
        std::string(SOAPY_SDR_ABI_VERSION), SoapySDR::getABIVersion()));

    //streaming
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setupDevice));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setupStream));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setSampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getSampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getSampleRates));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setFrontendMap));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getFrontendMap));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setAutoActivate));
    this->registerCallable("streamControl", Pothos::Callable(&SDRBlock::streamControl).bind(std::ref(*this), 0)); //3 arg version
    this->registerCallable("streamControl", Pothos::Callable(&SDRBlock::streamControl).bind(std::ref(*this), 0).bind(0, 3)); //2 arg version
    this->registerCallable("streamControl", Pothos::Callable(&SDRBlock::streamControl).bind(std::ref(*this), 0).bind(0, 2).bind(0, 3)); //1 arg version

    //channels -- called by setters
    this->registerCallable("setFrequency", Pothos::Callable::make<const double>(&SDRBlock::setFrequency).bind(std::ref(*this), 0));
    this->registerCallable("setFrequency", Pothos::Callable::make<const double>(&SDRBlock::setFrequency).bind(std::ref(*this), 0).bind(Pothos::ObjectKwargs(), 2));
    this->registerCallable("setFrequency", Pothos::Callable::make<const std::vector<double> &>(&SDRBlock::setFrequency).bind(std::ref(*this), 0));
    this->registerCallable("setFrequency", Pothos::Callable::make<const std::vector<double> &>(&SDRBlock::setFrequency).bind(std::ref(*this), 0).bind(Pothos::ObjectKwargs(), 2));
    this->registerCallable("setGainMode", Pothos::Callable::make<const bool>(&SDRBlock::setGainMode).bind(std::ref(*this), 0));
    this->registerCallable("setGainMode", Pothos::Callable::make<const std::vector<bool> &>(&SDRBlock::setGainMode).bind(std::ref(*this), 0));
    this->registerCallable("setGain", Pothos::Callable::make<const double>(&SDRBlock::setGain).bind(std::ref(*this), 0));
    this->registerCallable("setGain", Pothos::Callable::make<const Pothos::ObjectMap &>(&SDRBlock::setGain).bind(std::ref(*this), 0));
    this->registerCallable("setGain", Pothos::Callable::make<const Pothos::ObjectVector &>(&SDRBlock::setGain).bind(std::ref(*this), 0));
    this->registerCallable("setAntenna", Pothos::Callable::make<const std::string &>(&SDRBlock::setAntenna).bind(std::ref(*this), 0));
    this->registerCallable("setAntenna", Pothos::Callable::make<const std::vector<std::string> &>(&SDRBlock::setAntenna).bind(std::ref(*this), 0));
    this->registerCallable("setBandwidth", Pothos::Callable::make<const double>(&SDRBlock::setBandwidth).bind(std::ref(*this), 0));
    this->registerCallable("setBandwidth", Pothos::Callable::make<const std::vector<double> &>(&SDRBlock::setBandwidth).bind(std::ref(*this), 0));
    this->registerCallable("setDCOffsetMode", Pothos::Callable::make<const bool>(&SDRBlock::setDCOffsetMode).bind(std::ref(*this), 0));
    this->registerCallable("setDCOffsetMode", Pothos::Callable::make<const std::vector<bool> &>(&SDRBlock::setDCOffsetMode).bind(std::ref(*this), 0));
    this->registerCallable("setDCOffsetAdjust", Pothos::Callable::make<const std::complex<double> &>(&SDRBlock::setDCOffsetAdjust).bind(std::ref(*this), 0));

    //channels
    for (size_t i = 0; i < _channels.size(); i++)
    {
        const auto chanStr = std::to_string(i);
        //freq overall with tune args
        this->registerCallable("setFrequency"+chanStr, Pothos::Callable::make<const size_t, double>(&SDRBlock::setFrequency).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("setFrequency"+chanStr, Pothos::Callable::make<const size_t, double>(&SDRBlock::setFrequency).bind(std::ref(*this), 0).bind(i, 1).bind(Pothos::ObjectKwargs(), 3));
        this->registerCallable("getFrequency"+chanStr, Pothos::Callable::make<const size_t, double>(&SDRBlock::getFrequency).bind(std::ref(*this), 0).bind(i, 1));
        //freq component by name
        this->registerCallable("setFrequency"+chanStr, Pothos::Callable::make<const size_t, const std::string &>(&SDRBlock::setFrequency).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("setFrequency"+chanStr, Pothos::Callable::make<const size_t, const std::string &>(&SDRBlock::setFrequency).bind(std::ref(*this), 0).bind(i, 1).bind(Pothos::ObjectKwargs(), 4));
        this->registerCallable("getFrequency"+chanStr, Pothos::Callable::make<const size_t, const std::string &>(&SDRBlock::getFrequency).bind(std::ref(*this), 0).bind(i, 1));
        //gain by name
        this->registerCallable("setGain"+chanStr, Pothos::Callable::make<const size_t, const std::string &>(&SDRBlock::setGain).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getGain"+chanStr, Pothos::Callable::make<const size_t, const std::string &>(&SDRBlock::getGain).bind(std::ref(*this), 0).bind(i, 1));
        //gain overall
        this->registerCallable("setGain"+chanStr, Pothos::Callable::make<const size_t, const double>(&SDRBlock::setGain).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getGain"+chanStr, Pothos::Callable::make<const size_t, double>(&SDRBlock::getGain).bind(std::ref(*this), 0).bind(i, 1));
        //gain dict
        this->registerCallable("setGain"+chanStr, Pothos::Callable::make<const size_t, const Pothos::ObjectMap &>(&SDRBlock::setGain).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getGainNames"+chanStr, Pothos::Callable::make<const size_t>(&SDRBlock::getGainNames).bind(std::ref(*this), 0).bind(i, 1));
        //gain mode
        this->registerCallable("setGainMode"+chanStr, Pothos::Callable::make<const size_t>(&SDRBlock::setGainMode).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getGainMode"+chanStr, Pothos::Callable::make<const size_t>(&SDRBlock::setGainMode).bind(std::ref(*this), 0).bind(i, 1));
        //antenna
        this->registerCallable("setAntenna"+chanStr, Pothos::Callable::make<const size_t>(&SDRBlock::setAntenna).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getAntenna"+chanStr, Pothos::Callable::make<const size_t>(&SDRBlock::getAntenna).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getAntennas"+chanStr, Pothos::Callable::make<const size_t>(&SDRBlock::getAntennas).bind(std::ref(*this), 0).bind(i, 1));
        //bandwidth
        this->registerCallable("setBandwidth"+chanStr, Pothos::Callable::make<const size_t>(&SDRBlock::setBandwidth).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getBandwidth"+chanStr, Pothos::Callable::make<const size_t>(&SDRBlock::getBandwidth).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getBandwidths"+chanStr, Pothos::Callable::make<const size_t>(&SDRBlock::getBandwidths).bind(std::ref(*this), 0).bind(i, 1));
        //dc offset mode
        this->registerCallable("setDCOffsetMode"+chanStr, Pothos::Callable::make<const size_t>(&SDRBlock::setDCOffsetMode).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getDCOffsetMode"+chanStr, Pothos::Callable::make<const size_t>(&SDRBlock::getDCOffsetMode).bind(std::ref(*this), 0).bind(i, 1));
        //dc offset adjust
        this->registerCallable("setDCOffsetAdjust"+chanStr, Pothos::Callable::make<const size_t>(&SDRBlock::setDCOffsetAdjust).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getDCOffsetAdjust"+chanStr, Pothos::Callable::make<const size_t>(&SDRBlock::getDCOffsetAdjust).bind(std::ref(*this), 0).bind(i, 1));

        //channel probes
        this->registerProbe("getFrequency"+chanStr);
        this->registerProbe("getGain"+chanStr);
        this->registerProbe("getGainNames"+chanStr);
        this->registerProbe("getGainMode"+chanStr);
        this->registerProbe("getAntenna"+chanStr);
        this->registerProbe("getAntennas"+chanStr);
        this->registerProbe("getBandwidth"+chanStr);
        this->registerProbe("getBandwidths"+chanStr);
        this->registerProbe("getDCOffsetMode"+chanStr);
        this->registerProbe("getDCOffsetAdjust"+chanStr);
    }

    //clocking
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setClockRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getClockRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setClockSource));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getClockSource));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getClockSources));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setTimeSource));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getTimeSource));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getTimeSources));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setHardwareTime));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getHardwareTime));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setCommandTime));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getSensor));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getSensors));

    //probes
    this->registerProbe("getSampleRate");
    this->registerProbe("getSampleRates");
    this->registerProbe("getFrontendMap");
    this->registerProbe("getClockRate");
    this->registerProbe("getClockSource");
    this->registerProbe("getClockSources");
    this->registerProbe("getTimeSource");
    this->registerProbe("getTimeSources");
    this->registerProbe("getHardwareTime");
    this->registerProbe("getSensor");
    this->registerProbe("getSensors");
}

static std::mutex &getMutex(void)
{
    static Poco::SingletonHolder<std::mutex> sh;
    return *sh.get();
}

SDRBlock::~SDRBlock(void)
{
    //close the stream, the stream should be stopped by deactivate
    //but this actually cleans up and frees the stream object
    if (_stream != nullptr) _device->closeStream(_stream);

    //if for some reason we didn't complete the future
    //we have to wait on it here and catch all errors
    try {_device = _deviceFuture.get();} catch (...){}

    //now with the mutex locked, the device object can be released
    std::unique_lock<std::mutex> lock(getMutex());
    if (_device != nullptr) SoapySDR::Device::unmake(_device);
}

static SoapySDR::Device *makeDevice(const SoapySDR::Kwargs &deviceArgs)
{
    //protect device make -- its not thread safe
    std::unique_lock<std::mutex> lock(getMutex());
    return SoapySDR::Device::make(deviceArgs);
}

void SDRBlock::setupDevice(const Pothos::ObjectKwargs &deviceArgs)
{
    _deviceFuture = std::async(std::launch::async, &makeDevice, _toKwargs(deviceArgs));
}

Pothos::Object SDRBlock::opaqueCallHandler(const std::string &name, const Pothos::Object *inputArgs, const size_t numArgs)
{
    //try to setup the device future first
    if (name == "setupDevice") return Pothos::Block::opaqueCallHandler(name, inputArgs, numArgs);

    //when ready forward the call to the handler
    if (this->isReady()) return Pothos::Block::opaqueCallHandler(name, inputArgs, numArgs);

    //cache attempted settings when not ready
    const bool isSetter = (name.size() > 3 and name.substr(0, 3) == "set");
    if (isSetter) _cachedArgs[name] = std::vector<Pothos::Object>(inputArgs, inputArgs+numArgs);
    else throw Pothos::Exception("SDRBlock::"+name+"()", "device not ready");
    return Pothos::Object();
}

void SDRBlock::setupStream(const Pothos::ObjectKwargs &streamArgs)
{
    //create format string from the dtype
    std::string format;
    if (_dtype.isComplex()) format += "C";
    if (_dtype.isFloat()) format += "F";
    else if (_dtype.isInteger() and _dtype.isSigned()) format += "S";
    else if (_dtype.isInteger() and not _dtype.isSigned()) format += "U";
    size_t bits = _dtype.elemSize()*8;
    if (_dtype.isComplex()) bits /= 2;
    format += std::to_string(bits);

    //create the stream
    _stream = _device->setupStream(_direction, format, _channels, _toKwargs(streamArgs));
}

bool SDRBlock::isReady(void)
{
    if (_device != nullptr) return true;
    if (_deviceFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return false;
    _device = _deviceFuture.get();
    assert(_device != nullptr);

    //call the cached settings now that the device exists
    for (const auto &pair : _cachedArgs)
    {
        POTHOS_EXCEPTION_TRY
        {
            Pothos::Block::opaqueCallHandler(pair.first, pair.second.data(), pair.second.size());
        }
        POTHOS_EXCEPTION_CATCH (const Pothos::Exception &ex)
        {
            poco_error_f2(Poco::Logger::get("SDRBlock"), "call %s threw: %s", pair.first, ex.displayText());
        }
    }

    return true;
}

void SDRBlock::emitActivationSignals(void)
{
    this->callVoid("getSampleRateTriggered", this->getSampleRate());
    this->callVoid("getSampleRatesTriggered", this->getSampleRates());
    this->callVoid("getFrontendMapTriggered", this->getFrontendMap());
    this->callVoid("getClockRateTriggered", this->getClockRate());
    this->callVoid("getClockSourceTriggered", this->getClockSource());
    this->callVoid("getClockSourcesTriggered", this->getClockSources());
    this->callVoid("getTimeSourceTriggered", this->getTimeSource());
    this->callVoid("getTimeSourcesTriggered", this->getTimeSources());
    this->callVoid("getHardwareTimeTriggered", this->getHardwareTime());
    this->callVoid("getSensorsTriggered", this->getSensors());
    for (size_t i = 0; i < _channels.size(); i++)
    {
        const auto chanStr = std::to_string(i);
        this->callVoid("getFrequency"+chanStr+"Triggered", this->getFrequency(i));
        this->callVoid("getGain"+chanStr+"Triggered", this->getGain(i));
        this->callVoid("getGainNames"+chanStr+"Triggered", this->getGainNames(i));
        this->callVoid("getGainMode"+chanStr+"Triggered", this->getGainMode(i));
        this->callVoid("getAntenna"+chanStr+"Triggered", this->getAntenna(i));
        this->callVoid("getAntennas"+chanStr+"Triggered", this->getAntennas(i));
        this->callVoid("getBandwidth"+chanStr+"Triggered", this->getBandwidth(i));
        this->callVoid("getBandwidths"+chanStr+"Triggered", this->getBandwidths(i));
        this->callVoid("getDCOffsetMode"+chanStr+"Triggered", this->getDCOffsetMode(i));
    }
}

void SDRBlock::activate(void)
{
    if (not this->isReady()) throw Pothos::Exception("SDRSource::activate()", "device not ready");

    if (_autoActivate)
    {
        int ret = 0;
        //use time alignment for multi-channel rx setup when hardware time is supported
        if (_device->hasHardwareTime() and _direction == SOAPY_SDR_RX)
        {
            const auto delta = long(1e9*0.05); //50ms in the future for time-aligned streaming
            ret = _device->activateStream(_stream, SOAPY_SDR_HAS_TIME, _device->getHardwareTime()+delta);
        }
        else
        {
            ret = _device->activateStream(_stream);
        }
        if (ret != 0) throw Pothos::Exception("SDRBlock::activate()", "activateStream returned " + std::to_string(ret));
    }

    this->emitActivationSignals();
}

void SDRBlock::deactivate(void)
{
    const int ret = _device->deactivateStream(_stream);
    if (ret != 0) throw Pothos::Exception("SDRBlock::activate()", "deactivateStream returned " + std::to_string(ret));
}

void SDRBlock::streamControl(const std::string &what, const long long timeNs, const size_t numElems)
{
    int r = 0;
    if (what == "ACTIVATE")          r = _device->activateStream(_stream, 0, timeNs, numElems);
    if (what == "ACTIVATE_AT")       r = _device->activateStream(_stream, SOAPY_SDR_HAS_TIME, timeNs, numElems);
    if (what == "ACTIVATE_BURST")    r = _device->activateStream(_stream, SOAPY_SDR_END_BURST, timeNs, numElems);
    if (what == "ACTIVATE_BURST_AT") r = _device->activateStream(_stream, SOAPY_SDR_HAS_TIME | SOAPY_SDR_END_BURST, timeNs, numElems);
    if (what == "DEACTIVATE")        r = _device->deactivateStream(_stream, 0, timeNs);
    if (what == "DEACTIVATE_AT")     r = _device->deactivateStream(_stream, SOAPY_SDR_HAS_TIME, timeNs);
    if (r != 0) throw Pothos::Exception("SDRBlock::streamControl("+what+")", "de/activateStream returned " + std::to_string(r));
}
