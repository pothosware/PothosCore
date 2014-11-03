// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QPainter>
#include <vector>
#include <complex>
#include <iostream>

class PlanarSelectCrossHairs : public QGraphicsObject
{
    Q_OBJECT
public:
    PlanarSelectCrossHairs(void):
        _length(10)
    {
        this->setFlag(QGraphicsItem::ItemIsMovable);
        this->setFlag(QGraphicsItem::ItemIsSelectable);
    }

    QRectF boundingRect(void) const
    {
        return QRectF(QPointF(-_length/2, -_length/2), QSizeF(_length, _length));
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
    {
        painter->setPen(Qt::black);
        painter->drawLine(QPointF(0, -_length/2), QPointF(0, _length/2));
        painter->drawLine(QPointF(-_length/2, 0), QPointF(_length/2, 0));
    }

private:
    qreal _length;
};

class PlanarSelectGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    PlanarSelectGraphicsScene(QObject *parent):
        QGraphicsScene(parent)
    {
        return;
    }

    void drawBackground(QPainter *painter, const QRectF &rect)
    {
        QGraphicsScene::drawBackground(painter, rect);

        static const QColor lightGray("#D0D0D0");
        painter->setPen(lightGray);

        qreal x = this->sceneRect().center().x();
        qreal y = this->sceneRect().center().y();
        qreal width = this->sceneRect().width();
        qreal height = this->sceneRect().height();

        //main center lines
        painter->drawLine(QPointF(x, y-height/2), QPointF(x, y+height/2));
        painter->drawLine(QPointF(x-width/2, y), QPointF(x+width/2, y));

        //half-way lines
        qreal length = 5;
        painter->drawLine(QPointF(x-width/4, y-length), QPointF(x-width/4, y+length));
        painter->drawLine(QPointF(x+width/4, y-length), QPointF(x+width/4, y+length));
        painter->drawLine(QPointF(x-length, y-height/4), QPointF(x+length, y-height/4));
        painter->drawLine(QPointF(x-length, y+height/4), QPointF(x+length, y+height/4));
    }
};

class PlanarSelectGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    PlanarSelectGraphicsView(QWidget *parent):
        QGraphicsView(parent)
    {
        this->setScene(new PlanarSelectGraphicsScene(this));
        this->scene()->setBackgroundBrush(Qt::white);

        auto crossHairs = new PlanarSelectCrossHairs();
        this->scene()->addItem(crossHairs);

        //set high quality rendering
        this->setRenderHint(QPainter::Antialiasing);
        this->setRenderHint(QPainter::HighQualityAntialiasing);
        this->setRenderHint(QPainter::SmoothPixmapTransform);
    }

protected:
    void resizeEvent(QResizeEvent *event)
    {
        QGraphicsView::resizeEvent(event);
        this->scene()->setSceneRect(QRectF(QPointF(), event->size()));
    }
};


/***********************************************************************
 * |PothosDoc Planar Select
 *
 * A two-dimensional point selection widget.
 *
 * |category /Widgets
 * |keywords 2d plane cartesian complex
 *
 * |param value The initial value of this slider.
 * |default [0.0, 0.0]
 *
 * |param minimum The smallest X and Y bounds of the selection.
 * |default [-1.0, -1.0]
 *
 * |param maximum The largest X and Y bounds of the selection.
 * |default [1.0, 1.0]
 *
 * |mode graphWidget
 * |factory /widgets/planar_select()
 * |setter setMinimum(minimum)
 * |setter setMaximum(maximum)
 * |setter setValue(value)
 **********************************************************************/
class PlanarSelect : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(void)
    {
        return new PlanarSelect();
    }

    PlanarSelect():
        _view(new PlanarSelectGraphicsView(this)),
        _layout(new QHBoxLayout(this))
    {
        this->registerCall(this, POTHOS_FCN_TUPLE(PlanarSelect, widget));
        this->registerCall(this, POTHOS_FCN_TUPLE(PlanarSelect, value));
        this->registerCall(this, POTHOS_FCN_TUPLE(PlanarSelect, setValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(PlanarSelect, setMinimum));
        this->registerCall(this, POTHOS_FCN_TUPLE(PlanarSelect, setMaximum));
        this->registerSignal("valueChanged");
        this->registerSignal("complexValueChanged");

        _layout->addWidget(_view);
    }

    QWidget *widget(void)
    {
        return this;
    }

    std::vector<double> value(void) const
    {
        
    }

    std::complex<double> complexValue(void) const
    {
        const auto val = this->value();
        return std::complex<double>(val[0], val[1]);
    }

    void setValue(const std::vector<double> &value)
    {
        if (value.size() != 2) throw Pothos::RangeException("PlanarSelect::setValue()", "value size must be 2");
    }

    void setMinimum(const std::vector<double> &minimum)
    {
        if (minimum.size() != 2) throw Pothos::RangeException("PlanarSelect::setMinimum()", "minimum size must be 2");
        _minimum = QPointF(minimum[0], minimum[1]);
    }

    void setMaximum(const std::vector<double> &maximum)
    {
        if (maximum.size() != 2) throw Pothos::RangeException("PlanarSelect::setMaximum()", "maximum size must be 2");
        _maximum = QPointF(maximum[0], maximum[1]);
    }

    void activate(void)
    {
        //emit current value when design becomes active
        this->callVoid("valueChanged", this->value());
        this->callVoid("complexValueChanged", this->complexValue());
    }

private:
    QPointF _minimum;
    QPointF _maximum;
    QGraphicsView *_view;
    QHBoxLayout *_layout;
};

static Pothos::BlockRegistry registerPlanarSelect(
    "/widgets/planar_select", &PlanarSelect::make);

#include "PlanarSelect.moc"
