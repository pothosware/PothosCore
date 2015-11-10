// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PeriodogramDisplay.hpp"
#include "PeriodogramChannel.hpp"
#include "MyPlotUtils.hpp"
#include <qwt_plot_curve.h>
#include <qwt_plot.h>
#include <complex>

/***********************************************************************
 * work functions
 **********************************************************************/
void PeriodogramDisplay::handlePowerBins(const int index, const std::valarray<float> &powerBins)
{
    if (_queueDepth.at(index)->fetch_sub(1) != 1) return;

    auto &curve = _curves[index];
    if (not curve) curve.reset(new PeriodogramChannel(index, _mainPlot));
    curve->update(powerBins, _sampleRateWoAxisUnits, _centerFreqWoAxisUnits, _averageFactor);
    _mainPlot->replot();
}

void PeriodogramDisplay::work(void)
{
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
        const auto &packet = msg.convert<Pothos::Packet>();
        const auto indexIt = packet.metadata.find("index");
        const auto index = (indexIt == packet.metadata.end())?0:indexIt->second.convert<int>();

        const auto &buff = packet.payload;
        auto floatBuff = buff.convert(Pothos::DType(typeid(std::complex<float>)), buff.elements());

        //safe guard against FFT size changes, old buffers could still be in-flight
        if (floatBuff.elements() != this->numFFTBins()) return;

        //power bins to points on the curve
        CArray fftBins(floatBuff.as<const std::complex<float> *>(), this->numFFTBins());
        const auto powerBins = _fftPowerSpectrum.transform(fftBins);
        if (not _queueDepth[index]) _queueDepth[index].reset(new std::atomic<size_t>(0));
        _queueDepth[index]->fetch_add(1);
        QMetaObject::invokeMethod(this, "handlePowerBins", Qt::QueuedConnection, Q_ARG(int, index), Q_ARG(std::valarray<float>, powerBins));
    }
}
