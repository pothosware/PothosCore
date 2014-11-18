// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SpectrogramDisplay.hpp"
#include <Pothos/Framework.hpp>
#include <iostream>

/***********************************************************************
 * |PothosDoc Spectrogram
 *
 * The spectrogram plot displays a live plot of the spectrum vs time.
 *
 * |category /Widgets
 * |keywords frequency plot fft dft spectrum spectral
 *
 * |param title The title of the plot
 * |default "Spectrogram"
 * |widget StringEntry()
 *
 * |param displayRate[Display Rate] How often the plotter updates.
 * |default 10.0
 * |units updates/sec
 *
 * |param sampleRate[Sample Rate] The rate of the input elements.
 * |default 1e6
 * |units samples/sec
 *
 * |param centerFreq[Center Freq] The center frequency of the plot.
 * This value controls the labeling of the horizontal access.
 * |default 0.0
 * |units Hz
 *
 * |param numBins[Num FFT Bins] The number of bins per fourier transform.
 * |default 1024
 * |option 512
 * |option 1024
 * |option 2048
 * |option 4096
 * |widget ComboBox(editable=true)
 *
 * |param window[Window Type] The window function controls spectral leakage.
 * Enter "Kaiser(beta)" to use the parameterized Kaiser window.
 * |default "hann"
 * |option [Rectangular] "rectangular"
 * |option [Hann] "hann"
 * |option [Hamming] "hamming"
 * |option [Blackman] "blackman"
 * |option [Bartlett] "bartlett"
 * |option [Flat-top] "flattop"
 * |widget ComboBox(editable=true)
 *
 * |param timeSpan[Time Span] How many seconds of data to display in the plot.
 * |default 10.0
 * |units seconds
 *
 * |param refLevel[Reference Level] The maximum displayable power level.
 * |default 0.0
 * |units dBxx
 * |widget DoubleSpinBox(minimum=-150, maximum=150, step=10, decimals=1)
 * |preview disable
 *
 * |param dynRange[Dynamic Range] The ratio of largest to smallest displayable power level.
 * The vertical axis will display values from the ref level to ref level - dynamic range.
 * |default 100.0
 * |units dB
 * |widget DoubleSpinBox(minimum=10, maximum=150, step=10, decimals=1)
 * |preview disable
 *
 * |param enableXAxis[Enable X-Axis] Show or hide the horizontal axis markers.
 * |option [Show] true
 * |option [Hide] false
 * |default true
 * |preview disable
 *
 * |param enableYAxis[Enable Y-Axis] Show or hide the vertical axis markers.
 * |option [Show] true
 * |option [Hide] false
 * |default true
 * |preview disable
 *
 * |param freqLabelId[Freq Label ID] Labels with this ID can be used to set the center frequency.
 * To ignore frequency labels, set this parameter to an empty string.
 * |default "rxFreq"
 * |widget StringEntry()
 * |preview disable
 * |tab Labels
 *
 * |param rateLabelId[Rate Label ID] Labels with this ID can be used to set the sample rate.
 * To ignore sample rate labels, set this parameter to an empty string.
 * |default "rxRate"
 * |widget StringEntry()
 * |preview disable
 * |tab Labels
 *
 * |mode graphWidget
 * |factory /widgets/spectrogram(remoteEnv)
 * |setter setTitle(title)
 * |setter setDisplayRate(displayRate)
 * |setter setSampleRate(sampleRate)
 * |setter setCenterFrequency(centerFreq)
 * |setter setNumFFTBins(numBins)
 * |setter setWindowType(window)
 * |setter setTimeSpan(timeSpan)
 * |setter setReferenceLevel(refLevel)
 * |setter setDynamicRange(dynRange)
 * |setter enableXAxis(enableXAxis)
 * |setter enableYAxis(enableYAxis)
 * |setter setFreqLabelId(freqLabelId)
 * |setter setRateLabelId(rateLabelId)
 **********************************************************************/
class Spectrogram : public Pothos::Topology
{
public:
    static Topology *make(const Pothos::ProxyEnvironment::Sptr &remoteEnv)
    {
        return new Spectrogram(remoteEnv);
    }

    Spectrogram(const Pothos::ProxyEnvironment::Sptr &remoteEnv)
    {
        _display.reset(new SpectrogramDisplay());

        auto registry = remoteEnv->findProxy("Pothos/BlockRegistry");
        _snooper = registry.callProxy("/blocks/stream_snooper");

        //register calls in this topology
        this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, setNumFFTBins));

        //connect to internal display block
        this->connect(this, "setTitle", _display, "setTitle");
        this->connect(this, "setDisplayRate", _display, "setDisplayRate");
        this->connect(this, "setSampleRate", _display, "setSampleRate");
        this->connect(this, "setCenterFrequency", _display, "setCenterFrequency");
        this->connect(this, "setNumFFTBins", _display, "setNumFFTBins");
        this->connect(this, "setWindowType", _display, "setWindowType");
        this->connect(this, "setTimeSpan", _display, "setTimeSpan");
        this->connect(this, "setReferenceLevel", _display, "setReferenceLevel");
        this->connect(this, "setDynamicRange", _display, "setDynamicRange");
        this->connect(this, "enableXAxis", _display, "enableXAxis");
        this->connect(this, "enableYAxis", _display, "enableYAxis");
        this->connect(_display, "frequencySelected", this, "frequencySelected");

        //connect to the internal snooper block
        this->connect(_display, "updateRateChanged", _snooper, "setTriggerRate");
        this->connect(this, "setNumFFTBins", _snooper, "setChunkSize");

        //connect stream ports
        this->connect(this, 0, _snooper, 0);
        this->connect(_snooper, 0, _display, 0);
    }

    Pothos::Object opaqueCallMethod(const std::string &name, const Pothos::Object *inputArgs, const size_t numArgs) const
    {
        //calls that go to the topology
        try
        {
            return Pothos::Topology::opaqueCallMethod(name, inputArgs, numArgs);
        }
        catch (const Pothos::BlockCallNotFound &){}

        //forward everything else to display
        return _display->opaqueCallMethod(name, inputArgs, numArgs);
    }

    void setNumFFTBins(const size_t num)
    {
        _display->setName(this->getName()+"Display");
        _snooper.callVoid("setName", this->getName()+"Snooper");

        _snooper.callVoid("setChunkSize", num);
        _display->setNumFFTBins(num);
    }

private:
    Pothos::Proxy _snooper;
    std::shared_ptr<SpectrogramDisplay> _display;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerSpectrogram(
    "/widgets/spectrogram", &Spectrogram::make);
