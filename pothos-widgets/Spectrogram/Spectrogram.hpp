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
#include <functional>
#include "MyFFTUtils.hpp"

class QTimer;
class MyQwtPlot;
class QwtColorMap;
class QwtPlotZoomer;
class QwtPlotSpectrogram;
class MySpectrogramRasterData;

/***********************************************************************
 * |PothosDoc Spectrogram
 *
 * The spectrogram plot displays a live plot of the spectrum vs time.
 *
 * |category /Widgets
 * |keywords frequency plot fft dft spectrum spectral
 *
 * |param dtype[Data Type] The data type of the input elements.
 * |option [Complex128] "complex128"
 * |option [Float64] "float64"
 * |option [Complex64] "complex64"
 * |option [Float32] "float32"
 * |option [Complex Int64] "complex_int64"
 * |option [Int64] "int64"
 * |option [Complex Int32] "complex_int32"
 * |option [Int32] "int32"
 * |option [Complex Int16] "complex_int16"
 * |option [Int16] "int16"
 * |option [Complex Int8] "complex_int8"
 * |option [Int8] "int8"
 * |default "float32"
 * |preview disable
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
 * |mode graphWidget
 * |factory /widgets/spectrogram(dtype)
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
 **********************************************************************/
class Spectrogram : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(const Pothos::DType &dtype)
    {
        return new Spectrogram(dtype);
    }

    Spectrogram(const Pothos::DType &dtype);

    ~Spectrogram(void);

    QWidget *widget(void)
    {
        return this;
    }

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
    void setTimeSpan(const double timeSpan);
    void setReferenceLevel(const double refLevel);
    void setDynamicRange(const double dynRange);

    QString title(void) const;

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

    double timeSpan(void) const
    {
        return _timeSpan;
    }

    double referenceLevel(void) const
    {
        return _refLevel;
    }

    double dynamicRange(void) const
    {
        return _dynRange;
    }

    void enableXAxis(const bool enb);
    void enableYAxis(const bool enb);

    void activate(void);
    void deactivate(void);
    void work(void);
    void updateCurve(Pothos::InputPort *inPort);

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
    void handlePickerSelected(const QPointF &);
    void appendBins(const std::valarray<float> &bins);
    void handleUpdateAxis(void);

private:
    QTimer *_replotTimer;
    MyQwtPlot *_mainPlot;
    QwtPlotZoomer *_zoomer;
    std::shared_ptr<QwtPlotSpectrogram> _plotSpect;
    MySpectrogramRasterData *_plotRaster;
    Pothos::Proxy _window;
    double _displayRate;
    double _sampleRate;
    double _sampleRateWoAxisUnits;
    double _centerFreq;
    double _centerFreqWoAxisUnits;
    size_t _numBins;
    double _timeSpan;
    double _refLevel;
    double _dynRange;
    std::chrono::high_resolution_clock::time_point _timeLastUpdate;

    std::function<void(Pothos::InputPort *, CArray &)> _inputConverter;
    QwtColorMap *makeColorMap(void) const;
};
