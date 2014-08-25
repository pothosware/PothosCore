// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MyPlotStyler.hpp"
#include "MyPlotPicker.hpp"
#include "Periodogram.hpp"
#include <QResizeEvent>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <QHBoxLayout>

Periodogram::Periodogram(const Pothos::DType &dtype):
    _mainPlot(new QwtPlot(this)),
    _plotGrid(new QwtPlotGrid()),
    _displayRate(1.0),
    _sampleRate(1.0),
    _sampleRateWoAxisUnits(1.0),
    _numBins(1024)
{
    //setup block
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, widget));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, setNumInputs));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, setTitle));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, setDisplayRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, setSampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, setNumFFTBins));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, numInputs));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, title));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, displayRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, sampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(Periodogram, numFFTBins));
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
        //missing from qwt:
        qRegisterMetaType<QList<QwtLegendData>>("QList<QwtLegendData>");
        qRegisterMetaType<std::valarray<double>>("std::valarray<double>");
        _mainPlot->setCanvasBackground(MyPlotCanvasBg());
        _mainPlot->setAxisScale(QwtPlot::yLeft, -100, 0);
        auto picker = new MyPlotPicker(_mainPlot->canvas());
        connect(picker, SIGNAL(selected(const QPointF &)), this, SLOT(handlePickerSelected(const QPointF &)));
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
    _mainPlot->setTitle(MyPlotTitle(title));
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
    _mainPlot->setAxisTitle(QwtPlot::xBottom, MyPlotAxisTitle(axisTitle));
    _mainPlot->setAxisScale(QwtPlot::xBottom, -_sampleRateWoAxisUnits/2, +_sampleRateWoAxisUnits/2);
}

void Periodogram::setNumFFTBins(const size_t numBins)
{
    _numBins = numBins;
    for (auto inPort : this->inputs()) inPort->setReserve(_numBins);
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
    _mainPlot->setAxisTitle(QwtPlot::yLeft, MyPlotAxisTitle(title));
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
