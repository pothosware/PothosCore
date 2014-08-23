// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MyPlotPicker.hpp"
#include <qwt_plot.h>
#include <QMouseEvent>

MyPlotPicker::MyPlotPicker(QWidget *parent):
    QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, parent)
{
    return;
};

void MyPlotPicker::widgetMouseDoubleClickEvent(QMouseEvent *event)
{
    emit this->selected(this->invTransform(event->pos()));
}

QwtText MyPlotPicker::trackerTextF(const QPointF &pos) const
{
    QwtText text(QString("<span style='font-size:8pt;' >%1 <b>%2</b><br />%3 <b>%4</b></span>")
        .arg(pos.x())
        .arg(this->plot()->axisTitle(QwtPlot::xBottom).text().toHtmlEscaped())
        .arg(pos.y())
        .arg(this->plot()->axisTitle(QwtPlot::yLeft).text().toHtmlEscaped()));
    static const QColor paleYellow("#FFFFCC");
    text.setBackgroundBrush(QBrush(paleYellow));
    return text;
}
