// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SDRBlock.hpp"

SDRBlock::SDRBlock(const int direction, const std::vector<size_t> &channels):
    _direction(direction),
    _channels(channels),
    _device(nullptr),
    _stream(nullptr)
{
    //streaming
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setupDevice));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setupStream));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setSampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getSampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getSampleRates));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setFrontendMap));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getFrontendMap));

    //streaming probes
    /*
    this->registerProbe("getSampleRate");
    this->registerProbe("getSampleRates");
    this->registerProbe("getFrontendMap");
    */

    this->registerCallable("setFrequency", Pothos::Callable::make<const double>(&SDRBlock::setFrequency).bind(std::ref(*this), 0));
    this->registerCallable("setFrequency", Pothos::Callable::make<const std::vector<double> &>(&SDRBlock::setFrequency).bind(std::ref(*this), 0));
    this->registerCallable("setGainMode", Pothos::Callable::make<const bool>(&SDRBlock::setGainMode).bind(std::ref(*this), 0));
    this->registerCallable("setGainMode", Pothos::Callable::make<const std::vector<bool> &>(&SDRBlock::setGainMode).bind(std::ref(*this), 0));
    this->registerCallable("setGain", Pothos::Callable::make<const double>(&SDRBlock::setGain).bind(std::ref(*this), 0));
    this->registerCallable("setGain", Pothos::Callable::make<const Pothos::ObjectMap &>(&SDRBlock::setGain).bind(std::ref(*this), 0));
    this->registerCallable("setGain", Pothos::Callable::make<const Pothos::ObjectVector &>(&SDRBlock::setGain).bind(std::ref(*this), 0));
    this->registerCallable("setAntenna", Pothos::Callable::make<const std::string &>(&SDRBlock::setAntenna).bind(std::ref(*this), 0));
    this->registerCallable("setAntenna", Pothos::Callable::make<const std::vector<std::string> &>(&SDRBlock::setAntenna).bind(std::ref(*this), 0));
    this->registerCallable("setBandwidth", Pothos::Callable::make<const double>(&SDRBlock::setBandwidth).bind(std::ref(*this), 0));
    this->registerCallable("setBandwidth", Pothos::Callable::make<const std::vector<double> &>(&SDRBlock::setBandwidth).bind(std::ref(*this), 0));

    //all channels -- must have setters because of markup limitations
    /*
    for (size_t i = 0; i < 4; i++)
    {
        const auto chanStr = std::to_string(i);
        //freq with tune args
        this->registerCallable("setFrequency"+chanStr, Pothos::Callable(&SDRBlock::setFrequency).bind(std::ref(*this), 0).bind(i, 1));
        //gain overall
        this->registerCallable("setGain"+chanStr, Pothos::Callable::make<const size_t, const double>(&SDRBlock::setGain).bind(std::ref(*this), 0).bind(i, 1));
        //gain set dict
        this->registerCallable("setGain"+chanStr, Pothos::Callable::make<const size_t, const Pothos::ObjectMap &>(&SDRBlock::setGain).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("setGainMode"+chanStr, Pothos::Callable(&SDRBlock::setGainMode).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("setAntenna"+chanStr, Pothos::Callable(&SDRBlock::setAntenna).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("setBandwidth"+chanStr, Pothos::Callable(&SDRBlock::setBandwidth).bind(std::ref(*this), 0).bind(i, 1));
    }
    */

    //channels
    /*
    for (size_t i = 0; i < _channels.size(); i++)
    {
        const auto chanStr = std::to_string(i);
        //freq without tune args
        this->registerCallable("setFrequency"+chanStr, Pothos::Callable(&SDRBlock::setFrequency).bind(std::ref(*this), 0).bind(i, 1).bind(std::map<std::string, std::string>(), 3));
        this->registerCallable("getFrequency"+chanStr, Pothos::Callable(&SDRBlock::getFrequency).bind(std::ref(*this), 0).bind(i, 1));
        //gain by name
        this->registerCallable("setGain"+chanStr, Pothos::Callable::make<const size_t, const std::string &>(&SDRBlock::setGain).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getGain"+chanStr, Pothos::Callable::make<const size_t, const std::string &>(&SDRBlock::getGain).bind(std::ref(*this), 0).bind(i, 1));
        //gain overall
        this->registerCallable("getGain"+chanStr, Pothos::Callable::make<const size_t, double>(&SDRBlock::getGain).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getGainNames"+chanStr, Pothos::Callable(&SDRBlock::getGainNames).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getGainMode"+chanStr, Pothos::Callable(&SDRBlock::setGainMode).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getAntenna"+chanStr, Pothos::Callable(&SDRBlock::getAntenna).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getAntennas"+chanStr, Pothos::Callable(&SDRBlock::getAntennas).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getBandwidth"+chanStr, Pothos::Callable(&SDRBlock::getBandwidth).bind(std::ref(*this), 0).bind(i, 1));
        this->registerCallable("getBandwidths"+chanStr, Pothos::Callable(&SDRBlock::getBandwidths).bind(std::ref(*this), 0).bind(i, 1));

        //channel probes
        this->registerProbe("getFrequency"+chanStr);
        this->registerProbe("getGain"+chanStr);
        this->registerProbe("getGainNames"+chanStr);
        this->registerProbe("getGainMode"+chanStr);
        this->registerProbe("getAntenna"+chanStr);
        this->registerProbe("getAntennas"+chanStr);
        this->registerProbe("getBandwidth"+chanStr);
        this->registerProbe("getBandwidths"+chanStr);
    }
    */

    //clocking
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setClockRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getClockRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setClockSource));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getClockSource));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getClockSources));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, setTimeSource));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getTimeSource));
    this->registerCall(this, POTHOS_FCN_TUPLE(SDRBlock, getTimeSources));

    //clocking probes
    /*
    this->registerProbe("getClockRate");
    this->registerProbe("getClockSource");
    this->registerProbe("getClockSources");
    this->registerProbe("getTimeSource");
    this->registerProbe("getTimeSources");
    */
}

SDRBlock::~SDRBlock(void)
{
    if (_stream != nullptr) _device->closeStream(_stream);
    if (_device != nullptr) SoapySDR::Device::unmake(_device);
}
