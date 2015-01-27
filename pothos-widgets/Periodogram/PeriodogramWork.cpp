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
    if (_queueDepth.at(index).fetch_sub(1) != 1) return;

    auto &curve = _curves[index];
    if (not curve) curve.reset(new PeriodogramChannel(index, _mainPlot));
    curve->update(powerBins, _sampleRateWoAxisUnits, _centerFreqWoAxisUnits, _averageFactor);
    _mainPlot->replot();
}

void PeriodogramDisplay::work(void)
{
    for (auto inPort : this->inputs())
    {
        if (not inPort->hasMessage()) continue;
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
            if (floatBuff.elements() != this->numFFTBins()) continue;

            //power bins to points on the curve
            CArray fftBins(floatBuff.as<const std::complex<float> *>(), this->numFFTBins());
            const auto powerBins = fftPowerSpectrum(fftBins, _window.call<std::vector<double>>("window"), _window.call<double>("power"));
            _queueDepth[inPort->index()]++;
            QMetaObject::invokeMethod(this, "handlePowerBins", Qt::QueuedConnection, Q_ARG(int, inPort->index()), Q_ARG(std::valarray<float>, powerBins));
        }
    }
}
