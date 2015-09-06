// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Object/Containers.hpp>
#include <SoapySDR/Device.hpp>
#include <future>

class SDRBlock : public Pothos::Block
{
public:
    SDRBlock(const int direction, const Pothos::DType &dtype, const std::vector<size_t> &channels);
    virtual ~SDRBlock(void);

    /*******************************************************************
     * Device object creation
     ******************************************************************/
    void setupDevice(const Pothos::ObjectKwargs &deviceArgs);

    /*******************************************************************
     * Delayed method dispatch
     ******************************************************************/
    Pothos::Object opaqueCallHandler(const std::string &name, const Pothos::Object *inputArgs, const size_t numArgs);

    /*******************************************************************
     * Stream config
     ******************************************************************/
    void setupStream(const Pothos::ObjectKwargs &streamArgs);

    void setSampleRate(const double rate);

    double getSampleRate(void) const;

    std::vector<double> getSampleRates(void) const;

    void setAutoActivate(const bool autoActivate);

    void streamControl(const std::string &what, const long long timeNs, const size_t numElems);

    /*******************************************************************
     * Frontend map
     ******************************************************************/
    void setFrontendMap(const std::string &mapping);

    std::string getFrontendMap(void) const;

    /*******************************************************************
     * Frequency
     ******************************************************************/

    //-------- setFrequency(no tune args) ----------//

    void setFrequency(const double freq);

    void setFrequency(const std::vector<double> &freqs);

    void setFrequency(const size_t chan, const double freq);

    void setFrequency(const size_t chan, const std::string &name, const double freq);

    //-------- setFrequency(tune args) ----------//

    void setFrequency(const double freq, const Pothos::ObjectKwargs &args);

    void setFrequency(const std::vector<double> &freqs, const Pothos::ObjectKwargs &args);

    void setFrequency(const size_t chan, const double freq, const Pothos::ObjectKwargs &args);

    void setFrequency(const size_t chan, const std::string &name, const double freq, const Pothos::ObjectKwargs &args);

    //-------- getFrequency ----------//

    double getFrequency(const size_t chan) const;

    double getFrequency(const size_t chan, const std::string &name) const;

    /*******************************************************************
     * Gain mode
     ******************************************************************/
    void setGainMode(const bool automatic);

    void setGainMode(const std::vector<bool> &automatic);

    void setGainMode(const size_t chan, const bool automatic);

    double getGainMode(const size_t chan) const;

    /*******************************************************************
     * Gain
     ******************************************************************/
    void setGain(const double gain);

    void setGain(const Pothos::ObjectMap &gain);

    void setGain(const Pothos::ObjectVector &gains);

    void setGain(const size_t chan, const std::string &name, const double gain);

    double getGain(const size_t chan, const std::string &name) const;

    void setGain(const size_t chan, const double gain);

    double getGain(const size_t chan) const;

    void setGain(const size_t chan, const Pothos::ObjectMap &args);

    std::vector<std::string> getGainNames(const size_t chan) const;

    /*******************************************************************
     * Antennas
     ******************************************************************/
    void setAntenna(const std::string &name);

    void setAntenna(const std::vector<std::string> &names);

    void setAntenna(const size_t chan, const std::string &name);

    std::string getAntenna(const size_t chan) const;

    std::vector<std::string> getAntennas(const size_t chan) const;

    /*******************************************************************
     * Bandwidth
     ******************************************************************/
    void setBandwidth(const double bandwidth);

    void setBandwidth(const std::vector<double> &bandwidths);

    void setBandwidth(const size_t chan, const double bandwidth);

    double getBandwidth(const size_t chan) const;

    std::vector<double> getBandwidths(const size_t chan) const;

    /*******************************************************************
     * DC offset mode
     ******************************************************************/
    void setDCOffsetMode(const bool automatic);

    void setDCOffsetMode(const std::vector<bool> &automatic);

    void setDCOffsetMode(const size_t chan, const bool automatic);

    bool getDCOffsetMode(const size_t chan) const;

    /*******************************************************************
     * DC offset adjust
     ******************************************************************/
    void setDCOffsetAdjust(const std::complex<double> &correction);

    void setDCOffsetAdjust(const size_t chan, const std::complex<double> &correction);

    std::complex<double> getDCOffsetAdjust(const size_t chan) const;

    /*******************************************************************
     * Clocking config
     ******************************************************************/
    void setClockRate(const double rate);

    double getClockRate(void) const;

    void setClockSource(const std::string &source);

    std::string getClockSource(void) const;

    std::vector<std::string> getClockSources(void) const;

    /*******************************************************************
     * Timing
     ******************************************************************/
    void setTimeSource(const std::string &source);

    std::string getTimeSource(void) const;

    std::vector<std::string> getTimeSources(void) const;

    void setHardwareTime(const long long timeNs);

    long long getHardwareTime(void) const;

    void setCommandTime(const long long timeNs);

    /*******************************************************************
     * Sensors
     ******************************************************************/
    std::vector<std::string> getSensors(void) const;

    std::string getSensor(const std::string &name) const;

    std::vector<std::string> getSensors(const size_t chan) const;

    std::string getSensor(const size_t chan, const std::string &name) const;

    /*******************************************************************
     * GPIO
     ******************************************************************/
    std::vector<std::string> getGpioBanks(void) const;

    void setGpioConfig(const Pothos::ObjectKwargs &config);

    void setGpioConfig(const Pothos::ObjectVector &config);

    unsigned getGpioValue(const std::string &bank) const;

    /*******************************************************************
     * Streaming implementation
     ******************************************************************/
    virtual void activate(void);
    virtual void deactivate(void);
    virtual void work(void) = 0;

private:
    SoapySDR::Kwargs _toKwargs(const Pothos::ObjectKwargs &args)
    {
        SoapySDR::Kwargs kwargs;
        for (const auto &pair : args)
        {
            const auto val = pair.second;
            const auto valStr = (val.type() == typeid(std::string))?val.extract<std::string>():val.toString();
            kwargs[pair.first] = valStr;
        }
        return kwargs;
    }

protected:
    bool isReady(void);
    void emitActivationSignals(void);

    bool _autoActivate;
    const int _direction;
    const Pothos::DType _dtype;
    const std::vector<size_t> _channels;
    SoapySDR::Device *_device;
    SoapySDR::Stream *_stream;

    std::vector<std::pair<std::string, Pothos::ObjectVector>> _cachedArgs;
    std::shared_future<SoapySDR::Device *> _deviceFuture;

    std::vector<Pothos::ObjectKwargs> _pendingLabels;

    //Save the last tune args to re-use when slots are called without args.
    //This means that args can be set once at initialization and re-used.
    std::map<size_t, Pothos::ObjectKwargs> _cachedTuneArgs;
};
