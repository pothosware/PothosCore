// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "ConstellationDisplay.hpp"
#include "MyPlotUtils.hpp"
#include <qwt_plot_curve.h>
#include <qwt_plot.h>
#include <complex>

/***********************************************************************
 * work functions
 **********************************************************************/
void ConstellationDisplay::handleSamples(const Pothos::BufferChunk &buff)
{
    if (_queueDepth.fetch_sub(1) != 1) return;

    //create curve that it doesnt exist
    if (not _curve)
    {
        _curve.reset(new QwtPlotCurve());
        _curve->attach(_mainPlot);
        _curve->setPen(pastelize(getDefaultCurveColor(0)), 2.0);
        _curve->setStyle(QwtPlotCurve::Dots);
    }

    //convert to points and post to curve
    const auto samps = buff.as<const std::complex<float> *>();
    QVector<QPointF> points(buff.elements());
    for (int i = 0; i < points.size(); i++)
    {
        points[i] = QPointF(samps[i].real(), samps[i].imag());
    }
    _curve->setSamples(points);

    //replot
    _mainPlot->replot();
}

void ConstellationDisplay::work(void)
{
    auto inPort = this->input(0);

    if (not inPort->hasMessage()) return;
    const auto msg = inPort->popMessage();

    //packet-based messages have payloads to plot
    if (msg.type() == typeid(Pothos::Packet))
    {
        _queueDepth++;
        const auto &buff = msg.convert<Pothos::Packet>().payload;
        auto floatBuff = buff.convert(Pothos::DType(typeid(std::complex<float>)), buff.elements());
        QMetaObject::invokeMethod(this, "handleSamples", Qt::QueuedConnection, Q_ARG(Pothos::BufferChunk, floatBuff));
    }
}
