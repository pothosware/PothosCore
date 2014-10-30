// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <qwt_math.h> //_USE_MATH_DEFINES
#include "MyPlotUtils.hpp"
#include <valarray>
#include <QVector>
#include <QPointF>
#include <qwt_plot_curve.h>
#include <memory>

class QwtPlot;

class PeriodogramChannel
{
public:
    PeriodogramChannel(const size_t index, QwtPlot *plot):
        _alpha(0.1)
    {
        _channelCurve.reset(new QwtPlotCurve(QString("Ch%1").arg(index)));
        _maxHoldCurve.reset(new QwtPlotCurve(QString("Max%1").arg(index)));
        _minHoldCurve.reset(new QwtPlotCurve(QString("Min%1").arg(index)));
        _averageCurve.reset(new QwtPlotCurve(QString("Avg%1").arg(index)));

        _channelCurve->setPen(pastelize(getDefaultCurveColor(index)));
        _maxHoldCurve->setPen(pastelize(getDefaultCurveColor(index)));
        _minHoldCurve->setPen(pastelize(getDefaultCurveColor(index)));
        _averageCurve->setPen(pastelize(getDefaultCurveColor(index)));

        _channelCurve->attach(plot);
        _maxHoldCurve->attach(plot);
        _minHoldCurve->attach(plot);
        _averageCurve->attach(plot);

        /*
         * TODO default hidden
        _maxHoldCurve->setVisible(false);
        _minHoldCurve->setVisible(false);
        _averageCurve->setVisible(false);
        */
    }

    void update(const std::valarray<float> &powerBins, const double rate, const double freq)
    {
        initBufferSize(powerBins, _channelBuffer);
        initBufferSize(powerBins, _maxHoldBuffer);
        initBufferSize(powerBins, _minHoldBuffer);
        initBufferSize(powerBins, _averageBuffer);

        for (size_t i = 0; i < powerBins.size(); i++)
        {
            auto x = (rate*i)/(powerBins.size()-1) - rate/2 + freq;
            _channelBuffer[i] = QPointF(x, powerBins[i]);
            _maxHoldBuffer[i] = QPointF(x, std::max<float>(_maxHoldBuffer[i].y(), powerBins[i]));
            _minHoldBuffer[i] = QPointF(x, std::min<float>(_minHoldBuffer[i].y(), powerBins[i]));
            //FIXME not correct for log-scale
            _averageBuffer[i] = QPointF(x, (1.0-_alpha)*_averageBuffer[i].y() + (_alpha)*powerBins[i]);
        }

        _channelCurve->setSamples(_channelBuffer);
        _maxHoldCurve->setSamples(_maxHoldBuffer);
        _minHoldCurve->setSamples(_minHoldBuffer);
        _averageCurve->setSamples(_averageBuffer);
    }

private:

    void initBufferSize(const std::valarray<float> &powerBins, QVector<QPointF> &buff)
    {
        if (size_t(buff.size()) == powerBins.size()) return;
        buff.clear();
        buff.resize(powerBins.size());
        for (size_t i = 0; i < powerBins.size(); i++)
        {
            buff[i] = QPointF(0, powerBins[i]);
        }
    }

    float _alpha;
    QVector<QPointF> _channelBuffer;
    QVector<QPointF> _maxHoldBuffer;
    QVector<QPointF> _minHoldBuffer;
    QVector<QPointF> _averageBuffer;
    std::shared_ptr<QwtPlotCurve> _channelCurve;
    std::shared_ptr<QwtPlotCurve> _maxHoldCurve;
    std::shared_ptr<QwtPlotCurve> _minHoldCurve;
    std::shared_ptr<QwtPlotCurve> _averageCurve;
};
