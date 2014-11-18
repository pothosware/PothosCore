// Copyright (c) 2014-2014 Josh Blum
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
void WaveMonitorDisplay::handleSamples(const int index, const int whichCurve, const std::valarray<float> &samps, const std::vector<Pothos::Label> &labels)
{
    QVector<QPointF> points(samps.size());
    for (size_t i = 0; i < samps.size(); i++)
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
        if (label.index >= samps.size()) break;
        marker->setLabel(MyMarkerLabel(QString::fromStdString(label.id)));
        marker->setLabelAlignment(Qt::AlignHCenter);
        marker->setXValue(label.index/_sampleRateWoAxisUnits);
        marker->setYValue(samps[label.index]);
        marker->attach(_mainPlot);
        markers.emplace_back(marker);
    }
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

        //packet-based messages have payloads to FFT
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

            QMetaObject::invokeMethod(this, "handleSamples", Qt::QueuedConnection,
                Q_ARG(int, inPort->index()), Q_ARG(int, 0),
                Q_ARG(std::valarray<float>, std::valarray<float>(floatBuffs[0].as<const float *>(), floatBuffs[0].elements())),
                Q_ARG(std::vector<Pothos::Label>, packet.labels));

            const bool hasIm = floatBuffs.size() > 1;
            if (hasIm) QMetaObject::invokeMethod(this, "handleSamples", Qt::QueuedConnection,
                Q_ARG(int, inPort->index()), Q_ARG(int, 1),
                Q_ARG(std::valarray<float>, std::valarray<float>(floatBuffs[1].as<const float *>(), floatBuffs[1].elements())),
                Q_ARG(std::vector<Pothos::Label>, std::vector<Pothos::Label>()));
        }
    }

    QMetaObject::invokeMethod(_mainPlot, "replot", Qt::QueuedConnection);
}
