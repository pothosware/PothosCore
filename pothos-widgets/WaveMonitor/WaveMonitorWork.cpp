// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "WaveMonitorDisplay.hpp"
#include "MyPlotStyler.hpp"
#include "MyPlotUtils.hpp"
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot.h>
#include <complex>
#include <iostream>

/***********************************************************************
 * work functions
 **********************************************************************/
void WaveMonitorDisplay::handleSamples(const int index, const int whichCurve, const Pothos::BufferChunk &buff, const std::vector<Pothos::Label> &labels)
{
    if (_queueDepth.at(index).at(whichCurve)->fetch_sub(1) != 1) return;

    const auto samps = buff.as<const float *>();
    QVector<QPointF> points(buff.elements());
    for (int i = 0; i < points.size(); i++)
    {
        points[i] = QPointF(i/_sampleRateWoAxisUnits, samps[i]);
    }

    //install legend for multiple channels
    if (_curves.empty() and this->inputs().size() > 1) this->installLegend();

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
            //legend not installed with single input? well its complex...
            if (this->inputs().size() == 1) this->installLegend();
            _mainPlot->updateChecked(_curves[index][0].get());
            _mainPlot->updateChecked(_curves[index][1].get());
        }
        _mainPlot->updateChecked(curve.get());
    }
    curve->setSamples(points);

    //create markers from labels
    auto &markers = _markers[index];
    if (whichCurve == 0) markers.clear(); //clear old markers
    for (const auto &label : labels)
    {
        auto marker = new QwtPlotMarker();
        marker->setLabel(MyMarkerLabel(QString::fromStdString(label.id)));
        marker->setLabelAlignment(Qt::AlignHCenter);
        auto index = label.index + (label.width-1)/2.0;
        marker->setXValue(index/_sampleRateWoAxisUnits);
        marker->setYValue(samps[label.index]);
        marker->attach(_mainPlot);
        markers.emplace_back(marker);
    }

    _mainPlot->replot();
}

void WaveMonitorDisplay::work(void)
{
    for (auto inPort : this->inputs())
    {
        if (not inPort->hasMessage()) continue;
        const auto msg = inPort->popMessage();

        //label-based messages have in-line commands
        if (msg.type() == typeid(Pothos::Label))
        {
            const auto &label = msg.convert<Pothos::Label>();
            if (label.id == _rateLabelId and label.data.canConvert(typeid(double)))
            {
                this->setSampleRate(label.data.convert<double>());
            }
        }

        //packet-based messages have payloads to display
        if (msg.type() == typeid(Pothos::Packet))
        {
            const auto &packet = msg.convert<Pothos::Packet>();
            const auto &buff = packet.payload;
            std::vector<Pothos::BufferChunk> floatBuffs;

            if (buff.dtype.isComplex())
            {
                floatBuffs.resize(2);
                auto outs = buff.convertComplex(Pothos::DType(typeid(float)), buff.elements());
                floatBuffs[0].append(outs.first);
                floatBuffs[1].append(outs.second);
            }
            else
            {
                floatBuffs.resize(1);
                auto out = buff.convert(Pothos::DType(typeid(float)), buff.elements());
                floatBuffs[0].append(out);
            }

            //ensure that we have allocated depth counters (used to avoid displaying old data)
            if (not _queueDepth[inPort->index()][0]) _queueDepth[inPort->index()][0].reset(new std::atomic<size_t>());
            if (not _queueDepth[inPort->index()][1]) _queueDepth[inPort->index()][1].reset(new std::atomic<size_t>());

            _queueDepth[inPort->index()][0]->fetch_add(1);
            QMetaObject::invokeMethod(this, "handleSamples", Qt::QueuedConnection,
                Q_ARG(int, inPort->index()), Q_ARG(int, 0),
                Q_ARG(Pothos::BufferChunk, floatBuffs[0]),
                Q_ARG(std::vector<Pothos::Label>, packet.labels));

            const bool hasIm = floatBuffs.size() > 1;
            if (hasIm) _queueDepth[inPort->index()][1]->fetch_add(1);
            if (hasIm) QMetaObject::invokeMethod(this, "handleSamples", Qt::QueuedConnection,
                Q_ARG(int, inPort->index()), Q_ARG(int, 1),
                Q_ARG(Pothos::BufferChunk, floatBuffs[1]),
                Q_ARG(std::vector<Pothos::Label>, std::vector<Pothos::Label>()));
        }
    }
}
