// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <QGroupBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPainter>
#include <vector>
#include <complex>
#include <iostream>
#include <algorithm> //min/max

/***********************************************************************
 * Draggable crosshairs for point selection
 **********************************************************************/
class PlanarSelectCrossHairs : public QGraphicsObject
{
    Q_OBJECT
public:
    PlanarSelectCrossHairs(void):
        _length(10)
    {
        this->setFlag(QGraphicsItem::ItemIsMovable);
        this->setFlag(QGraphicsItem::ItemIsSelectable);
        this->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
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

signals:
    void positionChanged(void);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value)
    {
        if (change == ItemPositionChange) emit this->positionChanged();
        return QGraphicsObject::itemChange(change, value);
    }

private:
    qreal _length;
};

/***********************************************************************
 * Custom scene with axis background
 **********************************************************************/
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

/***********************************************************************
 * Custom view with scene resize
 **********************************************************************/
class PlanarSelectGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    PlanarSelectGraphicsView(QWidget *parent):
        QGraphicsView(parent),
        _crossHairs(new PlanarSelectCrossHairs())
    {
        this->setScene(new PlanarSelectGraphicsScene(this));
        this->scene()->setBackgroundBrush(Qt::white);
        this->scene()->addItem(_crossHairs);

        //set high quality rendering
        this->setRenderHint(QPainter::Antialiasing);
        this->setRenderHint(QPainter::HighQualityAntialiasing);
        this->setRenderHint(QPainter::SmoothPixmapTransform);

        //forward position changed signal
        connect(_crossHairs, SIGNAL(positionChanged(void)), this, SLOT(handleCrossHairsPointChanged(void)));
    }

    QPointF getPosition(void) const
    {
        return this->scenePosToRelPos(_crossHairs->pos());
    }

    void setPosition(const QPointF &rel_)
    {
        //clip to 0.0 -> 1.0 to keep in bounds
        QPointF rel(
            std::max(std::min(rel_.x(), 1.0), 0.0),
            std::max(std::min(rel_.y(), 1.0), 0.0));
        const auto sr = this->scene()->sceneRect();
        const auto p = QPointF(rel.x()*sr.width(), (1.0-rel.y())*sr.height());
        _crossHairs->setPos(p + sr.topLeft());
    }

signals:
    void positionChanged(const QPointF &);

private slots:
    void handleCrossHairsPointChanged(void)
    {
        emit this->positionChanged(this->getPosition());
    }

protected:
    void resizeEvent(QResizeEvent *event)
    {
        QGraphicsView::resizeEvent(event);
        bool oldState = this->blockSignals(true);
        const auto p = this->getPosition();
        this->scene()->setSceneRect(QRectF(QPointF(), event->size()));
        this->setPosition(p);
        this->blockSignals(oldState);
    }

    void mousePressEvent(QMouseEvent *mouseEvent)
    {
        QGraphicsView::mousePressEvent(mouseEvent);
        if (mouseEvent->button() == Qt::LeftButton)
        {
            mouseEvent->accept();
            const auto pos = this->scenePosToRelPos(this->mapToScene(mouseEvent->pos()));
            this->setPosition(pos);
            emit this->positionChanged(pos);
        }
    }

private:

    QPointF scenePosToRelPos(const QPointF &scenePos) const
    {
        const auto sr = this->scene()->sceneRect();
        auto p = scenePos - sr.topLeft();
        return QPointF(p.x()/sr.width(), 1.0-(p.y()/sr.height()));
    }

    PlanarSelectCrossHairs *_crossHairs;
};


/***********************************************************************
 * |PothosDoc Planar Select
 *
 * A two-dimensional point selection widget.
 * The point is changed graphically by dragging a crosshair across a rectangular region.
 * When the crosshair point is changed, the new value is emitted
 * as a two dimensional vector of doubles through the "valueChanged" signal,
 * and as a complex number through the "complexValueChanged" signal.
 *
 * |category /Widgets
 * |keywords 2d plane cartesian complex
 *
 * |param title The name of the value displayed by this widget
 * |default "My Coordinate"
 * |widget StringEntry()
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
 * |setter setTitle(title)
 * |setter setMinimum(minimum)
 * |setter setMaximum(maximum)
 * |setter setValue(value)
 **********************************************************************/
class PlanarSelect : public QGroupBox, public Pothos::Block
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
        this->setStyleSheet("QGroupBox {font-weight: bold;}");
        this->registerCall(this, POTHOS_FCN_TUPLE(PlanarSelect, widget));
        this->registerCall(this, POTHOS_FCN_TUPLE(PlanarSelect, value));
        this->registerCall(this, POTHOS_FCN_TUPLE(PlanarSelect, setTitle));
        this->registerCall(this, POTHOS_FCN_TUPLE(PlanarSelect, setValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(PlanarSelect, setMinimum));
        this->registerCall(this, POTHOS_FCN_TUPLE(PlanarSelect, setMaximum));
        this->registerSignal("valueChanged");
        this->registerSignal("complexValueChanged");

        _layout->addWidget(_view);
        _layout->setContentsMargins(QMargins(0, 0, 0, 0));
        _layout->setSpacing(0);
        connect(_view, SIGNAL(positionChanged(const QPointF &)), this, SLOT(handlePositionChanged(const QPointF &)));
    }

    QWidget *widget(void)
    {
        return this;
    }

    void setTitle(const QString &title)
    {
        QMetaObject::invokeMethod(this, "handleSetTitle", Qt::QueuedConnection, Q_ARG(QString, title));
    }

    std::vector<double> value(void) const
    {
        std::vector<double> vals(2);
        vals[0] = _value.x();
        vals[1] = _value.y();
        return vals;
    }

    std::complex<double> complexValue(void) const
    {
        return std::complex<double>(_value.x(), _value.y());
    }

    void setValue(const std::vector<double> &value)
    {
        if (value.size() != 2) throw Pothos::RangeException("PlanarSelect::setValue()", "value size must be 2");
        _value = QPointF(value[0], value[1]);
        const auto pos = _value - _minimum;
        const auto range = _maximum - _minimum;
        _view->setPosition(QPointF(pos.x()/range.x(), pos.y()/range.y()));
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
        this->emitValuesChanged();
    }

private slots:

    void handlePositionChanged(const QPointF &pos)
    {
        const auto range = _maximum - _minimum;
        _value = QPointF(pos.x()*range.x(), pos.y()*range.y()) + _minimum;
        this->emitValuesChanged();
    }

    void handleSetTitle(const QString &title)
    {
        QGroupBox::setTitle(title);
    }

protected:
    void mousePressEvent(QMouseEvent *event)
    {
        QGroupBox::mousePressEvent(event);
        event->ignore(); //allows for dragging from QGroupBox title
    }

private:

    void emitValuesChanged(void)
    {
        this->callVoid("valueChanged", this->value());
        this->callVoid("complexValueChanged", this->complexValue());
    }

    QPointF _minimum;
    QPointF _maximum;
    QPointF _value;
    PlanarSelectGraphicsView *_view;
    QHBoxLayout *_layout;
};

static Pothos::BlockRegistry registerPlanarSelect(
    "/widgets/planar_select", &PlanarSelect::make);

#include "PlanarSelect.moc"
