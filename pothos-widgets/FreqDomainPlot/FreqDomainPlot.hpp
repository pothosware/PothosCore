// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Framework.hpp>
#include <QWidget>
#include <memory>
#include <chrono>
#include <map>
#include <vector>
#include <functional>
#include "MyFFTUtils.hpp"

class QwtPlot;
class QwtPlotGrid;
class QwtPlotCurve;

/***********************************************************************
 * |PothosDoc Frequency Domain Plot
 *
 * The frequency domain plot displays a live two dimensional plot of power vs frequency.
 *
 * |category /Widgets
 * |keywords frequency plot
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
 * |mode graphWidget
 * |factory /widgets/freq_domain_plot(dtype)
 * |setter setNumInputs(numInputs)
 * |setter setTitle(title)
 * |setter setDisplayRate(displayRate)
 * |setter setSampleRate(sampleRate)
 * |setter setNumFFTBins(numBins)
 **********************************************************************/
class FreqDomainPlot : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(const Pothos::DType &dtype)
    {
        return new FreqDomainPlot(dtype);
    }

    FreqDomainPlot(const Pothos::DType &dtype);

    ~FreqDomainPlot(void);

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

private:
    QwtPlot *_mainPlot;
    QwtPlotGrid *_plotGrid;
    double _displayRate;
    double _sampleRate;
    double _sampleRateWoAxisUnits;
    size_t _numBins;
    std::chrono::high_resolution_clock::time_point _timeLastUpdate;

    //set of curves per index
    void setupPlotterCurves(void);
    std::map<size_t, std::shared_ptr<QwtPlotCurve>> _curves;
    std::map<size_t, std::function<void(Pothos::InputPort *, CArray &)>> _inputConverters;
};
