// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PeriodogramDisplay.hpp"
#include "MyPlotStyler.hpp"
#include "MyPlotPicker.hpp"
#include "MyPlotUtils.hpp"
#include <QResizeEvent>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <QHBoxLayout>
#include <algorithm> //min/max

PeriodogramDisplay::PeriodogramDisplay(void):
    _mainPlot(new MyQwtPlot(this)),
    _plotGrid(new QwtPlotGrid()),
    _zoomer(new MyPlotPicker(_mainPlot->canvas())),
    _sampleRate(1.0),
    _sampleRateWoAxisUnits(1.0),
    _centerFreq(0.0),
    _centerFreqWoAxisUnits(0.0),
    _numBins(1024),
    _refLevel(0.0),
    _dynRange(100.0),
    _autoScale(false),
    _freqLabelId("rxFreq"),
    _rateLabelId("rxRate"),
    _averageFactor(0.0)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    _window = env->findProxy("Pothos/Util/WindowFunction").callProxy("new");

    //setup block
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, widget));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, setTitle));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, setSampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, setCenterFrequency));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, setNumFFTBins));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, setWindowType));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, setReferenceLevel));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, setDynamicRange));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, setAutoScale));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, title));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, sampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, centerFrequency));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, numFFTBins));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, referenceLevel));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, dynamicRange));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, autoScale));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, setAverageFactor));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, enableXAxis));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, enableYAxis));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, setYAxisTitle));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, setFreqLabelId));
    this->registerCall(this, POTHOS_FCN_TUPLE(PeriodogramDisplay, setRateLabelId));
    this->registerSignal("frequencySelected");
    this->setupInput(0);

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

        auto legend = new QwtLegend(_mainPlot);
        legend->setDefaultItemMode(QwtLegendData::Checkable);
        _mainPlot->insertLegend(legend);
    }

    //setup grid
    {
        _plotGrid->attach(_mainPlot);
        _plotGrid->setPen(MyPlotGridPen());
    }
}

PeriodogramDisplay::~PeriodogramDisplay(void)
{
    return;
}

void PeriodogramDisplay::setTitle(const QString &title)
{
    QMetaObject::invokeMethod(_mainPlot, "setTitle", Qt::QueuedConnection, Q_ARG(QwtText, MyPlotTitle(title)));
}

void PeriodogramDisplay::setSampleRate(const double sampleRate)
{
    _sampleRate = sampleRate;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void PeriodogramDisplay::setCenterFrequency(const double freq)
{
    _centerFreq = freq;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void PeriodogramDisplay::setNumFFTBins(const size_t numBins)
{
    _numBins = numBins;
    _window.callVoid("setSize", numBins);
}

void PeriodogramDisplay::setWindowType(const std::string &windowType)
{
    _window.callVoid("setType", windowType);
}

void PeriodogramDisplay::setReferenceLevel(const double refLevel)
{
    _refLevel = refLevel;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void PeriodogramDisplay::setDynamicRange(const double dynRange)
{
    _dynRange = dynRange;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void PeriodogramDisplay::setAutoScale(const bool autoScale)
{
    _autoScale = autoScale;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void PeriodogramDisplay::handleUpdateAxis(void)
{
    QString axisTitle("Hz");
    double factor = std::max(_sampleRate, _centerFreq);
    if (factor >= 2e9)
    {
        factor = 1e9;
        axisTitle = "GHz";
    }
    else if (factor >= 2e6)
    {
        factor = 1e6;
        axisTitle = "MHz";
    }
    else if (factor >= 2e3)
    {
        factor = 1e3;
        axisTitle = "kHz";
    }
    _mainPlot->setAxisTitle(QwtPlot::xBottom, MyPlotAxisTitle(axisTitle));

    _zoomer->setAxis(QwtPlot::xBottom, QwtPlot::yLeft);
    _sampleRateWoAxisUnits = _sampleRate/factor;
    _centerFreqWoAxisUnits = _centerFreq/factor;
    _mainPlot->setAxisScale(QwtPlot::xBottom, _centerFreqWoAxisUnits-_sampleRateWoAxisUnits/2, _centerFreqWoAxisUnits+_sampleRateWoAxisUnits/2);
    _mainPlot->setAxisScale(QwtPlot::yLeft, _refLevel-_dynRange, _refLevel);
    _mainPlot->updateAxes(); //update after axis changes
    _zoomer->setZoomBase(); //record current axis settings
    this->handleZoomed(_zoomer->zoomBase()); //reload
}

void PeriodogramDisplay::handleZoomed(const QRectF &rect)
{
    //when zoomed all the way out, return to autoscale
    if (rect == _zoomer->zoomBase() and _autoScale)
    {
        _mainPlot->setAxisAutoScale(QwtPlot::yLeft);
    }
}

QString PeriodogramDisplay::title(void) const
{
    return _mainPlot->title().text();
}

void PeriodogramDisplay::enableXAxis(const bool enb)
{
    _mainPlot->enableAxis(QwtPlot::xBottom, enb);
}

void PeriodogramDisplay::enableYAxis(const bool enb)
{
    _mainPlot->enableAxis(QwtPlot::yLeft, enb);
}

void PeriodogramDisplay::setYAxisTitle(const QString &title)
{
    QMetaObject::invokeMethod(_mainPlot, "setAxisTitle", Qt::QueuedConnection, Q_ARG(int, QwtPlot::yLeft), Q_ARG(QwtText, MyPlotAxisTitle(title)));
}

void PeriodogramDisplay::handlePickerSelected(const QPointF &p)
{
    const double freq = p.x()*_sampleRate/_sampleRateWoAxisUnits;
    this->callVoid("frequencySelected", freq);
}
