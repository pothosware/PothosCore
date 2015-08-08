// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <qwt_math.h> //_USE_MATH_DEFINES
#include <Pothos/Framework.hpp>
#include <QWidget>
#include <memory>
#include <map>
#include <atomic>
#include <vector>
#include <qwt_text.h>

class MyQwtPlot;
class QwtPlotGrid;
class QwtPlotCurve;
class QwtPlotZoomer;
class QwtPlotMarker;

class WaveMonitorDisplay : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    WaveMonitorDisplay(void);

    ~WaveMonitorDisplay(void);

    QWidget *widget(void)
    {
        return this;
    }

    //! set the plotter's title
    void setTitle(const QString &title);

    /*!
     * sample rate for the plotter
     * controls the time scaling display
     */
    void setSampleRate(const double sampleRate);

    void setNumPoints(const size_t numPoints);

    void setAutoScale(const bool autoScale);

    void setYRange(const std::vector<double> &range);

    QString title(void) const;

    size_t numInputs(void) const
    {
        return this->inputs().size();
    }

    double sampleRate(void) const
    {
        return _sampleRate;
    }

    size_t numPoints(void) const
    {
        return _numPoints;
    }

    void enableXAxis(const bool enb);
    void enableYAxis(const bool enb);
    void setYAxisTitle(const QString &title);

    void setChannelLabel(const size_t ch, const QString &label)
    {
        _channelLabels[ch] = label;
        QMetaObject::invokeMethod(this, "handleUpdateCurves", Qt::QueuedConnection);
    }

    void setChannelStyle(const size_t ch, const std::string &style)
    {
        _channelStyles[ch] = style;
        QMetaObject::invokeMethod(this, "handleUpdateCurves", Qt::QueuedConnection);
    }

    void setRateLabelId(const std::string &id)
    {
        _rateLabelId = id;
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
    void installLegend(void);
    void handleLegendChecked(const QVariant &, bool, int);
    void handleSamples(const Pothos::Packet &pkt);
    void handleUpdateAxis(void);
    void handleUpdateCurves(void);
    void handleZoomed(const QRectF &rect);

private:
    std::shared_ptr<QwtPlotCurve> &getCurve(const size_t index, const size_t which);

    MyQwtPlot *_mainPlot;
    QwtPlotGrid *_plotGrid;
    QwtPlotZoomer *_zoomer;
    double _sampleRate;
    double _sampleRateWoAxisUnits;
    size_t _numPoints;
    bool _autoScale;
    std::vector<double> _yRange;
    std::string _rateLabelId;
    QwtText _triggerMarkerLabel;

    //channel configs
    std::map<size_t, QString> _channelLabels;
    std::map<size_t, std::string> _channelStyles;

    //per-port data structs
    size_t _curveCount;
    std::map<size_t, std::map<size_t, std::shared_ptr<QwtPlotCurve>>> _curves;
    std::map<size_t, std::vector<std::shared_ptr<QwtPlotMarker>>> _markers;
    std::map<size_t, std::shared_ptr<std::atomic<size_t>>> _queueDepth;
};
