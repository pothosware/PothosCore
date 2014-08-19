// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphObjects/GraphWidgetContainer.hpp"
#include "GraphEditor/Constants.hpp"
#include <QSizeGrip>
#include <QVBoxLayout>
#include <QStaticText>
#include <QMouseEvent>
#include <QPainter>

/***********************************************************************
 * A special QSizeGrip with resize completion signal
 **********************************************************************/
class MySizeGrip : public QSizeGrip
{
    Q_OBJECT
public:
    MySizeGrip(QWidget *parent):
        QSizeGrip(parent)
    {
        return;
    }

signals:
    void resized(void);

protected:
    void mousePressEvent(QMouseEvent *event)
    {
        if (event->button() == Qt::LeftButton)
        {
            _pressPos = this->pos();
        }
        QSizeGrip::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event)
    {
        if (event->button() == Qt::LeftButton)
        {
            if (_pressPos != this->pos()) emit this->resized();
        }
        QSizeGrip::mouseReleaseEvent(event);
    }

private:
    QPointF _pressPos;
};

/***********************************************************************
 * GraphWidgetContainer implementation
 **********************************************************************/
GraphWidgetContainer::GraphWidgetContainer(QWidget *parent):
    QFrame(parent),
    _layout(new QVBoxLayout(this)),
    _grip(new MySizeGrip(this)),
    _widget(nullptr)
{
    this->setLayout(_layout);
    _layout->setContentsMargins(QMargins(3, 3, 3, 3));
    _layout->setSpacing(1);
    _layout->addWidget(_grip, 0, Qt::AlignBottom | Qt::AlignRight);
    connect(_grip, SIGNAL(resized(void)), this, SIGNAL(resized(void)));
    _grip->hide();
    this->setSelected(false);
}

GraphWidgetContainer::~GraphWidgetContainer(void)
{
    //remove this container as a parent to widget
    this->setWidget(nullptr);
}

void GraphWidgetContainer::setWidget(QWidget *widget)
{
    //no change, just return
    if (_widget == widget) return;

    //remove old widget, dont delete it
    if (_widget)
    {
        //we dont own the widget, dont delete it
        _layout->removeWidget(_widget);
        _widget->setParent(nullptr);
    }

    //stash new widget and add to layout
    _widget = widget;
    if (_widget) _layout->insertWidget(0, _widget);
    this->setShowGrip(false);
}

void GraphWidgetContainer::setGripLabel(const QString &name)
{
    _gripLabel = QStaticText(QString("<span style='color:%1;font-size:%2'>(%3)</span>")
        .arg(GraphWidgetGripLabelColor)
        .arg(GraphWidgetGripLabelFontSize)
        .arg(name.toHtmlEscaped()));
}

void GraphWidgetContainer::setSelected(const bool selected)
{
    this->setStyleSheet(QString("GraphWidgetContainer {"
        "border-width: %1px;"
        "border-style: solid;"
        "border-radius: %2px;"
        "border-color: %3;"
        "background-color: %4;"
    "}").arg(GraphObjectBorderWidth)
        .arg(GraphBlockMainArc)
        .arg(selected?GraphObjectHighlightPenColor:GraphObjectDefaultPenColor)
        .arg(GraphWidgetBackgroundColor));
}

void GraphWidgetContainer::enterEvent(QEvent *event)
{
    this->setShowGrip(true);
    QWidget::enterEvent(event);
}

void GraphWidgetContainer::leaveEvent(QEvent *event)
{
    this->setShowGrip(false);
    QWidget::leaveEvent(event);
}

void GraphWidgetContainer::setShowGrip(const bool visible)
{
    if (not _widget) return;
    _widget->show(); //needs visibility to calculate size

    //stash the relevant settings
    auto oldPolicy = _widget->sizePolicy();
    auto oldMinSize = _widget->minimumSize();

    //fix the sizes so the widget wont be hurt by adjustSize
    _widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _widget->setMinimumSize(_widget->size());

    //change the visibility and adjust to the new overall size
    _grip->setVisible(visible);
    this->adjustSize();

    //restore settings to the widget
    _widget->setMinimumSize(oldMinSize);
    _widget->setSizePolicy(oldPolicy);
}

void GraphWidgetContainer::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    if (not _widget) return;
    if (not _grip->isVisible()) return;

    QPainter painter(this);
    auto panelH = this->height() - _widget->height();
    painter.drawStaticText(QPointF((this->width()-_gripLabel.size().width())/2.,
        _widget->height()+(panelH-_gripLabel.size().height())/2.), _gripLabel);
    painter.end();
}

#include "GraphWidgetContainer.moc"
