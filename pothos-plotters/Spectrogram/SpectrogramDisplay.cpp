// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SpectrogramDisplay.hpp"
#include "MyPlotStyler.hpp"
#include "MyPlotPicker.hpp"
#include "MyPlotUtils.hpp"
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
#include <algorithm> //min/max

SpectrogramDisplay::SpectrogramDisplay(void):
    _replotTimer(new QTimer(this)),
    _mainPlot(new MyQwtPlot(this)),
    _zoomer(new MyPlotPicker(_mainPlot->canvas())),
    _plotSpect(new QwtPlotSpectrogram()),
    _plotRaster(new MySpectrogramRasterData()),
    _lastUpdateRate(1.0),
    _displayRate(1.0),
    _sampleRate(1.0),
    _sampleRateWoAxisUnits(1.0),
    _centerFreq(0.0),
    _centerFreqWoAxisUnits(0.0),
    _numBins(1024),
    _timeSpan(10.0),
    _refLevel(0.0),
    _dynRange(100.0),
    _freqLabelId("rxFreq"),
    _rateLabelId("rxRate")
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    _window = env->findProxy("Pothos/Comms/WindowFunction").callProxy("new");

    //setup block
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, widget));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, setTitle));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, setDisplayRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, setSampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, setCenterFrequency));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, setNumFFTBins));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, setWindowType));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, setTimeSpan));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, setReferenceLevel));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, setDynamicRange));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, title));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, displayRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, sampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, centerFrequency));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, numFFTBins));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, timeSpan));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, referenceLevel));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, dynamicRange));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, enableXAxis));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, enableYAxis));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, setFreqLabelId));
    this->registerCall(this, POTHOS_FCN_TUPLE(SpectrogramDisplay, setRateLabelId));
    this->registerSignal("frequencySelected");
    this->registerSignal("updateRateChanged");
    this->setupInput(0);

    //layout
    auto layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(QMargins());
    layout->addWidget(_mainPlot);

    //setup plotter
    {
        _mainPlot->setCanvasBackground(MyPlotCanvasBg());
        dynamic_cast<MyPlotPicker *>(_zoomer)->registerRaster(_plotRaster);
        connect(_zoomer, SIGNAL(selected(const QPointF &)), this, SLOT(handlePickerSelected(const QPointF &)));
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
        _plotSpect->setDisplayMode(QwtPlotSpectrogram::ImageMode, true);
        _plotSpect->setRenderThreadCount(0); //enable multi-thread
    }

    connect(_replotTimer, SIGNAL(timeout(void)), _mainPlot, SLOT(replot(void)));
}

SpectrogramDisplay::~SpectrogramDisplay(void)
{
    return;
}

void SpectrogramDisplay::setTitle(const QString &title)
{
    QMetaObject::invokeMethod(_mainPlot, "setTitle", Qt::QueuedConnection, Q_ARG(QwtText, MyPlotTitle(title)));
}

void SpectrogramDisplay::setDisplayRate(const double displayRate)
{
    _displayRate = displayRate;
    _replotTimer->setInterval(int(1000/_displayRate));
}

void SpectrogramDisplay::setSampleRate(const double sampleRate)
{
    _sampleRate = sampleRate;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void SpectrogramDisplay::setCenterFrequency(const double freq)
{
    _centerFreq = freq;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void SpectrogramDisplay::setNumFFTBins(const size_t numBins)
{
    _numBins = numBins;
    _plotRaster->setNumColumns(numBins);
    _window.callVoid("setSize", numBins);
}

void SpectrogramDisplay::setWindowType(const std::string &windowType)
{
    _window.callVoid("setType", windowType);
}

void SpectrogramDisplay::setTimeSpan(const double timeSpan)
{
    _timeSpan = timeSpan;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

QString SpectrogramDisplay::title(void) const
{
    return _mainPlot->title().text();
}

void SpectrogramDisplay::setReferenceLevel(const double refLevel)
{
    _refLevel = refLevel;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void SpectrogramDisplay::setDynamicRange(const double dynRange)
{
    _dynRange = dynRange;
    QMetaObject::invokeMethod(this, "handleUpdateAxis", Qt::QueuedConnection);
}

void SpectrogramDisplay::handleUpdateAxis(void)
{
    QString timeAxisTitle("secs");
    if (_timeSpan <= 100e-9)
    {
        _timeSpan *= 1e9;
        timeAxisTitle = "nsecs";
    }
    else if (_timeSpan <= 100e-6)
    {
        _timeSpan *= 1e6;
        timeAxisTitle = "usecs";
    }
    else if (_timeSpan <= 100e-3)
    {
        _timeSpan *= 1e3;
        timeAxisTitle = "msecs";
    }
    _mainPlot->setAxisTitle(QwtPlot::yLeft, MyPlotAxisTitle(timeAxisTitle));

    QString freqAxisTitle("Hz");
    double factor = std::max(_sampleRate, _centerFreq);
    if (factor >= 2e9)
    {
        factor = 1e9;
        freqAxisTitle = "GHz";
    }
    else if (factor >= 2e6)
    {
        factor = 1e6;
        freqAxisTitle = "MHz";
    }
    else if (factor >= 2e3)
    {
        factor = 1e3;
        freqAxisTitle = "kHz";
    }
    _mainPlot->setAxisTitle(QwtPlot::xBottom, MyPlotAxisTitle(freqAxisTitle));

    _zoomer->setAxis(QwtPlot::xBottom, QwtPlot::yLeft);

    _sampleRateWoAxisUnits = _sampleRate/factor;
    _centerFreqWoAxisUnits = _centerFreq/factor;

    //update main plot axis
    _mainPlot->setAxisScale(QwtPlot::xBottom, _centerFreqWoAxisUnits-_sampleRateWoAxisUnits/2, _centerFreqWoAxisUnits+_sampleRateWoAxisUnits/2);
    _mainPlot->setAxisScale(QwtPlot::yLeft, 0, _timeSpan);
    _mainPlot->setAxisScale(QwtPlot::yRight, _refLevel-_dynRange, _refLevel);

    _mainPlot->updateAxes(); //update after axis changes before setting raster
    _plotRaster->setInterval(Qt::XAxis, _mainPlot->axisInterval(QwtPlot::xBottom));
    _plotRaster->setInterval(Qt::YAxis, _mainPlot->axisInterval(QwtPlot::yLeft));
    _plotRaster->setInterval(Qt::ZAxis, _mainPlot->axisInterval(QwtPlot::yRight));
    _mainPlot->axisWidget(QwtPlot::yRight)->setColorMap(_plotRaster->interval(Qt::ZAxis), this->makeColorMap());

    _zoomer->setZoomBase(); //record current axis settings
}

void SpectrogramDisplay::enableXAxis(const bool enb)
{
    _mainPlot->enableAxis(QwtPlot::xBottom, enb);
}

void SpectrogramDisplay::enableYAxis(const bool enb)
{
    _mainPlot->enableAxis(QwtPlot::yLeft, enb);
}

void SpectrogramDisplay::handlePickerSelected(const QPointF &p)
{
    const double freq = p.x()*_sampleRate/_sampleRateWoAxisUnits;
    this->callVoid("frequencySelected", freq);
}

void SpectrogramDisplay::appendBins(const std::valarray<float> &bins)
{
    _plotRaster->appendBins(bins);
}

QwtColorMap *SpectrogramDisplay::makeColorMap(void) const
{
    auto cMap = new QwtLinearColorMap(Qt::darkCyan, Qt::red);
    cMap->addColorStop(0.1, Qt::cyan);
    cMap->addColorStop(0.6, Qt::green);
    cMap->addColorStop(0.95, Qt::yellow);
    return cMap;
}
