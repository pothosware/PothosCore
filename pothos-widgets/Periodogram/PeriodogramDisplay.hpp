// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <qwt_math.h> //_USE_MATH_DEFINES
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <QWidget>
#include <memory>
#include <map>
#include <vector>
#include <atomic>
#include "MyFFTUtils.hpp"

class MyQwtPlot;
class QwtPlotGrid;
class QwtPlotCurve;
class QwtPlotZoomer;
class PeriodogramChannel;

class PeriodogramDisplay : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    PeriodogramDisplay(void);

    ~PeriodogramDisplay(void);

    QWidget *widget(void)
    {
        return this;
    }

    void setNumInputs(const size_t numInputs);

    //! set the plotter's title
    void setTitle(const QString &title);

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
    void setReferenceLevel(const double refLevel);
    void setDynamicRange(const double dynRange);
    void setAutoScale(const bool autoScale);

    QString title(void) const;

    size_t numInputs(void) const
    {
        return this->inputs().size();
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

    double referenceLevel(void) const
    {
        return _refLevel;
    }

    double dynamicRange(void) const
    {
        return _dynRange;
    }

    bool autoScale(void) const
    {
        return _autoScale;
    }

    void enableXAxis(const bool enb);
    void enableYAxis(const bool enb);
    void setYAxisTitle(const QString &title);

    void setFreqLabelId(const std::string &id)
    {
        _freqLabelId = id;
    }

    void setRateLabelId(const std::string &id)
    {
        _rateLabelId = id;
    }

    void setAverageFactor(const double factor)
    {
        if (factor > 1.0 or factor < 0.0) throw Pothos::RangeException(
            "Periodogram::setAverageFactor("+std::to_string(factor)+")",
            "factor must be in [1.0, 0.0]");
        _averageFactor = factor;
    }

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
    void handlePowerBins(const int index, const std::valarray<float> &bins);
    void handleUpdateAxis(void);
    void handleZoomed(const QRectF &rect);

private:
    MyQwtPlot *_mainPlot;
    QwtPlotGrid *_plotGrid;
    QwtPlotZoomer *_zoomer;
    Pothos::Proxy _window;
    double _sampleRate;
    double _sampleRateWoAxisUnits;
    double _centerFreq;
    double _centerFreqWoAxisUnits;
    size_t _numBins;
    double _refLevel;
    double _dynRange;
    bool _autoScale;
    std::string _freqLabelId;
    std::string _rateLabelId;
    double _averageFactor;

    //per-port data structs
    std::map<size_t, std::shared_ptr<PeriodogramChannel>> _curves;
    std::map<size_t, std::shared_ptr<std::atomic<size_t>>> _queueDepth;
};
