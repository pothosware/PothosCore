// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QStackedWidget>
#include <Poco/JSON/Object.h>

//! A string-based edit widget to abstract the different types of editors
class BlockPropertyEditWidget : public QStackedWidget
{
    Q_OBJECT
public:
    BlockPropertyEditWidget(const Poco::JSON::Object::Ptr &paramDesc, QWidget *parent);

    //! Set the value of the edit widget
    void setValue(const QString &value);

    //! Get the current value
    QString value(void) const;

    //! Set the fg and bg colors based on the data type
    void setColors(const std::string &typeStr);

signals:

    //! The commit action was triggered
    void commitRequested(void);

    //! Emitted when a widget button or the like changes
    void widgetChanged(void);

    //! Emitted when a text-like editor changes value
    void entryChanged(void);

private slots:
    void handleEntryChanged(const QString &)
    {
        emit this->entryChanged();
    }
    void handleEntryChanged(void)
    {
        emit this->entryChanged();
    }
    void handleWidgetChanged(const QString &)
    {
        emit this->widgetChanged();
    }
    void handleWidgetChanged(void)
    {
        emit this->widgetChanged();
    }

private:
    QWidget *_edit;
};
