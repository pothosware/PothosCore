// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MyPlotUtils.hpp"
#include <QList>
#include <valarray>
#include <qwt_legend_data.h>
#include <qwt_plot_canvas.h>
#include <qwt_legend.h>
#include <qwt_legend_label.h>
#include <qwt_text.h>
#include <QMouseEvent>

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

/***********************************************************************
 * Custom QwtPlotCanvas that accepts the mousePressEvent
 * (needed to make mouse-based events work within QGraphicsItem)
 **********************************************************************/
class MyQwtPlotCanvas : public QwtPlotCanvas
{
    Q_OBJECT
public:
    MyQwtPlotCanvas(QwtPlot *parent):
        QwtPlotCanvas(parent)
    {
        return;
    }
protected:
    void mousePressEvent(QMouseEvent *event)
    {
        QwtPlotCanvas::mousePressEvent(event);
        event->accept();
    }
};

/***********************************************************************
 * Custom QwtPlot implementation
 **********************************************************************/
MyQwtPlot::MyQwtPlot(QWidget *parent):
    QwtPlot(parent)
{
    this->setCanvas(new MyQwtPlotCanvas(this));
    qRegisterMetaType<QList<QwtLegendData>>("QList<QwtLegendData>"); //missing from qwt
    qRegisterMetaType<std::valarray<float>>("std::valarray<float>"); //used for plot data
    qRegisterMetaType<QwtText>("QwtText"); //used in this class's public slots
}

void MyQwtPlot::setTitle(const QwtText &text)
{
    QwtPlot::setTitle(text);
}

void MyQwtPlot::setAxisTitle(const int id, const QwtText &text)
{
    QwtPlot::setAxisTitle(id, text);
}

void MyQwtPlot::updateChecked(QwtPlotItem *item)
{
    auto legend = dynamic_cast<QwtLegend *>(this->legend());
    if (legend == nullptr) return; //no legend
    auto info = legend->legendWidget(this->itemToInfo(item));
    auto label = dynamic_cast<QwtLegendLabel *>(info);
    if (label == nullptr) return; //no label
    label->setChecked(item->isVisible());
    this->updateLegend();
}

#include "MyPlotUtils.moc"
