// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Spectrogram.hpp"
#include "MyPlotUtils.hpp"
#include <qwt_plot.h>
#include <QTimer>
#include <complex>

/***********************************************************************
 * initialization functions
 **********************************************************************/
void Spectrogram::activate(void)
{
    QMetaObject::invokeMethod(_replotTimer, "start", Qt::QueuedConnection);
}

void Spectrogram::deactivate(void)
{
    QMetaObject::invokeMethod(_replotTimer, "stop", Qt::QueuedConnection);
}

/***********************************************************************
 * work functions
 **********************************************************************/
bool Spectrogram::updateCurve(Pothos::InputPort *inPort)
{
    //convert and append the new buffer
    auto buff = inPort->buffer();
    auto floatBuff = buff.convert(Pothos::DType(typeid(std::complex<float>)), buff.elements());
    _rasterBuff.append(floatBuff);

    //enough in the raster buffer?
    if (_rasterBuff.elements() < this->numFFTBins()) return false;

    //power bins to points on the curve
    CArray fftBins(_rasterBuff.as<const std::complex<float> *>(), this->numFFTBins());
    const auto powerBins = fftPowerSpectrum(fftBins, _window.call<std::vector<double>>("window"), _window.call<double>("power"));
    this->appendBins(powerBins);

    //clear old raster buffer
    _rasterBuff = Pothos::BufferChunk();
    return true;
}

void Spectrogram::work(void)
{
    auto updateRate = this->height()/_timeSpan;

    //should we update the plotter with these values?
    const auto timeBetweenUpdates = std::chrono::nanoseconds((long long)(1e9/updateRate));
    bool doUpdate = (std::chrono::high_resolution_clock::now() - _timeLastUpdate) > timeBetweenUpdates;

    //create a new entry in the raster data + consume
    auto inPort = this->input(0);
    if (inPort->elements() == 0) return;
    inPort->consume(inPort->elements());

    //perform the plotter update
    if (doUpdate and this->updateCurve(inPort))
    {
        _timeLastUpdate = std::chrono::high_resolution_clock::now();
    }
}
