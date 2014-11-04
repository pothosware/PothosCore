// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Periodogram.hpp"
#include "PeriodogramChannel.hpp"
#include "MyPlotUtils.hpp"
#include <qwt_plot_curve.h>
#include <qwt_plot.h>
#include <complex>

/***********************************************************************
 * initialization functions
 **********************************************************************/
void Periodogram::activate(void)
{
    return;
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
    auto &curve = _curves[index];
    if (not curve) curve.reset(new PeriodogramChannel(index, _mainPlot));
    curve->update(powerBins, _sampleRateWoAxisUnits, _centerFreqWoAxisUnits, _averageFactor);
}

void Periodogram::work(void)
{
    bool doReplot = false;

    for (auto inPort : this->inputs())
    {
        //always consume all available input
        if (inPort->elements() == 0) continue;
        inPort->consume(inPort->elements());

        //use special stream labels to modify parameters
        for (const auto &label : inPort->labels())
        {
            if (label.id == _freqLabelId and label.data.canConvert(typeid(double)))
            {
                this->setCenterFrequency(label.data.convert<double>());
            }
            if (label.id == _rateLabelId and label.data.canConvert(typeid(double)))
            {
                this->setSampleRate(label.data.convert<double>());
            }
        }

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
