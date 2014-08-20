// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "TimeDomainPlot.hpp"
#include <qwt_plot_curve.h>
#include <qwt_plot.h>
#include <complex>

static const size_t pointsPerPlot = 1024; //TODO variable later

template <typename T>
void plotCurvesFromElements(Pothos::InputPort *inPort, const size_t numElems, const double elemRate,
    std::shared_ptr<QwtPlotCurve> curve)
{
    auto buff = inPort->buffer().as<const T *>();
    QVector<QPointF> points;
    for (size_t i = 0; i < numElems; i++)
    {
        points.push_back(QPointF(i/elemRate, buff[i]));
    }

    curve->setSamples(points);
}

template <typename T>
void plotCurvesFromComplexElements(Pothos::InputPort *inPort, const size_t numElems, const double elemRate,
    std::shared_ptr<QwtPlotCurve> curveRe, std::shared_ptr<QwtPlotCurve> curveIm)
{
    auto buff = inPort->buffer().as<const std::complex<T> *>();
    QVector<QPointF> pointsRe, pointsIm;
    for (size_t i = 0; i < numElems; i++)
    {
        pointsRe.push_back(QPointF(i/elemRate, buff[i].real()));
        pointsIm.push_back(QPointF(i/elemRate, buff[i].imag()));
    }
    curveRe->setSamples(pointsRe);
    curveIm->setSamples(pointsIm);
}

void TimeDomainPlot::activate(void)
{
    for (auto inPort : this->inputs()) inPort->setReserve(pointsPerPlot);

    //clear old curves
    _curves.clear();
    _curveUpdaters.clear();
}

void TimeDomainPlot::setupPlotterCurves(void)
{
    for (auto inPort : this->inputs())
    {
        #define doForThisType(type) \
        else if (inPort->dtype() == Pothos::DType(typeid(std::complex<type>))) \
        { \
            _curves[inPort->index()].emplace_back(new QwtPlotCurve()); \
            _curves[inPort->index()].emplace_back(new QwtPlotCurve()); \
            _curveUpdaters[inPort->index()] = std::bind( \
                &plotCurvesFromComplexElements<type>, \
                std::placeholders::_1, \
                std::placeholders::_2, \
                std::placeholders::_3, \
                _curves[inPort->index()][0], \
                _curves[inPort->index()][1]); \
        } \
        else if (inPort->dtype() == Pothos::DType(typeid(type))) \
        { \
            _curves[inPort->index()].emplace_back(new QwtPlotCurve()); \
            _curveUpdaters[inPort->index()] = std::bind( \
                &plotCurvesFromElements<type>, \
                std::placeholders::_1, \
                std::placeholders::_2, \
                std::placeholders::_3, \
                _curves[inPort->index()][0]); \
        }
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

    for (const auto &pair : _curves)
    {
        for (const auto &curve : pair.second)
        {
            curve->attach(_mainPlot);
        }
    }
}

void TimeDomainPlot::work(void)
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
        if (doUpdate) _curveUpdaters.at(inPort->index())(inPort, std::min(nsamps, pointsPerPlot), _sampleRate);
        inPort->consume(nsamps);
    }

    //perform the plotter update
    if (doUpdate)
    {
        QMetaObject::invokeMethod(_mainPlot, "replot", Qt::QueuedConnection);
        _timeLastUpdate = std::chrono::high_resolution_clock::now();
    }
}
