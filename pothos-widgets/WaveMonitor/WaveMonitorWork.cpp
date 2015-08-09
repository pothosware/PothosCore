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
void WaveMonitorDisplay::handleSamples(const Pothos::Packet &packet)
{
    //extract index
    const auto indexIt = packet.metadata.find("index");
    const auto index = (indexIt == packet.metadata.end())?0:indexIt->second.convert<size_t>();
    if (_queueDepth.at(index)->fetch_sub(1) != 1) return;

    //extract offset
    const auto offsetIt = packet.metadata.find("position");
    const auto offset = (offsetIt == packet.metadata.end())?0:offsetIt->second.convert<qreal>();
    const auto offsetFrac = offset-size_t(offset);

    //extract level
    const auto levelIt = packet.metadata.find("level");
    const auto level = (levelIt == packet.metadata.end())?0:levelIt->second.convert<qreal>();

    //extract and convert buffer
    const auto &buff = packet.payload;
    Pothos::BufferChunk buffI, buffQ;
    if (buff.dtype.isComplex())
    {
        const auto outs = buff.convertComplex(typeid(float));
        buffI = outs.first; buffQ = outs.second;
        const auto sampsI = outs.first.as<const float *>();
        const auto sampsQ = outs.second.as<const float *>();
        QVector<QPointF> pointsI(buff.elements());
        QVector<QPointF> pointsQ(buff.elements());
        for (int i = 0; i < pointsI.size(); i++)
        {
            const auto x = (i-offsetFrac)/_sampleRateWoAxisUnits;
            pointsI[i] = QPointF(x, sampsI[i]);
            pointsQ[i] = QPointF(x, sampsQ[i]);
        }
        this->getCurve(index, 0)->setSamples(pointsI);
        this->getCurve(index, 1)->setSamples(pointsQ);
    }
    else
    {
        buffI = buff.convert(typeid(float));
        const auto samps = buffI.as<const float *>();
        QVector<QPointF> points(buff.elements());
        for (int i = 0; i < points.size(); i++)
        {
            const auto x = (i-offsetFrac)/_sampleRateWoAxisUnits;
            points[i] = QPointF(x, samps[i]);
        }
        this->getCurve(index, 0)->setSamples(points);
    }

    //create markers from labels
    auto &markers = _markers[index];
    markers.clear(); //clear old markers
    const auto samps = buffI.as<const float *>();
    for (const auto &label : packet.labels)
    {
        auto marker = new QwtPlotMarker();
        marker->setLabel(MyMarkerLabel(QString::fromStdString(label.id)));
        marker->setLabelAlignment(Qt::AlignHCenter);
        const auto i = label.index + (label.width-1)/2.0;
        marker->setXValue((i-offsetFrac)/_sampleRateWoAxisUnits);
        marker->setYValue(samps[label.index]);
        marker->attach(_mainPlot);
        markers.emplace_back(marker);
        if (label.id == "T")
        {
            marker->setLabel(_triggerMarkerLabel);
            marker->setXValue(i/_sampleRateWoAxisUnits);
            marker->setYValue(level);
        }
    }

    _mainPlot->replot();
}

void WaveMonitorDisplay::work(void)
{
    auto inPort = this->input(0);

    if (not inPort->hasMessage()) return;
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
        const auto indexIt = packet.metadata.find("index");
        const auto index = (indexIt == packet.metadata.end())?0:indexIt->second.convert<size_t>();

        //ensure that we have allocated depth counters (used to avoid displaying old data)
        if (not _queueDepth[index]) _queueDepth[index].reset(new std::atomic<size_t>(0));

        //send the entire packet into the qt domain for processing
        _queueDepth[index]->fetch_add(1);
        QMetaObject::invokeMethod(this, "handleSamples", Qt::QueuedConnection,
            Q_ARG(Pothos::Packet, packet));
    }
}
