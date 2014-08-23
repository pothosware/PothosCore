// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <qwt_plot_picker.h>

/*!
 * Plot picker with custom marker style
 */
class QWT_EXPORT MyPlotPicker : public QwtPlotPicker
{
    Q_OBJECT
public:
    MyPlotPicker(QWidget *parent);

protected:
    QwtText trackerTextF(const QPointF &pos) const;
    void widgetMouseDoubleClickEvent(QMouseEvent *);
};
