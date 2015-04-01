// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MyPlotPicker.hpp"
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_raster_data.h>
#include <QMouseEvent>
#include <cassert>
#include <algorithm> //min/max

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

bool MyPlotPicker::accept(QPolygon &pa) const
{
    if (pa.count() < 2) return false;

    auto rect = QRect(pa[0], pa[int(pa.count()) - 1]).normalized();
    const auto canvas = dynamic_cast<const QwtPlotCanvas *>(this->plot()->canvas());
    assert(canvas != nullptr);
    const auto size = canvas->frameRect().size();
    const auto pad = canvas->frameWidth();

    rect.setTopLeft(QPoint(
        std::max(pad, rect.topLeft().x()),
        std::max(pad, rect.topLeft().y())));

    rect.setBottomRight(QPoint(
        std::min(size.width()-pad-1, rect.bottomRight().x()),
        std::min(size.height()-pad-1, rect.bottomRight().y())));

    pa.resize(2);
    pa[0] = rect.topLeft();
    pa[1] = rect.bottomRight();

    return QwtPlotZoomer::accept(pa);
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
