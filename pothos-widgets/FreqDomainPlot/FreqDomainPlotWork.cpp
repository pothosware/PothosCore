// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MyPlotterUtils.hpp"
#include "MyFFTUtils.hpp"
#include "FreqDomainPlot.hpp"
#include <qwt_plot_curve.h>
#include <qwt_plot.h>
#include <complex>

void FreqDomainPlot::activate(void)
{
    //reload num bins so we know inPort->setReserve is set
    this->setNumFFTBins(_numBins);

    //clear old curves
    _curves.clear();
    _curveUpdaters.clear();
}

template <typename InType>
Complex toComplex(const InType &in)
{
    return Complex(in);
}

template <typename InType>
Complex toComplex(const std::complex<InType> &in)
{
    return Complex(in.real(), in.imag());
}

template <typename T>
void plotCurvesFromElements(Pothos::InputPort *inPort, const size_t numElems, const double elemRate,
    std::shared_ptr<QwtPlotCurve> curve)
{
    //create an array of complex doubles to transform with FFT
    auto buff = inPort->buffer().as<const T *>();
    CArray fftBins(numElems);
    for (size_t i = 0; i < numElems; i++) fftBins[i] = toComplex(buff[i]);
    fft(fftBins);

    //TODO windowing

    //TODO power calculation

    //TODO bin reorder

    QVector<QPointF> points;
    for (size_t i = 0; i < fftBins.size(); i++)
    {
        points.push_back(QPointF(elemRate/i, std::abs(fftBins[i])));
    }

    curve->setSamples(points);
}

void FreqDomainPlot::setupPlotterCurves(void)
{
    for (auto inPort : this->inputs())
    {
        #define doForThisType__(type) \
        else if (inPort->dtype() == Pothos::DType(typeid(type))) \
        { \
            _curves[inPort->index()].reset(new QwtPlotCurve(QString("Ch%1").arg(inPort->index()))); \
            _curveUpdaters[inPort->index()] = std::bind( \
                &plotCurvesFromElements<type>, \
                std::placeholders::_1, \
                std::placeholders::_2, \
                std::placeholders::_3, \
                _curves[inPort->index()]); \
        }
        #define doForThisType(type) \
            doForThisType__(type) \
            doForThisType__(std::complex<type>)
        if (false){}
        doForThisType(double)
        doForThisType(float)
        doForThisType(signed long long)
        doForThisType(unsigned long long)
        doForThisType(signed long)
        doForThisType(unsigned long)
        doForThisType(signed int)
        doForThisType(unsigned int)
        doForThisType(signed short)
        doForThisType(unsigned short)
        doForThisType(signed char)
        doForThisType(unsigned char)
        doForThisType(char)
    }

    //continued setup for the curves
    size_t whichCurve = 0;
    for (const auto &pair : _curves)
    {
        auto &curve = pair.second;
        {
            curve->attach(_mainPlot);
            curve->setPen(pastelize(getDefaultCurveColor(whichCurve)));
            whichCurve++;
        }
    }
}

void FreqDomainPlot::work(void)
{
    //initialize the curves with a blocking call to setup
    if (_curves.empty()) QMetaObject::invokeMethod(this, "setupPlotterCurves", Qt::BlockingQueuedConnection);

    //should we update the plotter with these values?
    const auto timeBetweenUpdates = std::chrono::nanoseconds((long long)(1e9/_displayRate));
    bool doUpdate = (std::chrono::high_resolution_clock::now() - _timeLastUpdate) > timeBetweenUpdates;

    //reload the curves with new data -- also consume all input
    const size_t nsamps = this->workInfo().minElements;
    for (auto inPort : this->inputs())
    {
        if (doUpdate) _curveUpdaters.at(inPort->index())(inPort, std::min(nsamps, _numBins), _sampleRate);
        inPort->consume(nsamps);
    }

    //perform the plotter update
    if (doUpdate)
    {
        QMetaObject::invokeMethod(_mainPlot, "replot", Qt::QueuedConnection);
        _timeLastUpdate = std::chrono::high_resolution_clock::now();
    }
}
