// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "WaveMonitorDisplay.hpp"
#include "MyPlotStyler.hpp"
#include "MyPlotPicker.hpp"
#include "MyPlotUtils.hpp"
#include <QResizeEvent>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <QHBoxLayout>
#include <iostream>

WaveMonitorDisplay::WaveMonitorDisplay(void):
    _mainPlot(new MyQwtPlot(this)),
    _plotGrid(new QwtPlotGrid()),
    _zoomer(new MyPlotPicker(_mainPlot->canvas())),
    _sampleRate(1.0),
    _sampleRateWoAxisUnits(1.0),
    _numPoints(1024),
    _autoScale(false),
    _rateLabelId("rxRate")
{
    //setup block
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, widget));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, setNumInputs));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, setTitle));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, setSampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, setNumPoints));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, numInputs));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, title));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, sampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, numPoints));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, setAutoScale));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, setYRange));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, enableXAxis));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, enableYAxis));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, setYAxisTitle));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, setChannelLabel));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, setChannelStyle));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitorDisplay, setRateLabelId));
    this->setupInput(0);

    //layout
    auto layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(QMargins());
    layout->addWidget(_mainPlot);

    //setup plotter
    {
        _mainPlot->setCanvasBackground(MyPlotCanvasBg());
        _mainPlot->setAxisFont(QwtPlot::xBottom, MyPlotAxisFontSize());
        _mainPlot->setAxisFont(QwtPlot::yLeft, MyPlotAxisFontSize());
        connect(_zoomer, SIGNAL(zoomed(const QRectF &)), this, SLOT(handleZoomed(const QRectF &)));
    }

    //setup grid
    {
        _plotGrid->attach(_mainPlot);
        _plotGrid->setPen(MyPlotGridPen());
    }

    //register types passed to gui thread from work
    qRegisterMetaType<Pothos::BufferChunk>("Pothos::BufferChunk");
    qRegisterMetaType<std::vector<Pothos::Label>>("std::vector<Pothos::Label>");
}

WaveMonitorDisplay::~WaveMonitorDisplay(void)
{
    return;
}

void WaveMonitorDisplay::setNumInputs(const size_t numInputs)
{
    for (size_t i = this->inputs().size(); i < numInputs; i++)
    {
        this->setupInput(i, this->input(0)->dtype());
    }
}

void WaveMonitorDisplay::setTitle(const QString &title)
{
    QMetaObject::invokeMethod(_mainPlot, "setTitle", Qt::QueuedConnection, Q_ARG(QwtText, MyPlotTitle(title)));
}

void WaveMonitorDisplay::setSampleRate(const double sampleRate)
{
    _sampleRate = sampleRate;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void WaveMonitorDisplay::setNumPoints(const size_t numPoints)
{
    _numPoints = numPoints;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

QString WaveMonitorDisplay::title(void) const
{
    return _mainPlot->title().text();
}

void WaveMonitorDisplay::setAutoScale(const bool autoScale)
{
    _autoScale = autoScale;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void WaveMonitorDisplay::setYRange(const std::vector<double> &range)
{
    if (range.size() != 2) throw Pothos::RangeException("WaveMonitorDisplay::setYRange()", "range vector must be size 2");
    _yRange = range;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void WaveMonitorDisplay::enableXAxis(const bool enb)
{
    _mainPlot->enableAxis(QwtPlot::xBottom, enb);
}

void WaveMonitorDisplay::enableYAxis(const bool enb)
{
    _mainPlot->enableAxis(QwtPlot::yLeft, enb);
}

void WaveMonitorDisplay::setYAxisTitle(const QString &title)
{
    QMetaObject::invokeMethod(_mainPlot, "setAxisTitle", Qt::QueuedConnection, Q_ARG(int, QwtPlot::yLeft), Q_ARG(QwtText, MyPlotAxisTitle(title)));
}

void WaveMonitorDisplay::handleUpdateAxis(void)
{
    if (_yRange.size() == 2) _mainPlot->setAxisScale(QwtPlot::yLeft, _yRange[0], _yRange[1]);

    QString timeAxisTitle("secs");
    double factor = 1.0;

    double timeSpan = _numPoints/_sampleRate;
    if (timeSpan <= 100e-9)
    {
        factor = 1e9;
        timeAxisTitle = "nsecs";
    }
    else if (timeSpan <= 100e-6)
    {
        factor = 1e6;
        timeAxisTitle = "usecs";
    }
    else if (timeSpan <= 100e-3)
    {
        factor = 1e3;
        timeAxisTitle = "msecs";
    }
    _mainPlot->setAxisTitle(QwtPlot::xBottom, MyPlotAxisTitle(timeAxisTitle));

    _zoomer->setAxis(QwtPlot::xBottom, QwtPlot::yLeft);
    _sampleRateWoAxisUnits = _sampleRate/factor;
    _mainPlot->setAxisScale(QwtPlot::xBottom, 0, _numPoints/_sampleRateWoAxisUnits);
    _mainPlot->updateAxes(); //update after axis changes
    _zoomer->setZoomBase(); //record current axis settings
    this->handleZoomed(_zoomer->zoomBase()); //reload
}

void WaveMonitorDisplay::handleUpdateCurves(void)
{
    size_t count = 0;

    for (auto &pair : _curves)
    {
        const auto &index = pair.first;
        auto &curves = pair.second;
        const auto &label = _channelLabels[index];
        const auto &styleStr = _channelStyles[index];

        Qt::PenStyle style(Qt::SolidLine);
        if (styleStr == "LINE") style = Qt::SolidLine;
        if (styleStr == "DASH") style = Qt::DashLine;
        if (styleStr == "DOTS") style = Qt::DotLine;

        qreal width = 1.0;
        if (style != Qt::SolidLine) width += 0.5;

        for (const auto &curvePair : curves)
        {
            const auto &curve = curvePair.second;
            const auto color = pastelize(getDefaultCurveColor(count++));
            curve->setPen(color, width, style);
            curve->detach();
            curve->attach(_mainPlot);
            _mainPlot->updateChecked(curve.get());
        }

        if (label.isEmpty())
        {
            if (curves.size() == 1)
            {
                curves[0]->setTitle(QString("Ch%1").arg(index));
            }
            if (curves.size() == 2)
            {
                curves[0]->setTitle(QString("Re%1").arg(index));
                curves[1]->setTitle(QString("Im%1").arg(index));
            }
        }
        else
        {
            if (curves.size() == 1)
            {
                curves[0]->setTitle(label);
            }
            if (curves.size() == 2)
            {
                curves[0]->setTitle(label+"I");
                curves[1]->setTitle(label+"Q");
            }
        }
    }

    if (count > 1) this->installLegend();
    _mainPlot->replot();
}

void WaveMonitorDisplay::handleZoomed(const QRectF &rect)
{
    //when zoomed all the way out, return to autoscale
    if (rect == _zoomer->zoomBase() and _autoScale)
    {
        _mainPlot->setAxisAutoScale(QwtPlot::yLeft);
        _mainPlot->updateAxes(); //update after axis changes
    }
}

void WaveMonitorDisplay::installLegend(void)
{
    if (_mainPlot->legend() != nullptr) return;
    auto legend = new QwtLegend(_mainPlot);
    legend->setDefaultItemMode(QwtLegendData::Checkable);
    connect(legend, SIGNAL(checked(const QVariant &, bool, int)), this, SLOT(handleLegendChecked(const QVariant &, bool, int)));
    _mainPlot->insertLegend(legend);
}

void WaveMonitorDisplay::handleLegendChecked(const QVariant &itemInfo, bool on, int)
{
    _mainPlot->infoToItem(itemInfo)->setVisible(on);
    _mainPlot->replot();
}
