// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Periodogram.hpp"
#include "MyPlotUtils.hpp"
#include <qwt_plot_curve.h>
#include <qwt_plot.h>
#include <complex>

/***********************************************************************
 * initialization functions
 **********************************************************************/
void Periodogram::activate(void)
{
    //install legend for multiple channels
    if (this->inputs().size() > 1) QMetaObject::invokeMethod(this, "installLegend", Qt::QueuedConnection);
}

/***********************************************************************
 * work functions
 **********************************************************************/
bool Periodogram::updateCurve(Pothos::InputPort *inPort)
{
    //convert and append the new buffer
    auto buff = inPort->buffer();
    auto floatBuff = buff.convert(Pothos::DType(typeid(std::complex<float>)), buff.elements());
    auto &rasterBuff = _rasterBuffs[inPort->index()];
    rasterBuff.append(floatBuff);

    //enough in the raster buffer?
    if (rasterBuff.elements() < this->numFFTBins()) return false;

    //power bins to points on the curve
    CArray fftBins(rasterBuff.as<const std::complex<float> *>(), this->numFFTBins());
    const auto powerBins = fftPowerSpectrum(fftBins, _window.call<std::vector<double>>("window"), _window.call<double>("power"));
    QMetaObject::invokeMethod(this, "handlePowerBins", Qt::QueuedConnection, Q_ARG(int, inPort->index()), Q_ARG(std::valarray<float>, powerBins));

    //clear old raster buffer
    rasterBuff = Pothos::BufferChunk();
    return true;
}

void Periodogram::handlePowerBins(const int index, const std::valarray<float> &powerBins)
{
    QVector<QPointF> points(powerBins.size());
    for (size_t i = 0; i < powerBins.size(); i++)
    {
        auto freq = (_sampleRateWoAxisUnits*i)/(powerBins.size()-1) - _sampleRateWoAxisUnits/2;
        points[i] = QPointF(freq+_centerFreqWoAxisUnits, powerBins[i]);
    }

    auto &curve = _curves[index];
    if (not curve)
    {
        curve.reset(new QwtPlotCurve(QString("Ch%1").arg(index)));
        curve->attach(_mainPlot);
        curve->setPen(pastelize(getDefaultCurveColor(index)));
    }
    _curves.at(index)->setSamples(points);
}

void Periodogram::work(void)
{
    bool doReplot = false;

    for (auto inPort : this->inputs())
    {
        //always consume all available input
        if (inPort->elements() == 0) continue;
        inPort->consume(inPort->elements());

        //should we update the plotter with these values?
        auto &lastUpdateTime = _lastUpdateTimes[inPort->index()];
        const auto timeBetweenUpdates = std::chrono::nanoseconds((long long)(1e9/_displayRate));
        bool doUpdate = (std::chrono::high_resolution_clock::now() - lastUpdateTime) > timeBetweenUpdates;

        //perform the raster update
        if (doUpdate and this->updateCurve(inPort))
        {
            lastUpdateTime = std::chrono::high_resolution_clock::now();
            doReplot = true;
        }
    }

    if (doReplot) QMetaObject::invokeMethod(_mainPlot, "replot", Qt::QueuedConnection);
}
