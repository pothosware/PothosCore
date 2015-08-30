// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <qwt_math.h> //_USE_MATH_DEFINES
#include "MyPlotUtils.hpp"
#include <valarray>
#include <QVector>
#include <QPointF>
#include <memory>

class QwtPlot;
class QwtPlotCurve;

class PeriodogramChannel : QObject
{
    Q_OBJECT
public:
    PeriodogramChannel(const size_t index, MyQwtPlot *plot);

    ~PeriodogramChannel(void);

    void update(const std::valarray<float> &powerBins, const double rate, const double freq, const double factor);

private slots:

    void handleLegendChecked(const QVariant &itemInfo, bool on, int);

private:

    void initBufferSize(const std::valarray<float> &powerBins, QVector<QPointF> &buff);

    QwtPlot *_plot;
    QVector<QPointF> _channelBuffer;
    QVector<QPointF> _maxHoldBuffer;
    QVector<QPointF> _minHoldBuffer;
    std::shared_ptr<QwtPlotCurve> _channelCurve;
    std::shared_ptr<QwtPlotCurve> _maxHoldCurve;
    std::shared_ptr<QwtPlotCurve> _minHoldCurve;
};
