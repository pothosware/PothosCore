// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Spectrogram.hpp"
#include "MyPlotStyler.hpp"
#include "MyPlotPicker.hpp"
#include "SpectrogramRaster.hpp"
#include <QTimer>
#include <QResizeEvent>
#include <qwt_plot.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot_spectrocurve.h>
#include <qwt_scale_widget.h>
#include <qwt_color_map.h>
#include <qwt_legend.h>
#include <QHBoxLayout>
#include <iostream>

Spectrogram::Spectrogram(const Pothos::DType &dtype):
    _replotTimer(new QTimer(this)),
    _mainPlot(new QwtPlot(this)),
    _plotSpect(new QwtPlotSpectrogram()),
    _plotRaster(new MySpectrogramRasterData()),
    _sampleRate(1.0),
    _sampleRateWoAxisUnits(1.0),
    _numBins(1024),
    _timeSpan(10.0)
{
    //setup block
    this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, widget));
    this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, setTitle));
    this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, setDisplayRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, setSampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, setNumFFTBins));
    this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, setTimeSpan));
    this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, title));
    this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, displayRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, sampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, numFFTBins));
    this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, timeSpan));
    this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, enableXAxis));
    this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, enableYAxis));
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
        qRegisterMetaType<std::valarray<float>>("std::valarray<float>");
        _mainPlot->setCanvasBackground(MyPlotCanvasBg());
        _mainPlot->setAxisScale(QwtPlot::yRight, -100, 0);
        _plotRaster->setInterval(Qt::ZAxis, QwtInterval(-100, 0));
        auto picker = new MyPlotPicker(_mainPlot->canvas());
        connect(picker, SIGNAL(selected(const QPointF &)), this, SLOT(handlePickerSelected(const QPointF &)));
        _mainPlot->setAxisFont(QwtPlot::xBottom, MyPlotAxisFontSize());
        _mainPlot->setAxisFont(QwtPlot::yLeft, MyPlotAxisFontSize());
        _mainPlot->setAxisFont(QwtPlot::yRight, MyPlotAxisFontSize());
        _mainPlot->setAxisTitle(QwtPlot::yRight, MyPlotAxisTitle("dB"));
        _mainPlot->plotLayout()->setAlignCanvasToScales(true);
        _mainPlot->enableAxis(QwtPlot::yRight);
        _mainPlot->axisWidget(QwtPlot::yRight)->setColorBarEnabled(true);
    }

    //setup spectrogram plot item
    {
        _plotSpect->attach(_mainPlot);
        _plotSpect->setData(_plotRaster);
        _plotSpect->setColorMap(this->makeColorMap());
        _mainPlot->axisWidget(QwtPlot::yRight)->setColorMap(_plotRaster->interval(Qt::ZAxis), this->makeColorMap());
        _plotSpect->setDisplayMode(QwtPlotSpectrogram::ImageMode, true);
        _plotSpect->setRenderThreadCount(0); //enable multi-thread
    }

    connect(_replotTimer, SIGNAL(timeout(void)), _mainPlot, SLOT(replot(void)));
}

Spectrogram::~Spectrogram(void)
{
    return;
}

void Spectrogram::setTitle(const QString &title)
{
    _mainPlot->setTitle(MyPlotTitle(title));
}

void Spectrogram::setDisplayRate(const double displayRate)
{
    _displayRate = displayRate;
    _replotTimer->setInterval(int(1000/_displayRate));
}

void Spectrogram::setSampleRate(const double sampleRate)
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
    _plotRaster->setInterval(Qt::XAxis, QwtInterval(-_sampleRateWoAxisUnits/2, +_sampleRateWoAxisUnits/2));
}

void Spectrogram::setNumFFTBins(const size_t numBins)
{
    _numBins = numBins;
    for (auto inPort : this->inputs()) inPort->setReserve(_numBins);
    _plotRaster->setNumColumns(numBins);
}

void Spectrogram::setTimeSpan(const double timeSpan)
{
    _timeSpan = timeSpan;
    QString axisTitle("secs");
    if (_timeSpan <= 100e-9)
    {
        _timeSpan *= 1e9;
        axisTitle = "nsecs";
    }
    else if (_timeSpan <= 100e-6)
    {
        _timeSpan *= 1e6;
        axisTitle = "usecs";
    }
    else if (_timeSpan <= 100e-3)
    {
        _timeSpan *= 1e3;
        axisTitle = "msecs";
    }
    _mainPlot->setAxisTitle(QwtPlot::yLeft, MyPlotAxisTitle(axisTitle));
    _mainPlot->setAxisScale(QwtPlot::yLeft, 0, _timeSpan);
    _plotRaster->setInterval(Qt::YAxis, QwtInterval(0, _timeSpan));
}

QString Spectrogram::title(void) const
{
    return _mainPlot->title().text();
}

void Spectrogram::enableXAxis(const bool enb)
{
    _mainPlot->enableAxis(QwtPlot::xBottom, enb);
}

void Spectrogram::enableYAxis(const bool enb)
{
    _mainPlot->enableAxis(QwtPlot::yLeft, enb);
}

void Spectrogram::handlePickerSelected(const QPointF &p)
{
    const double freq = p.x()*_sampleRate/_sampleRateWoAxisUnits;
    this->callVoid("frequencySelected", freq);
}

void Spectrogram::appendBins(const std::valarray<float> &bins)
{
    _plotRaster->appendBins(bins);
}

QwtColorMap *Spectrogram::makeColorMap(void) const
{
    auto cMap = new QwtLinearColorMap(Qt::darkCyan, Qt::red);
    cMap->addColorStop(0.1, Qt::cyan);
    cMap->addColorStop(0.6, Qt::green);
    cMap->addColorStop(0.95, Qt::yellow);
    return cMap;
}

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerSpectrogram(
    "/widgets/spectrogram", &Spectrogram::make);
