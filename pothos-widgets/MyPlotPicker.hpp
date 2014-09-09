// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <qwt_plot_zoomer.h>

class QwtRasterData;

/*!
 * Plot picker with custom marker style
 */
class QWT_EXPORT MyPlotPicker : public QwtPlotZoomer
{
    Q_OBJECT
public:
    MyPlotPicker(QWidget *parent);

    //! support for getting a z-axis value
    void registerRaster(QwtRasterData *raster);

protected:
    QwtText trackerTextF(const QPointF &pos) const;
    void widgetMouseDoubleClickEvent(QMouseEvent *);

private:
    QwtRasterData *_raster;
};
