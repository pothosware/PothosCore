// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "WaveMonitor.hpp"
#include "MyPlotStyler.hpp"
#include "MyPlotUtils.hpp"
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot.h>
#include <complex>
#include <iostream>

/***********************************************************************
 * initialization functions
 **********************************************************************/
void WaveMonitor::activate(void)
{
    //install legend for multiple channels
    if (this->inputs().size() > 1) QMetaObject::invokeMethod(this, "installLegend", Qt::QueuedConnection);
}

/***********************************************************************
 * work functions
 **********************************************************************/
bool WaveMonitor::updateCurve(Pothos::InputPort *inPort)
{
    //convert and append the new buffer
    auto buff = inPort->buffer();
    auto &rasterBuffs = _rasterBuffs[inPort->index()];
    if (buff.dtype.isComplex())
    {
        rasterBuffs.resize(2);
        auto outs = buff.convertComplex(Pothos::DType(typeid(float)), buff.elements());
        rasterBuffs[0].append(outs.first);
        rasterBuffs[1].append(outs.second);
    }
    else
    {
        rasterBuffs.resize(1);
        auto out = buff.convert(Pothos::DType(typeid(float)), buff.elements());
        rasterBuffs[0].append(out);
    }

    //enough in the raster buffer?
    if (rasterBuffs[0].elements() < this->numPoints()) return false;

    auto labels = std::vector<Pothos::Label>(inPort->labels().begin(), inPort->labels().end());

    QMetaObject::invokeMethod(this, "handleSamples", Qt::QueuedConnection,
        Q_ARG(int, inPort->index()), Q_ARG(int, 0),
        Q_ARG(std::valarray<float>, std::valarray<float>(rasterBuffs[0].as<const float *>(), this->numPoints())),
        Q_ARG(std::vector<Pothos::Label>, labels));

    const bool hasIm = rasterBuffs.size() > 1;
    if (hasIm) QMetaObject::invokeMethod(this, "handleSamples", Qt::QueuedConnection,
        Q_ARG(int, inPort->index()), Q_ARG(int, 1),
        Q_ARG(std::valarray<float>, std::valarray<float>(rasterBuffs[1].as<const float *>(), this->numPoints())),
        Q_ARG(std::vector<Pothos::Label>, std::vector<Pothos::Label>()));

    //clear old raster buffer
    rasterBuffs.clear();
    return true;
}

void WaveMonitor::handleSamples(const int index, const int whichCurve, const std::valarray<float> &samps, const std::vector<Pothos::Label> &labels)
{
    QVector<QPointF> points(samps.size());
    for (size_t i = 0; i < samps.size(); i++)
    {
        points[i] = QPointF((i*_timeSpan)/(samps.size()-1), samps[i]);
    }

    //create curve if it doesnt exist
    auto &curve = _curves[index][whichCurve];
    if (not curve)
    {
        curve.reset(new QwtPlotCurve(QString("Ch%1").arg(index)));
        curve->attach(_mainPlot);
        curve->setPen(pastelize(getDefaultCurveColor(_nextColorIndex++)));
        //relabel curves
        if (_curves[index].size() == 2)
        {
            _curves[index][0]->setTitle(QString("Re%1").arg(index));
            _curves[index][1]->setTitle(QString("Im%1").arg(index));
            this->installLegend(); //legend not installed with single input? well its complex...
        }
    }
    curve->setSamples(points);

    //create markers from labels
    auto &markers = _markers[index];
    markers.clear(); //clear old markers
    for (const auto &label : labels)
    {
        auto marker = new QwtPlotMarker();
        if (label.index >= samps.size()) break;
        marker->setLabel(MyMarkerLabel(QString::fromStdString(label.id)));
        marker->setLabelAlignment(Qt::AlignHCenter);
        marker->setXValue((label.index*_timeSpan)/(samps.size()-1));
        marker->setYValue(samps[label.index]);
        marker->attach(_mainPlot);
        markers.emplace_back(marker);
    }
}

void WaveMonitor::work(void)
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
