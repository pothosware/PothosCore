// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MyPlotStyler.hpp"
#include "MyPlotPicker.hpp"
#include "WaveMonitor.hpp"
#include <QResizeEvent>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <QHBoxLayout>

WaveMonitor::WaveMonitor(const Pothos::DType &dtype):
    _mainPlot(new QwtPlot(this)),
    _plotGrid(new QwtPlotGrid()),
    _displayRate(1.0),
    _sampleRate(1.0),
    _timeSpan(1.0),
    _numPoints(1024)
{
    //setup block
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, widget));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, setNumInputs));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, setTitle));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, setDisplayRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, setSampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, setNumPoints));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, numInputs));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, title));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, displayRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, sampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, numPoints));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, enableXAxis));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, enableYAxis));
    this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, setYAxisTitle));
    this->setupInput(0, dtype);

    //layout
    auto layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(QMargins());
    layout->addWidget(_mainPlot);

    //setup plotter
    {
        //missing from qwt:
        qRegisterMetaType<QList<QwtLegendData>>("QList<QwtLegendData>");
        _mainPlot->setCanvasBackground(MyPlotCanvasBg());
        new MyPlotPicker(_mainPlot->canvas());
        _mainPlot->setAxisFont(QwtPlot::xBottom, MyPlotAxisFontSize());
        _mainPlot->setAxisFont(QwtPlot::yLeft, MyPlotAxisFontSize());
    }

    //setup grid
    {
        _plotGrid->attach(_mainPlot);
        _plotGrid->setPen(MyPlotGridPen());
    }

}

WaveMonitor::~WaveMonitor(void)
{
    return;
}

void WaveMonitor::setNumInputs(const size_t numInputs)
{
    for (size_t i = this->inputs().size(); i < numInputs; i++)
    {
        this->setupInput(i, this->input(0)->dtype());
    }
}

void WaveMonitor::setTitle(const QString &title)
{
    _mainPlot->setTitle(MyPlotTitle(title));
}

void WaveMonitor::setDisplayRate(const double displayRate)
{
    _displayRate = displayRate;
}

void WaveMonitor::setSampleRate(const double sampleRate)
{
    _sampleRate = sampleRate;
    this->updateXAxis();
}

void WaveMonitor::setNumPoints(const size_t numPoints)
{
    _numPoints = numPoints;
    for (auto inPort : this->inputs()) inPort->setReserve(_numPoints);
    this->updateXAxis();
}

QString WaveMonitor::title(void) const
{
    return _mainPlot->title().text();
}

void WaveMonitor::enableXAxis(const bool enb)
{
    _mainPlot->enableAxis(QwtPlot::xBottom, enb);
}

void WaveMonitor::enableYAxis(const bool enb)
{
    _mainPlot->enableAxis(QwtPlot::yLeft, enb);
}

void WaveMonitor::setYAxisTitle(const QString &title)
{
    _mainPlot->setAxisTitle(QwtPlot::yLeft, MyPlotAxisTitle(title));
}

void WaveMonitor::updateXAxis(void)
{
    QString axisTitle("s");
    _timeSpan = _numPoints/_sampleRate;
    if (_timeSpan <= 100e-9)
    {
        _timeSpan *= 1e9;
        axisTitle = "ns";
    }
    else if (_timeSpan <= 100e-6)
    {
        _timeSpan *= 1e6;
        axisTitle = "us";
    }
    else if (_timeSpan <= 100e-3)
    {
        _timeSpan *= 1e3;
        axisTitle = "ms";
    }
    _mainPlot->setAxisTitle(QwtPlot::xBottom, MyPlotAxisTitle(axisTitle));
    _mainPlot->setAxisScale(QwtPlot::xBottom, 0, _timeSpan);
}

void WaveMonitor::installLegend(void)
{
    auto legend = new QwtLegend(_mainPlot);
    legend->setDefaultItemMode(QwtLegendData::Checkable);
    connect(legend, SIGNAL(checked(const QVariant &, bool, int)), this, SLOT(handleLegendChecked(const QVariant &, bool, int)));
    _mainPlot->insertLegend(legend);
}

void WaveMonitor::handleLegendChecked(const QVariant &itemInfo, bool on, int)
{
    _mainPlot->infoToItem(itemInfo)->setVisible(not on);
}

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerWaveMonitor(
    "/widgets/wave_monitor", &WaveMonitor::make);
