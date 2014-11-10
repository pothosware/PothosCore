// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PeriodogramChannel.hpp"
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <cmath>

template <typename T>
T movingAvgPowerBinFilter(const T alpha, const T prev, const T curr)
{
    return 10*std::log((1-alpha)*std::exp(prev/10) + alpha*std::exp(curr/10));
}

PeriodogramChannel::PeriodogramChannel(const size_t index, MyQwtPlot *plot):
    _plot(plot)
{
    _channelCurve.reset(new QwtPlotCurve(QString("Ch%1").arg(index)));
    _maxHoldCurve.reset(new QwtPlotCurve(QString("Max%1").arg(index)));
    _minHoldCurve.reset(new QwtPlotCurve(QString("Min%1").arg(index)));

    _channelCurve->setPen(pastelize(getDefaultCurveColor(index)));

    auto minColor = pastelize(getDefaultCurveColor(2*index+8+0));
    auto maxColor = pastelize(getDefaultCurveColor(2*index+8+1));
    minColor.setAlphaF(0.5);
    maxColor.setAlphaF(0.5);

    _maxHoldCurve->setPen(minColor);
    _minHoldCurve->setPen(maxColor);

    _maxHoldCurve->setVisible(false);
    _minHoldCurve->setVisible(false);

    _channelCurve->attach(plot);
    _maxHoldCurve->attach(plot);
    _minHoldCurve->attach(plot);

    plot->updateChecked(_channelCurve.get());
    plot->updateChecked(_maxHoldCurve.get());
    plot->updateChecked(_minHoldCurve.get());

    connect(plot->legend(), SIGNAL(checked(const QVariant &, bool, int)), this, SLOT(handleLegendChecked(const QVariant &, bool, int)));
}

PeriodogramChannel::~PeriodogramChannel(void)
{
    return;
}

void PeriodogramChannel::update(const std::valarray<float> &powerBins, const double rate, const double freq, const double factor)
{
    //scale (0.0 to 1.0) to log10(1.0 to 10.0) = 0.0 to 1.0
    //alpha has a reversed log-scale effect on the averaging
    const float alpha = 1 - float(std::log10(9*factor + 1));

    initBufferSize(powerBins, _channelBuffer);
    initBufferSize(powerBins, _maxHoldBuffer);
    initBufferSize(powerBins, _minHoldBuffer);

    for (size_t i = 0; i < powerBins.size(); i++)
    {
        auto x = (rate*i)/(powerBins.size()-1) - rate/2 + freq;
        _channelBuffer[i] = QPointF(x, movingAvgPowerBinFilter<float>(alpha, _channelBuffer[i].y(), powerBins[i]));
        _maxHoldBuffer[i] = QPointF(x, std::max<float>(_maxHoldBuffer[i].y(), powerBins[i]));
        _minHoldBuffer[i] = QPointF(x, std::min<float>(_minHoldBuffer[i].y(), powerBins[i]));
    }

    _channelCurve->setSamples(_channelBuffer);
    _maxHoldCurve->setSamples(_maxHoldBuffer);
    _minHoldCurve->setSamples(_minHoldBuffer);
}

void PeriodogramChannel::handleLegendChecked(const QVariant &itemInfo, bool on, int)
{
    auto item = _plot->infoToItem(itemInfo);
    if (item == _channelCurve.get()) _channelCurve->setVisible(on);
    if (item == _maxHoldCurve.get())
    {
        _maxHoldCurve->setVisible(on);
        if (on) _maxHoldBuffer.clear();
    }
    if (item == _minHoldCurve.get())
    {
        _minHoldCurve->setVisible(on);
        if (on) _minHoldBuffer.clear();
    }
}

void PeriodogramChannel::initBufferSize(const std::valarray<float> &powerBins, QVector<QPointF> &buff)
{
    if (size_t(buff.size()) == powerBins.size()) return;
    buff.clear();
    buff.resize(powerBins.size());
    for (size_t i = 0; i < powerBins.size(); i++)
    {
        buff[i] = QPointF(0, powerBins[i]);
    }
}
