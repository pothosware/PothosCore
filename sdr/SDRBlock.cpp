// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SDRBlock.hpp"
#include <SoapySDR/Version.hpp>
#include <Poco/SingletonHolder.h>
#include <Poco/Format.h>
#include <Poco/Logger.h>
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
    this->registerCallable("setFrequency", Pothos::Callable::make<const double, void>(&SDRBlock::setFrequency).bind(std::ref(*this), 0));
    this->registerCallable("setFrequency", Pothos::Callable::make<const double, const Pothos::ObjectKwargs &>(&SDRBlock::setFrequency).bind(std::ref(*this), 0));
    this->registerCallable("setFrequency", Pothos::Callable::make<const std::vector<double> &, void>(&SDRBlock::setFrequency).bind(std::ref(*this), 0));
    this->registerCallable("setFrequency", Pothos::Callable::make<const std::vector<double> &, const Pothos::ObjectKwargs &>(&SDRBlock::setFrequency).bind(std::ref(*this), 0));
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
        this->registerCallable("setFrequency"+chanStr, Pothos::Callable::make<const size_t, double, void>(&SDRBlock::setFrequency).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("setFrequency"+chanStr, Pothos::Callable::make<const size_t, double, const Pothos::ObjectKwargs &>(&SDRBlock::setFrequency).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getFrequency"+chanStr, Pothos::Callable::make<const size_t, double>(&SDRBlock::getFrequency).bind(std::ref(*this), 0).bind(i, 1));
        //freq component by name
        this->registerCallable("setFrequency"+chanStr, Pothos::Callable::make<const size_t, const std::string &, double, void>(&SDRBlock::setFrequency).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("setFrequency"+chanStr, Pothos::Callable::make<const size_t, const std::string &, double, const Pothos::ObjectKwargs &>(&SDRBlock::setFrequency).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getFrequency"+chanStr, Pothos::Callable::make<const size_t, const std::string &, double>(&SDRBlock::getFrequency).bind(std::ref(*this), 0).bind(i, 1));
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
        //sensors
        this->registerCallable("getSensors"+chanStr, Pothos::Callable::make<const size_t>(&SDRBlock::getSensors).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getSensor"+chanStr, Pothos::Callable::make<const size_t>(&SDRBlock::getSensor).bind(std::ref(*this), 0).bind(i, 1));

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
        this->registerProbe("getSensors"+chanStr);
        this->registerProbe("getSensor"+chanStr);
    }

    //clocking
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setClockRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getClockRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setClockSource));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getClockSource));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getClockSources));

    //time
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setTimeSource));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getTimeSource));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getTimeSources));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setHardwareTime));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getHardwareTime));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setCommandTime));

    //sensors
    this->registerCallable("getSensors", Pothos::Callable::make<std::vector<std::string>>(&SDRBlock::getSensors).bind(std::ref(*this), 0));
    this->registerCallable("getSensor", Pothos::Callable::make<const std::string &>(&SDRBlock::getSensor).bind(std::ref(*this), 0));

    //gpio
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setGpioConfig));

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
    this->registerProbe("getGpioBanks");
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

/*******************************************************************
 * Device object creation
 ******************************************************************/
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

/*******************************************************************
 * Delayed method dispatch
 ******************************************************************/
Pothos::Object SDRBlock::opaqueCallHandler(const std::string &name, const Pothos::Object *inputArgs, const size_t numArgs)
{
    //try to setup the device future first
    if (name == "setupDevice") return Pothos::Block::opaqueCallHandler(name, inputArgs, numArgs);

    //when ready forward the call to the handler
    if (this->isReady()) return Pothos::Block::opaqueCallHandler(name, inputArgs, numArgs);

    //cache attempted settings when not ready
    const bool isSetter = (name.size() > 3 and name.substr(0, 3) == "set");
    if (isSetter) _cachedArgs.push_back(std::make_pair(name, Pothos::ObjectVector(inputArgs, inputArgs+numArgs)));
    else throw Pothos::Exception("SDRBlock::"+name+"()", "device not ready");
    return Pothos::Object();
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

/*******************************************************************
 * Stream config
 ******************************************************************/
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

void SDRBlock::setSampleRate(const double rate)
{
    for (size_t i = 0; i < _channels.size(); i++)
    {
        _device->setSampleRate(_direction, _channels.at(i), rate);
        _pendingLabels[i]["rxRate"] = Pothos::Object(_device->getSampleRate(_direction, _channels.at(i)));
    }
}

double SDRBlock::getSampleRate(void) const
{
    return _device->getSampleRate(_direction, _channels.front());
}

std::vector<double> SDRBlock::getSampleRates(void) const
{
    return _device->listSampleRates(_direction, _channels.front());
}

void SDRBlock::setAutoActivate(const bool autoActivate)
{
    _autoActivate = autoActivate;
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

/*******************************************************************
 * Frontend map
 ******************************************************************/
void SDRBlock::setFrontendMap(const std::string &mapping)
{
    if (mapping.empty()) return;
    return _device->setFrontendMapping(_direction, mapping);
}

std::string SDRBlock::getFrontendMap(void) const
{
    return _device->getFrontendMapping(_direction);
}

/*******************************************************************
 * Frequency
 ******************************************************************/

//-------- setFrequency(no tune args) ----------//

void SDRBlock::setFrequency(const double freq)
{
    this->setFrequency(freq, _cachedTuneArgs[0]);
}

void SDRBlock::setFrequency(const std::vector<double> &freqs)
{
    this->setFrequency(freqs, _cachedTuneArgs[0]);
}

void SDRBlock::setFrequency(const size_t chan, const double freq)
{
    this->setFrequency(chan, freq, _cachedTuneArgs[chan]);
}

void SDRBlock::setFrequency(const size_t chan, const std::string &name, const double freq)
{
    this->setFrequency(chan, name, freq, _cachedTuneArgs[chan]);
}

//-------- setFrequency(tune args) ----------//

void SDRBlock::setFrequency(const double freq, const Pothos::ObjectKwargs &args)
{
    for (size_t i = 0; i < _channels.size(); i++) this->setFrequency(i, freq, args);
}

void SDRBlock::setFrequency(const std::vector<double> &freqs, const Pothos::ObjectKwargs &args)
{
    for (size_t i = 0; i < freqs.size(); i++) this->setFrequency(i, freqs[i], args);
}

void SDRBlock::setFrequency(const size_t chan, const double freq, const Pothos::ObjectKwargs &args)
{
    if (chan >= _channels.size()) return;
    _cachedTuneArgs[chan] = args;
    _device->setFrequency(_direction, _channels.at(chan), freq, _toKwargs(args));
    _pendingLabels[chan]["rxFreq"] = Pothos::Object(_device->getFrequency(_direction, _channels.at(chan)));
}

void SDRBlock::setFrequency(const size_t chan, const std::string &name, const double freq, const Pothos::ObjectKwargs &args)
{
    if (chan >= _channels.size()) return;
    _cachedTuneArgs[chan] = args;
    _device->setFrequency(_direction, _channels.at(chan), name, freq, _toKwargs(args));
}

//-------- getFrequency ----------//

double SDRBlock::getFrequency(const size_t chan) const
{
    if (chan >= _channels.size()) return 0.0;
    return _device->getFrequency(_direction, _channels.at(chan));
}

double SDRBlock::getFrequency(const size_t chan, const std::string &name) const
{
    if (chan >= _channels.size()) return 0.0;
    return _device->getFrequency(_direction, _channels.at(chan), name);
}

/*******************************************************************
 * Gain mode
 ******************************************************************/
void SDRBlock::setGainMode(const bool automatic)
{
    for (size_t i = 0; i < _channels.size(); i++) this->setGainMode(i, automatic);
}

void SDRBlock::setGainMode(const std::vector<bool> &automatic)
{
    for (size_t i = 0; i < automatic.size(); i++) this->setGainMode(i, automatic[i]);
}

void SDRBlock::setGainMode(const size_t chan, const bool automatic)
{
    if (chan >= _channels.size()) return;
    return _device->setGainMode(_direction, _channels.at(chan), automatic);
}

double SDRBlock::getGainMode(const size_t chan) const
{
    if (chan >= _channels.size()) return false;
    return _device->getGainMode(_direction, _channels.at(chan));
}

/*******************************************************************
 * Gain
 ******************************************************************/
void SDRBlock::setGain(const double gain)
{
    for (size_t i = 0; i < _channels.size(); i++) this->setGain(i, gain);
}

void SDRBlock::setGain(const Pothos::ObjectMap &gain)
{
    for (size_t i = 0; i < _channels.size(); i++) this->setGain(i, gain);
}

void SDRBlock::setGain(const Pothos::ObjectVector &gains)
{
    for (size_t i = 0; i < gains.size(); i++)
    {
        if (gains[i].canConvert(typeid(Pothos::ObjectMap))) this->setGain(i, gains[i].convert<Pothos::ObjectMap>());
        else this->setGain(i, gains[i].convert<double>());
    }
}

void SDRBlock::setGain(const size_t chan, const std::string &name, const double gain)
{
    if (chan >= _channels.size()) return;
    return _device->setGain(_direction, _channels.at(chan), name, gain);
}

double SDRBlock::getGain(const size_t chan, const std::string &name) const
{
    if (chan >= _channels.size()) return 0.0;
    return _device->getGain(_direction, _channels.at(chan), name);
}

void SDRBlock::setGain(const size_t chan, const double gain)
{
    if (chan >= _channels.size()) return;
    return _device->setGain(_direction, _channels.at(chan), gain);
}

double SDRBlock::getGain(const size_t chan) const
{
    if (chan >= _channels.size()) return 0.0;
    return _device->getGain(_direction, _channels.at(chan));
}

void SDRBlock::setGain(const size_t chan, const Pothos::ObjectMap &args)
{
    if (chan >= _channels.size()) return;
    for (const auto &pair : args)
    {
        const auto name = pair.first.convert<std::string>();
        const auto gain = pair.second.convert<double>();
        _device->setGain(_direction, _channels.at(chan), name, gain);
    }
}

std::vector<std::string> SDRBlock::getGainNames(const size_t chan) const
{
    if (chan >= _channels.size()) return std::vector<std::string>();
    return _device->listGains(_direction, _channels.at(chan));
}

/*******************************************************************
 * Antennas
 ******************************************************************/
void SDRBlock::setAntenna(const std::string &name)
{
    for (size_t i = 0; i < _channels.size(); i++) this->setAntenna(i, name);
}

void SDRBlock::setAntenna(const std::vector<std::string> &names)
{
    for (size_t i = 0; i < names.size(); i++) this->setAntenna(i, names[i]);
}

void SDRBlock::setAntenna(const size_t chan, const std::string &name)
{
    if (chan >= _channels.size()) return;
    if (name.empty()) return;
    return _device->setAntenna(_direction, _channels.at(chan), name);
}

std::string SDRBlock::getAntenna(const size_t chan) const
{
    if (chan >= _channels.size()) return "";
    return _device->getAntenna(_direction, _channels.at(chan));
}

std::vector<std::string> SDRBlock::getAntennas(const size_t chan) const
{
    if (chan >= _channels.size()) return std::vector<std::string>();
    return _device->listAntennas(_direction, _channels.at(chan));
}

/*******************************************************************
 * Bandwidth
 ******************************************************************/
void SDRBlock::setBandwidth(const double bandwidth)
{
    for (size_t i = 0; i < _channels.size(); i++) this->setBandwidth(i, bandwidth);
}

void SDRBlock::setBandwidth(const std::vector<double> &bandwidths)
{
    for (size_t i = 0; i < bandwidths.size(); i++) this->setBandwidth(i, bandwidths[i]);
}

void SDRBlock::setBandwidth(const size_t chan, const double bandwidth)
{
    if (chan >= _channels.size()) return;
    if (bandwidth == 0) return;
    return _device->setBandwidth(_direction, _channels.at(chan), bandwidth);
}

double SDRBlock::getBandwidth(const size_t chan) const
{
    if (chan >= _channels.size()) return 0.0;
    return _device->getBandwidth(_direction, _channels.at(chan));
}

std::vector<double> SDRBlock::getBandwidths(const size_t chan) const
{
    if (chan >= _channels.size()) return std::vector<double>();
    return _device->listBandwidths(_direction, _channels.at(chan));
}

/*******************************************************************
 * DC offset mode
 ******************************************************************/
void SDRBlock::setDCOffsetMode(const bool automatic)
{
    for (size_t i = 0; i < _channels.size(); i++) this->setDCOffsetMode(i, automatic);
}

void SDRBlock::setDCOffsetMode(const std::vector<bool> &automatic)
{
    for (size_t i = 0; i < automatic.size(); i++) this->setDCOffsetMode(i, automatic[i]);
}

void SDRBlock::setDCOffsetMode(const size_t chan, const bool automatic)
{
    if (chan >= _channels.size()) return;
    return _device->setDCOffsetMode(_direction, _channels.at(chan), automatic);
}

bool SDRBlock::getDCOffsetMode(const size_t chan) const
{
    if (chan >= _channels.size()) return 0.0;
    return _device->getDCOffsetMode(_direction, _channels.at(chan));
}

/*******************************************************************
 * DC offset adjust
 ******************************************************************/
void SDRBlock::setDCOffsetAdjust(const std::complex<double> &correction)
{
    for (size_t i = 0; i < _channels.size(); i++) this->setDCOffsetAdjust(i, correction);
}

void SDRBlock::setDCOffsetAdjust(const size_t chan, const std::complex<double> &correction)
{
    if (chan >= _channels.size()) return;
    return _device->setDCOffset(_direction, _channels.at(chan), correction);
}

std::complex<double> SDRBlock::getDCOffsetAdjust(const size_t chan) const
{
    if (chan >= _channels.size()) return 0.0;
    return _device->getDCOffset(_direction, _channels.at(chan));
}

/*******************************************************************
 * Clocking config
 ******************************************************************/
void SDRBlock::setClockRate(const double rate)
{
    if (rate == 0.0) return;
    return _device->setMasterClockRate(rate);
}

double SDRBlock::getClockRate(void) const
{
    return _device->getMasterClockRate();
}

void SDRBlock::setClockSource(const std::string &source)
{
    if (source.empty()) return;
    return _device->setClockSource(source);
}

std::string SDRBlock::getClockSource(void) const
{
    return _device->getClockSource();
}

std::vector<std::string> SDRBlock::getClockSources(void) const
{
    return _device->listClockSources();
}

/*******************************************************************
 * Timing
 ******************************************************************/
void SDRBlock::setTimeSource(const std::string &source)
{
    if (source.empty()) return;
    return _device->setTimeSource(source);
}

std::string SDRBlock::getTimeSource(void) const
{
    return _device->getTimeSource();
}

std::vector<std::string> SDRBlock::getTimeSources(void) const
{
    return _device->listTimeSources();
}

void SDRBlock::setHardwareTime(const long long timeNs)
{
    return _device->setHardwareTime(timeNs);
}

long long SDRBlock::getHardwareTime(void) const
{
    return _device->getHardwareTime();
}

void SDRBlock::setCommandTime(const long long timeNs)
{
    return _device->setCommandTime(timeNs);
}

/*******************************************************************
 * Sensors
 ******************************************************************/
std::vector<std::string> SDRBlock::getSensors(void) const
{
    return _device->listSensors();
}

std::string SDRBlock::getSensor(const std::string &name) const
{
    return _device->readSensor(name);
}

std::vector<std::string> SDRBlock::getSensors(const size_t chan) const
{
    return _device->listSensors(_direction, chan);
}

std::string SDRBlock::getSensor(const size_t chan, const std::string &name) const
{
    return _device->readSensor(_direction, chan, name);
}

/*******************************************************************
 * GPIO
 ******************************************************************/
std::vector<std::string> SDRBlock::getGpioBanks(void) const
{
    return _device->listGPIOBanks();
}

void SDRBlock::setGpioConfig(const Pothos::ObjectKwargs &config)
{
    //nested maps represent each bank
    for (const auto &pair : config)
    {
        if (pair.second.canConvert(typeid(Pothos::ObjectKwargs)))
        {
            setGpioConfigBank(pair.first, pair.second.convert<Pothos::ObjectKwargs>());
        }
    }

    //when a bank name is specified in the config
    const auto bankIt = config.find("bank");
    if (bankIt != config.end())
    {
        setGpioConfigBank(bankIt->second.convert<std::string>(), config);
    }
}

void SDRBlock::setGpioConfigBank(const std::string &bank, const Pothos::ObjectKwargs &config)
{
    const auto dirIt = config.find("dir");
    const auto maskIt = config.find("mask");
    const auto valueIt = config.find("value");

    //set data direction without mask
    if (dirIt != config.end() and maskIt == config.end())
    {
        _device->writeGPIODir(bank, dirIt->second.convert<unsigned>());
    }

    //set data direction with mask
    if (dirIt != config.end() and maskIt != config.end())
    {
        _device->writeGPIODir(bank, dirIt->second.convert<unsigned>(), maskIt->second.convert<unsigned>());
    }

    //set GPIO value without mask
    if (valueIt != config.end() and maskIt == config.end())
    {
        _device->writeGPIO(bank, valueIt->second.convert<unsigned>());
    }

    //set valueIt value with mask
    if (dirIt != config.end() and maskIt != config.end())
    {
        _device->writeGPIO(bank, valueIt->second.convert<unsigned>(), maskIt->second.convert<unsigned>());
    }
}

unsigned SDRBlock::getGpioValue(const std::string &bank) const
{
    return _device->readGPIO(bank);
}

/*******************************************************************
 * Streaming implementation
 ******************************************************************/
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
