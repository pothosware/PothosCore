// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <QColor>
#include <qwt_global.h>
#include <qwt_plot.h>

//! Get a color for a plotter curve given an index
QWT_EXPORT QColor getDefaultCurveColor(const size_t whichCurve);

//! make a color have pastel-properties
QWT_EXPORT QColor pastelize(const QColor &c);

/*!
 * A QwtPlot extension class that has slots for certain things.
 */
class QWT_EXPORT MyQwtPlot : public QwtPlot
{
    Q_OBJECT
public:
    MyQwtPlot(QWidget *parent);

public slots:
    void setTitle(const QwtText &text);
    void setAxisTitle(const int id, const QwtText &text);
};
