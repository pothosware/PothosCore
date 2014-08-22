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

class QwtPlot;
class QwtPlotGrid;
class QwtPlotCurve;

/***********************************************************************
 * |PothosDoc Time Domain Plot
 *
 * The time domain plot displays a live two dimensional plot of input elements vs time.
 *
 * |category /Widgets
 * |keywords time plot
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
 * |default "Amplitude vs Time"
 *
 * |param displayRate[Display Rate] How often the plotter updates.
 * |default 10.0
 * |units updates/sec
 *
 * |param sampleRate[Sample Rate] The rate of the input elements.
 * |default 1e6
 * |units samples/sec
 *
 * |param numPoints[Num Points] The number of points per plot capture.
 * |default 1024
 *
 * |mode graphWidget
 * |factory /widgets/time_domain_plot(dtype)
 * |setter setNumInputs(numInputs)
 * |setter setTitle(title)
 * |setter setDisplayRate(displayRate)
 * |setter setSampleRate(sampleRate)
 * |setter setNumPoints(numPoints)
 **********************************************************************/
class TimeDomainPlot : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(const Pothos::DType &dtype)
    {
        return new TimeDomainPlot(dtype);
    }

    TimeDomainPlot(const Pothos::DType &dtype);

    ~TimeDomainPlot(void);

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
     * controls the time scaling display
     */
    void setSampleRate(const double sampleRate);

    void setNumPoints(const size_t numPoints);

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

    size_t numPoints(void) const
    {
        return _numPoints;
    }

    void activate(void);
    void work(void);
    void updateCurve(Pothos::InputPort *inPort);

private slots:
    void setupPlotterCurves(void);

private:
    QwtPlot *_mainPlot;
    QwtPlotGrid *_plotGrid;
    double _displayRate;
    double _sampleRate;
    double _timeSpan;
    size_t _numPoints;
    std::chrono::high_resolution_clock::time_point _timeLastUpdate;

    //set of curves per index
    std::map<size_t, std::vector<std::shared_ptr<QwtPlotCurve>>> _curves;
    std::map<size_t, std::function<void(Pothos::InputPort *, const size_t, const double)>> _curveUpdaters;

    void updateXAxis(void);
};
