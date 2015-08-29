// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "PlotUtilsConfig.hpp"
#include <QColor>
#include <qwt_plot.h>

class QwtPlotItem;

//! Get a color for a plotter curve given an index
POTHOS_PLOTTER_UTILS_EXPORT QColor getDefaultCurveColor(const size_t whichCurve);

//! make a color have pastel-properties
POTHOS_PLOTTER_UTILS_EXPORT QColor pastelize(const QColor &c);

/*!
 * A QwtPlot extension class that has slots for certain things.
 */
class POTHOS_PLOTTER_UTILS_EXPORT MyQwtPlot : public QwtPlot
{
    Q_OBJECT
public:
    MyQwtPlot(QWidget *parent);

    //! update checked status based on visibility
    void updateChecked(QwtPlotItem *item);

public slots:
    void setTitle(const QwtText &text);
    void setAxisTitle(const int id, const QwtText &text);
};
