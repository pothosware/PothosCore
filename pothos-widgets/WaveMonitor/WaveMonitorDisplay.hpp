// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <qwt_math.h> //_USE_MATH_DEFINES
#include <Pothos/Framework.hpp>
#include <QWidget>
#include <memory>
#include <valarray>
#include <map>
#include <vector>

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

    void setNumInputs(const size_t numInputs);

    //! set the plotter's title
    void setTitle(const QString &title);

    /*!
     * sample rate for the plotter
     * controls the time scaling display
     */
    void setSampleRate(const double sampleRate);

    void setNumPoints(const size_t numPoints);

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
    void handleSamples(const int index, const int curve, const std::valarray<float> &samps, const std::vector<Pothos::Label> &labels);
    void handleUpdateAxis(void);
    void handleZoomed(const QRectF &rect);

private:
    MyQwtPlot *_mainPlot;
    QwtPlotGrid *_plotGrid;
    QwtPlotZoomer *_zoomer;
    double _sampleRate;
    double _sampleRateWoAxisUnits;
    size_t _numPoints;
    std::string _rateLabelId;

    //per-port data structs
    std::map<size_t, std::map<size_t, std::shared_ptr<QwtPlotCurve>>> _curves;
    std::map<size_t, std::vector<std::shared_ptr<QwtPlotMarker>>> _markers;
    size_t _nextColorIndex;
};
