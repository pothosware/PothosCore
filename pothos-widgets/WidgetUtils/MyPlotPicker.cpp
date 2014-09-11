// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MyPlotPicker.hpp"
#include <qwt_plot.h>
#include <qwt_raster_data.h>
#include <QMouseEvent>

MyPlotPicker::MyPlotPicker(QWidget *parent):
    QwtPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, parent),
    _raster(nullptr)
{
    this->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    this->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
    this->setTrackerMode(QwtPicker::AlwaysOn);
};

void MyPlotPicker::registerRaster(QwtRasterData *raster)
{
    _raster = raster;
}

void MyPlotPicker::widgetMouseDoubleClickEvent(QMouseEvent *event)
{
    emit this->selected(this->invTransform(event->pos()));
}

QwtText MyPlotPicker::trackerTextF(const QPointF &pos) const
{
    QString zvalue;
    if (_raster != nullptr and
        this->plot()->axisInterval(this->xAxis()).contains(pos.x()) and
        this->plot()->axisInterval(this->yAxis()).contains(pos.y())
    ) zvalue = QString("<br />%1 <b>%2</b>")
        .arg(_raster->value(pos.x(), pos.y()))
        .arg(this->plot()->axisTitle(QwtPlot::yRight).text().toHtmlEscaped());

    QwtText text(QString("<span style='font-size:7pt;' >%1 <b>%2</b><br />%3 <b>%4</b>%5</span>")
        .arg(pos.x())
        .arg(this->plot()->axisTitle(this->xAxis()).text().toHtmlEscaped())
        .arg(pos.y())
        .arg(this->plot()->axisTitle(this->yAxis()).text().toHtmlEscaped())
        .arg(zvalue));

    static const QColor paleYellow("#FFFFCC");
    text.setBackgroundBrush(QBrush(paleYellow));
    return text;
}
