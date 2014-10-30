// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <qwt_math.h> //_USE_MATH_DEFINES
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <QWidget>
#include <memory>
#include <chrono>
#include <map>
#include <vector>
#include "MyFFTUtils.hpp"

class MyQwtPlot;
class QwtPlotGrid;
class QwtPlotCurve;
class QwtPlotZoomer;
class PeriodogramChannel;

/***********************************************************************
 * |PothosDoc Periodogram
 *
 * The periodogram plot displays a live two dimensional plot of power vs frequency.
 *
 * |category /Widgets
 * |keywords frequency plot fft dft spectrum spectral
 *
 * |param numInputs[Num Inputs] The number of input ports.
 * |default 1
 * |widget SpinBox(minimum=1)
 * |preview disable
 *
 * |param title The title of the plot
 * |default "Power vs Frequency"
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
 * |param autoScale[Auto-Scale] Enable automatic scaling for the vertical axis.
 * |default false
 * |option [Auto scale] true
 * |option [Use limits] false
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
 * |param yAxisTitle[Y-Axis Title] The title of the verical axis.
 * |default "dB"
 * |widget StringEntry()
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
 * |factory /widgets/periodogram()
 * |initializer setNumInputs(numInputs)
 * |setter setTitle(title)
 * |setter setDisplayRate(displayRate)
 * |setter setSampleRate(sampleRate)
 * |setter setCenterFrequency(centerFreq)
 * |setter setNumFFTBins(numBins)
 * |setter setWindowType(window)
 * |setter setReferenceLevel(refLevel)
 * |setter setDynamicRange(dynRange)
 * |setter setAutoScale(autoScale)
 * |setter enableXAxis(enableXAxis)
 * |setter enableYAxis(enableYAxis)
 * |setter setYAxisTitle(yAxisTitle)
 * |setter setFreqLabelId(freqLabelId)
 * |setter setRateLabelId(rateLabelId)
 **********************************************************************/
class Periodogram : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(void)
    {
        return new Periodogram();
    }

    Periodogram(void);

    ~Periodogram(void);

    QWidget *widget(void)
    {
        return this;
    }

    void setNumInputs(const size_t numInputs);

    //! set the plotter's title
    void setTitle(const QString &title);

    /*!
     * update rate for the plotter
     * how often to update the display
     */
    void setDisplayRate(const double displayRate);

    /*!
     * sample rate for the plotter
     * controls the frequency scaling display
     */
    void setSampleRate(const double sampleRate);

    /*!
     * center frequency of the plot
     */
    void setCenterFrequency(const double freq);

    void setNumFFTBins(const size_t numBins);
    void setWindowType(const std::string &windowType);
    void setReferenceLevel(const double refLevel);
    void setDynamicRange(const double dynRange);
    void setAutoScale(const bool autoScale);

    QString title(void) const;

    size_t numInputs(void) const
    {
        return this->inputs().size();
    }

    double displayRate(void) const
    {
        return _displayRate;
    }

    double sampleRate(void) const
    {
        return _sampleRate;
    }

    double centerFrequency(void) const
    {
        return _centerFreq;
    }

    size_t numFFTBins(void) const
    {
        return _numBins;
    }

    double referenceLevel(void) const
    {
        return _refLevel;
    }

    double dynamicRange(void) const
    {
        return _dynRange;
    }

    bool autoScale(void) const
    {
        return _autoScale;
    }

    void enableXAxis(const bool enb);
    void enableYAxis(const bool enb);
    void setYAxisTitle(const QString &title);

    void setFreqLabelId(const std::string &id)
    {
        _freqLabelId = id;
    }

    void setRateLabelId(const std::string &id)
    {
        _rateLabelId = id;
    }

    void activate(void);
    void work(void);
    bool updateCurve(Pothos::InputPort *inPort);

    //allow for standard resize controls with the default size policy
    QSize minimumSizeHint(void) const
    {
        return QSize(300, 150);
    }
    QSize sizeHint(void) const
    {
        return this->minimumSizeHint();
    }

private slots:
    void installLegend(void);
    void handleLegendChecked(const QVariant &, bool, int);
    void handlePickerSelected(const QPointF &);
    void handlePowerBins(const int index, const std::valarray<float> &bins);
    void handleUpdateAxis(void);
    void handleZoomed(const QRectF &rect);

private:
    MyQwtPlot *_mainPlot;
    QwtPlotGrid *_plotGrid;
    QwtPlotZoomer *_zoomer;
    Pothos::Proxy _window;
    double _displayRate;
    double _sampleRate;
    double _sampleRateWoAxisUnits;
    double _centerFreq;
    double _centerFreqWoAxisUnits;
    size_t _numBins;
    double _refLevel;
    double _dynRange;
    bool _autoScale;
    std::string _freqLabelId;
    std::string _rateLabelId;

    //per-port data structs
    std::map<size_t, std::chrono::high_resolution_clock::time_point> _lastUpdateTimes;
    std::map<size_t, Pothos::BufferChunk> _rasterBuffs;
    std::map<size_t, std::shared_ptr<PeriodogramChannel>> _curves;
};
