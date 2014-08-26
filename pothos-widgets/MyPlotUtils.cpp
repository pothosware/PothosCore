// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MyPlotUtils.hpp"
#include <QList>
#include <valarray>
#include <qwt_legend_data.h>
#include <qwt_text.h>

QColor getDefaultCurveColor(const size_t whichCurve)
{
    switch(whichCurve % 12)
    {
    case 0: return Qt::blue;
    case 1: return Qt::green;
    case 2: return Qt::red;
    case 3: return Qt::cyan;
    case 4: return Qt::magenta;
    case 5: return Qt::yellow;
    case 6: return Qt::darkBlue;
    case 7: return Qt::darkGreen;
    case 8: return Qt::darkRed;
    case 9: return Qt::darkCyan;
    case 10: return Qt::darkMagenta;
    case 11: return Qt::darkYellow;
    };
    return QColor();
}

QColor pastelize(const QColor &c)
{
    //Pastels have high value and low to intermediate saturation:
    //http://en.wikipedia.org/wiki/Pastel_%28color%29
    return QColor::fromHsv(c.hue(), int(c.saturationF()*128), int(c.valueF()*64)+191);
}

MyQwtPlot::MyQwtPlot(QWidget *parent):
    QwtPlot(parent)
{
    qRegisterMetaType<QList<QwtLegendData>>("QList<QwtLegendData>"); //missing from qwt
    qRegisterMetaType<std::valarray<float>>("std::valarray<float>"); //used for plot data
    qRegisterMetaType<QwtText>("QwtText"); //used in this class's public slots
}
