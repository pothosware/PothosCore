// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QFrame>
#include <QPointer>
#include <QStaticText>

class QVBoxLayout;

/*!
 * A container for a graph widget with resize grip and label.
 */
class GraphWidgetContainer : public QFrame
{
    Q_OBJECT
public:
    GraphWidgetContainer(QWidget *parent = nullptr);

    ~GraphWidgetContainer(void);

    /*!
     * Set the internal widget.
     * This container does not take ownership.
     */
    void setWidget(QWidget *widget);

    //! get container widget
    QWidget *widget(void) const
    {
        return _widget;
    }

    //! set a label for the resize grip
    void setGripLabel(const QString &name);

    //! set the selection state -- drawing highlight border
    void setSelected(const bool selected);

signals:
    //! emit when the resize operation completes
    void resized(void);

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void updateShowGrip(void);
    void paintEvent(QPaintEvent *event);

private:
    QStaticText _gripLabel;
    QVBoxLayout *_layout;
    QWidget *_grip;
    QPointer<QWidget> _widget;
    bool _selected;
};
