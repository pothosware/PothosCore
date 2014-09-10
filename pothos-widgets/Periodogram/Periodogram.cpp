// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Periodogram.hpp"
#include "MyPlotStyler.hpp"
#include "MyPlotPicker.hpp"
#include "MyPlotUtils.hpp"
#include <QResizeEvent>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <QHBoxLayout>

Periodogram::Periodogram(const Pothos::DType &dtype):
    _mainPlot(new MyQwtPlot(this)),
    _plotGrid(new QwtPlotGrid()),
    _zoomer(new MyPlotPicker(_mainPlot->canvas())),
    _displayRate(1.0),
    _sampleRate(1.0),
    _sampleRateWoAxisUnits(1.0),
    _numBins(1024),
    _refLevel(0.0),
    _dynRange(100.0),
    _autoScale(false)
{
    //setup block
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, widget));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, setNumInputs));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, setTitle));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, setDisplayRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, setSampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, setNumFFTBins));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, setWindowFunction));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, setReferenceLevel));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, setDynamicRange));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, setAutoScale));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, numInputs));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, title));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, displayRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, sampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, numFFTBins));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, referenceLevel));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, dynamicRange));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, autoScale));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, enableXAxis));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, enableYAxis));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, setYAxisTitle));
    this->registerSignal("frequencySelected");
    this->setupInput(0, dtype);

    //layout
    auto layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(QMargins());
    layout->addWidget(_mainPlot);

    //setup plotter
    {
        _mainPlot->setCanvasBackground(MyPlotCanvasBg());
        connect(_zoomer, SIGNAL(selected(const QPointF &)), this, SLOT(handlePickerSelected(const QPointF &)));
        connect(_zoomer, SIGNAL(zoomed(const QRectF &)), this, SLOT(handleZoomed(const QRectF &)));
        _mainPlot->setAxisFont(QwtPlot::xBottom, MyPlotAxisFontSize());
        _mainPlot->setAxisFont(QwtPlot::yLeft, MyPlotAxisFontSize());
    }

    //setup grid
    {
        _plotGrid->attach(_mainPlot);
        _plotGrid->setPen(MyPlotGridPen());
    }
}

Periodogram::~Periodogram(void)
{
    return;
}

void Periodogram::setNumInputs(const size_t numInputs)
{
    for (size_t i = this->inputs().size(); i < numInputs; i++)
    {
        this->setupInput(i, this->input(0)->dtype());
    }
}

void Periodogram::setTitle(const QString &title)
{
    QMetaObject::invokeMethod(_mainPlot, "setTitle", Qt::QueuedConnection, Q_ARG(QwtText, MyPlotTitle(title)));
}

void Periodogram::setDisplayRate(const double displayRate)
{
    _displayRate = displayRate;
}

void Periodogram::setSampleRate(const double sampleRate)
{
    _sampleRate = sampleRate;
    QString axisTitle("Hz");
    _sampleRateWoAxisUnits = sampleRate;
    if (sampleRate >= 2e9)
    {
        _sampleRateWoAxisUnits /= 1e9;
        axisTitle = "GHz";
    }
    else if (sampleRate >= 2e6)
    {
        _sampleRateWoAxisUnits /= 1e6;
        axisTitle = "MHz";
    }
    else if (sampleRate >= 2e3)
    {
        _sampleRateWoAxisUnits /= 1e3;
        axisTitle = "kHz";
    }
    QMetaObject::invokeMethod(_mainPlot, "setAxisTitle", Qt::QueuedConnection, Q_ARG(int, QwtPlot::xBottom), Q_ARG(QwtText, MyPlotAxisTitle(axisTitle)));
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void Periodogram::setNumFFTBins(const size_t numBins)
{
    _numBins = numBins;
    for (auto inPort : this->inputs()) inPort->setReserve(_numBins);
}

void Periodogram::setWindowFunction(const std::string &windowType)
{
    _window = WindowFunction(windowType);
}

void Periodogram::setReferenceLevel(const double refLevel)
{
    _refLevel = refLevel;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void Periodogram::setDynamicRange(const double dynRange)
{
    _dynRange = dynRange;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void Periodogram::setAutoScale(const bool autoScale)
{
    _autoScale = autoScale;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void Periodogram::handleUpdateAxis(void)
{
    _zoomer->setAxis(QwtPlot::xBottom, QwtPlot::yLeft);
    _mainPlot->setAxisScale(QwtPlot::xBottom, -_sampleRateWoAxisUnits/2, +_sampleRateWoAxisUnits/2);
    _mainPlot->setAxisScale(QwtPlot::yLeft, _refLevel-_dynRange, _refLevel);
    _zoomer->setZoomBase(); //record current axis settings
    this->handleZoomed(_zoomer->zoomBase()); //reload
}

void Periodogram::handleZoomed(const QRectF &rect)
{
    //when zoomed all the way out, return to autoscale
    if (rect == _zoomer->zoomBase() and _autoScale)
    {
        _mainPlot->setAxisAutoScale(QwtPlot::yLeft);
    }
}

QString Periodogram::title(void) const
{
    return _mainPlot->title().text();
}

void Periodogram::enableXAxis(const bool enb)
{
    _mainPlot->enableAxis(QwtPlot::xBottom, enb);
}

void Periodogram::enableYAxis(const bool enb)
{
    _mainPlot->enableAxis(QwtPlot::yLeft, enb);
}

void Periodogram::setYAxisTitle(const QString &title)
{
    QMetaObject::invokeMethod(_mainPlot, "setAxisTitle", Qt::QueuedConnection, Q_ARG(int, QwtPlot::yLeft), Q_ARG(QwtText, MyPlotAxisTitle(title)));
}

void Periodogram::installLegend(void)
{
    auto legend = new QwtLegend(_mainPlot);
    legend->setDefaultItemMode(QwtLegendData::Checkable);
    connect(legend, SIGNAL(checked(const QVariant &, bool, int)), this, SLOT(handleLegendChecked(const QVariant &, bool, int)));
    _mainPlot->insertLegend(legend);
}

void Periodogram::handleLegendChecked(const QVariant &itemInfo, bool on, int)
{
    _mainPlot->infoToItem(itemInfo)->setVisible(not on);
}

void Periodogram::handlePickerSelected(const QPointF &p)
{
    const double freq = p.x()*_sampleRate/_sampleRateWoAxisUnits;
    this->callVoid("frequencySelected", freq);
}

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerPeriodogram(
    "/widgets/periodogram", &Periodogram::make);
