// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "ConstellationDisplay.hpp"
#include "MyPlotStyler.hpp"
#include "MyPlotPicker.hpp"
#include "MyPlotUtils.hpp"
#include <QResizeEvent>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <QHBoxLayout>

ConstellationDisplay::ConstellationDisplay(void):
    _mainPlot(new MyQwtPlot(this)),
    _plotGrid(new QwtPlotGrid()),
    _zoomer(new MyPlotPicker(_mainPlot->canvas())),
    _autoScale(false),
    _queueDepth(0)
{
    //setup block
    this->registerCall(this, POTHOS_FCN_TUPLE(ConstellationDisplay, widget));
    this->registerCall(this, POTHOS_FCN_TUPLE(ConstellationDisplay, setTitle));
    this->registerCall(this, POTHOS_FCN_TUPLE(ConstellationDisplay, setAutoScale));
    this->registerCall(this, POTHOS_FCN_TUPLE(ConstellationDisplay, title));
    this->registerCall(this, POTHOS_FCN_TUPLE(ConstellationDisplay, autoScale));
    this->registerCall(this, POTHOS_FCN_TUPLE(ConstellationDisplay, setXRange));
    this->registerCall(this, POTHOS_FCN_TUPLE(ConstellationDisplay, setYRange));
    this->registerCall(this, POTHOS_FCN_TUPLE(ConstellationDisplay, enableXAxis));
    this->registerCall(this, POTHOS_FCN_TUPLE(ConstellationDisplay, enableYAxis));
    this->setupInput(0);

    //layout
    auto layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(QMargins());
    layout->addWidget(_mainPlot);

    //setup plotter
    {
        _mainPlot->setCanvasBackground(MyPlotCanvasBg());
        connect(_zoomer, SIGNAL(zoomed(const QRectF &)), this, SLOT(handleZoomed(const QRectF &)));
        _mainPlot->setAxisFont(QwtPlot::xBottom, MyPlotAxisFontSize());
        _mainPlot->setAxisFont(QwtPlot::yLeft, MyPlotAxisFontSize());
    }

    //setup grid
    {
        _plotGrid->attach(_mainPlot);
        _plotGrid->setPen(MyPlotGridPen());
    }

    //register types passed to gui thread from work
    qRegisterMetaType<Pothos::BufferChunk>("Pothos::BufferChunk");
}

ConstellationDisplay::~ConstellationDisplay(void)
{
    return;
}

void ConstellationDisplay::setTitle(const QString &title)
{
    QMetaObject::invokeMethod(_mainPlot, "setTitle", Qt::QueuedConnection, Q_ARG(QwtText, MyPlotTitle(title)));
}

void ConstellationDisplay::setAutoScale(const bool autoScale)
{
    _autoScale = autoScale;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void ConstellationDisplay::setXRange(const std::vector<double> &range)
{
    if (range.size() != 2) throw Pothos::RangeException("ConstellationDisplay::setXRange()", "range vector must be size 2");
    _xRange = range;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void ConstellationDisplay::setYRange(const std::vector<double> &range)
{
    if (range.size() != 2) throw Pothos::RangeException("ConstellationDisplay::setYRange()", "range vector must be size 2");
    _yRange = range;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void ConstellationDisplay::handleUpdateAxis(void)
{
    if (_xRange.size() == 2) _mainPlot->setAxisScale(QwtPlot::xBottom, _xRange[0], _xRange[1]);
    if (_yRange.size() == 2) _mainPlot->setAxisScale(QwtPlot::yLeft, _yRange[0], _yRange[1]);

    _mainPlot->setAxisTitle(QwtPlot::xBottom, MyPlotAxisTitle("In-Phase"));
    _mainPlot->setAxisTitle(QwtPlot::yLeft, MyPlotAxisTitle("Quadrature"));

    _mainPlot->updateAxes(); //update after axis changes
    _zoomer->setZoomBase(); //record current axis settings
    this->handleZoomed(_zoomer->zoomBase()); //reload
}

void ConstellationDisplay::handleZoomed(const QRectF &rect)
{
    //when zoomed all the way out, return to autoscale
    if (rect == _zoomer->zoomBase() and _autoScale)
    {
        _mainPlot->setAxisAutoScale(QwtPlot::xBottom);
        _mainPlot->setAxisAutoScale(QwtPlot::yLeft);
    }
}

QString ConstellationDisplay::title(void) const
{
    return _mainPlot->title().text();
}

void ConstellationDisplay::enableXAxis(const bool enb)
{
    _mainPlot->enableAxis(QwtPlot::xBottom, enb);
}

void ConstellationDisplay::enableYAxis(const bool enb)
{
    _mainPlot->enableAxis(QwtPlot::yLeft, enb);
}
