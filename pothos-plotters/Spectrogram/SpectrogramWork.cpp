// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SpectrogramDisplay.hpp"
#include "MyPlotUtils.hpp"
#include <qwt_plot.h>
#include <QTimer>
#include <complex>

/***********************************************************************
 * initialization functions
 **********************************************************************/
void SpectrogramDisplay::activate(void)
{
    QMetaObject::invokeMethod(_replotTimer, "start", Qt::QueuedConnection);
}

void SpectrogramDisplay::deactivate(void)
{
    QMetaObject::invokeMethod(_replotTimer, "stop", Qt::QueuedConnection);
}

/***********************************************************************
 * work function
 **********************************************************************/
void SpectrogramDisplay::work(void)
{
    auto updateRate = this->height()/_timeSpan;
    if (updateRate != _lastUpdateRate) this->callVoid("updateRateChanged", updateRate);
    _lastUpdateRate = updateRate;

    auto inPort = this->input(0);
    if (not inPort->hasMessage()) return;
    const auto msg = inPort->popMessage();

    //label-based messages have in-line commands
    if (msg.type() == typeid(Pothos::Label))
    {
        const auto &label = msg.convert<Pothos::Label>();
        if (label.id == _freqLabelId and label.data.canConvert(typeid(double)))
        {
            this->setCenterFrequency(label.data.convert<double>());
        }
        if (label.id == _rateLabelId and label.data.canConvert(typeid(double)))
        {
            this->setSampleRate(label.data.convert<double>());
        }
    }

    //packet-based messages have payloads to FFT
    if (msg.type() == typeid(Pothos::Packet))
    {
        const auto &buff = msg.convert<Pothos::Packet>().payload;
        auto floatBuff = buff.convert(Pothos::DType(typeid(std::complex<float>)), buff.elements());

        //safe guard against FFT size changes, old buffers could still be in-flight
        if (floatBuff.elements() != this->numFFTBins()) return;

        //power bins to points on the curve
        CArray fftBins(floatBuff.as<const std::complex<float> *>(), this->numFFTBins());
        const auto powerBins = fftPowerSpectrum(fftBins, _window.call<std::vector<double>>("window"), _window.call<double>("power"));
        this->appendBins(powerBins);
    }
}
