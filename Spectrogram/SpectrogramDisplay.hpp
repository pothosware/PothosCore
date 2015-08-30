// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <qwt_math.h> //_USE_MATH_DEFINES
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <QWidget>
#include <memory>
#include <map>
#include <vector>
#include "MyFFTUtils.hpp"

class QTimer;
class MyQwtPlot;
class QwtColorMap;
class QwtPlotZoomer;
class QwtPlotSpectrogram;
class MySpectrogramRasterData;

class SpectrogramDisplay : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    SpectrogramDisplay(void);

    ~SpectrogramDisplay(void);

    QWidget *widget(void)
    {
        return this;
    }

    //! set the plotter's title
    void setTitle(const QString &title);

    /*!
     * update rate for the plotter
     * how often to update the display
     */
    void setDisplayRate(const double displayRate);

    /*!
     * sample rate for the plotter
     * controls the frequency scaling display
     */
    void setSampleRate(const double sampleRate);

    /*!
     * center frequency of the plot
     */
    void setCenterFrequency(const double freq);

    void setNumFFTBins(const size_t numBins);
    void setWindowType(const std::string &windowType);
    void setTimeSpan(const double timeSpan);
    void setReferenceLevel(const double refLevel);
    void setDynamicRange(const double dynRange);

    QString title(void) const;

    double displayRate(void) const
    {
        return _displayRate;
    }

    double sampleRate(void) const
    {
        return _sampleRate;
    }

    double centerFrequency(void) const
    {
        return _centerFreq;
    }

    size_t numFFTBins(void) const
    {
        return _numBins;
    }

    double timeSpan(void) const
    {
        return _timeSpan;
    }

    double referenceLevel(void) const
    {
        return _refLevel;
    }

    double dynamicRange(void) const
    {
        return _dynRange;
    }

    void enableXAxis(const bool enb);
    void enableYAxis(const bool enb);

    void setFreqLabelId(const std::string &id)
    {
        _freqLabelId = id;
    }

    void setRateLabelId(const std::string &id)
    {
        _rateLabelId = id;
    }

    void activate(void);
    void deactivate(void);
    void work(void);

    //allow for standard resize controls with the default size policy
    QSize minimumSizeHint(void) const
    {
        return QSize(300, 150);
    }
    QSize sizeHint(void) const
    {
        return this->minimumSizeHint();
    }

private slots:
    void handlePickerSelected(const QPointF &);
    void appendBins(const std::valarray<float> &bins);
    void handleUpdateAxis(void);

private:
    QTimer *_replotTimer;
    MyQwtPlot *_mainPlot;
    QwtPlotZoomer *_zoomer;
    std::shared_ptr<QwtPlotSpectrogram> _plotSpect;
    MySpectrogramRasterData *_plotRaster;
    Pothos::Proxy _window;
    double _lastUpdateRate;
    double _displayRate;
    double _sampleRate;
    double _sampleRateWoAxisUnits;
    double _centerFreq;
    double _centerFreqWoAxisUnits;
    size_t _numBins;
    double _timeSpan;
    double _refLevel;
    double _dynRange;
    std::string _freqLabelId;
    std::string _rateLabelId;
    QwtColorMap *makeColorMap(void) const;
};
