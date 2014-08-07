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

    //! Emitted when the editor changes value
    void valueChanged(void);

private slots:
    void handleEditWidgetChanged(const QString &);
    void handleEditWidgetChanged(void);

private:
    QWidget *_edit;
};
