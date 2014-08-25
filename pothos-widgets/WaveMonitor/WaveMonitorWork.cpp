// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MyPlotterUtils.hpp"
#include "WaveMonitor.hpp"
#include <qwt_plot_curve.h>
#include <qwt_plot.h>
#include <complex>

/***********************************************************************
 * conversion support
 **********************************************************************/
template <typename T>
void convertRealElements(Pothos::InputPort *inPort, std::valarray<float> &samps, std::valarray<float> &)
{
    auto buff = inPort->buffer().as<const T *>();
    for (size_t i = 0; i < samps.size(); i++)
    {
        samps[i] = float(buff[i]);
    }
}

template <typename T>
void convertComplexElements(Pothos::InputPort *inPort, std::valarray<float> &sampsRe, std::valarray<float> &sampsIm)
{
    auto buff = inPort->buffer().as<const std::complex<T> *>();
    for (size_t i = 0; i < sampsRe.size(); i++)
    {
        sampsRe[i] = float(buff[i].real());
        sampsIm[i] = float(buff[i].imag());
    }
}

/***********************************************************************
 * initialization functions
 **********************************************************************/
void WaveMonitor::activate(void)
{
    //reload num bins so we know inPort->setReserve is set
    this->setNumPoints(this->numPoints());
    this->setupPlotterCurves();
}

void WaveMonitor::setupPlotterCurves(void)
{
    //clear old curves
    _curves.clear();
    _inputConverters.clear();
    for (auto inPort : this->inputs())
    {
        #define doForThisType(type) \
        else if (inPort->dtype() == Pothos::DType(typeid(std::complex<type>))) \
        { \
            _curves[inPort->index()].emplace_back(new QwtPlotCurve(QString("Re%1").arg(inPort->index()))); \
            _curves[inPort->index()].emplace_back(new QwtPlotCurve(QString("Im%1").arg(inPort->index()))); \
            _inputConverters[inPort->index()] = std::bind( \
                &convertComplexElements<type>, \
                std::placeholders::_1, \
                std::placeholders::_2, \
                std::placeholders::_3); \
        } \
        else if (inPort->dtype() == Pothos::DType(typeid(type))) \
        { \
            _curves[inPort->index()].emplace_back(new QwtPlotCurve(QString("Ch%1").arg(inPort->index()))); \
            _inputConverters[inPort->index()] = std::bind( \
                &convertRealElements<type>, \
                std::placeholders::_1, \
                std::placeholders::_2, \
                std::placeholders::_3); \
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
        else throw Pothos::InvalidArgumentException("WaveMonitor::setupPlotterCurves("+inPort->dtype().toString()+")", "dtype not supported");
    }

    //continued setup for the curves
    size_t whichCurve = 0;
    for (const auto &pair : _curves)
    {
        for (const auto &curve : pair.second)
        {
            curve->attach(_mainPlot);
            curve->setPen(pastelize(getDefaultCurveColor(whichCurve)));
            whichCurve++;
        }
    }

    //install legend for multiple channels
    if (whichCurve > 1) QMetaObject::invokeMethod(this, "installLegend", Qt::QueuedConnection);
}

/***********************************************************************
 * work functions
 **********************************************************************/
void WaveMonitor::updateCurve(Pothos::InputPort *inPort)
{
    const bool hasIm = _curves.at(inPort->index()).size() > 1;

    std::valarray<float> sampsRe(std::min(inPort->elements(), this->numPoints()));
    std::valarray<float> sampsIm; if (hasIm) sampsIm.resize(sampsRe.size());

    _inputConverters.at(inPort->index())(inPort, std::ref(sampsRe), std::ref(sampsIm));

    QMetaObject::invokeMethod(this, "handleSamples", Qt::QueuedConnection,
        Q_ARG(int, inPort->index()), Q_ARG(int, 0), Q_ARG(std::valarray<float>, sampsRe));

    if (hasIm) QMetaObject::invokeMethod(this, "handleSamples", Qt::QueuedConnection,
        Q_ARG(int, inPort->index()), Q_ARG(int, 1), Q_ARG(std::valarray<float>, sampsIm));
}

void WaveMonitor::handleSamples(const int index, const int curve, const std::valarray<float> &samps)
{
    QVector<QPointF> points(samps.size());
    for (size_t i = 0; i < samps.size(); i++)
    {
        points[i] = QPointF((i*_timeSpan)/(samps.size()-1), samps[i]);
    }
    _curves.at(index).at(curve)->setSamples(points);
}

void WaveMonitor::work(void)
{
    //should we update the plotter with these values?
    const auto timeBetweenUpdates = std::chrono::nanoseconds((long long)(1e9/_displayRate));
    bool doUpdate = (std::chrono::high_resolution_clock::now() - _timeLastUpdate) > timeBetweenUpdates;

    //reload the curves with new data -- also consume all input
    const size_t nsamps = this->workInfo().minElements;
    for (auto inPort : this->inputs())
    {
        if (doUpdate) this->updateCurve(inPort);
        inPort->consume(nsamps);
    }

    //perform the plotter update
    if (doUpdate)
    {
        QMetaObject::invokeMethod(_mainPlot, "replot", Qt::QueuedConnection);
        _timeLastUpdate = std::chrono::high_resolution_clock::now();
    }
}
