// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <qwt_math.h> //_USE_MATH_DEFINES
#include <Pothos/Framework.hpp>
#include <QWidget>
#include <memory>
#include <chrono>
#include <map>
#include <vector>
#include <functional>
#include "MyFFTUtils.hpp"

class MyQwtPlot;
class QwtPlotGrid;
class QwtPlotCurve;
class QwtPlotZoomer;

/***********************************************************************
 * |PothosDoc Periodogram
 *
 * The periodogram plot displays a live two dimensional plot of power vs frequency.
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
 * |param numBins[Num FFT Bins] The number of bins per fourier transform.
 * |default 1024
 *
 * |param window[Window Type] The spectral analysis window function type.
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
 * |mode graphWidget
 * |factory /widgets/periodogram(dtype)
 * |initializer setNumInputs(numInputs)
 * |setter setTitle(title)
 * |setter setDisplayRate(displayRate)
 * |setter setSampleRate(sampleRate)
 * |setter setNumFFTBins(numBins)
 * |setter setWindowType(window)
 * |setter setReferenceLevel(refLevel)
 * |setter setDynamicRange(dynRange)
 * |setter setAutoScale(autoScale)
 * |setter enableXAxis(enableXAxis)
 * |setter enableYAxis(enableYAxis)
 * |setter setYAxisTitle(yAxisTitle)
 **********************************************************************/
class Periodogram : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(const Pothos::DType &dtype)
    {
        return new Periodogram(dtype);
    }

    Periodogram(const Pothos::DType &dtype);

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

    void activate(void);
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
    WindowFunction _window;
    double _displayRate;
    double _sampleRate;
    double _sampleRateWoAxisUnits;
    size_t _numBins;
    double _refLevel;
    double _dynRange;
    bool _autoScale;
    std::chrono::high_resolution_clock::time_point _timeLastUpdate;

    //set of curves per index
    void setupPlotterCurves(void);
    std::map<size_t, std::shared_ptr<QwtPlotCurve>> _curves;
    std::map<size_t, std::function<void(Pothos::InputPort *, CArray &)>> _inputConverters;
};
