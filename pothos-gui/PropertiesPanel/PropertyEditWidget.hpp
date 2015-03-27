// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QWidget>
#include <QString>
#include <QLabel>
#include <Poco/JSON/Object.h>
#include <string>

class QLabel;

/*!
 * The property edit widget creates an entry widget through a JSON description.
 * Possible edit widgets are arbitrary and found in the plugin registry.
 * This is a wrapper around an edit widget that handles evaluation results.
 */
class PropertyEditWidget : public QWidget
{
    Q_OBJECT
public:

    /*!
     * Make a new edit widget from a JSON description.
     * The initial value allows the widget to determine if a change occurred.
     */
    PropertyEditWidget(const QString &initialValue, const Poco::JSON::Object::Ptr &paramDesc, QWidget *parent);

    //! Get the initial value of the edit widget
    const QString &initialValue(void) const;

    //! Does this edit widget have a change vs the original value?
    bool changed(void) const;

    //! get the value of the edit widget
    QString value(void) const;

    //! set the value of the edit widget
    void setValue(const QString &value);

    //! Set the type string from an evaluation
    void setTypeStr(const std::string &typeStr);

    //! Set the error message from an evaluation
    void setErrorMsg(const QString &errorMsg);

    /*!
     * Make a label that will track the widget's status.
     * This label might be used in a QFormLayout for example.
     * Label ownership goes to the caller.
     */
    QLabel *makeFormLabel(const QString &text, QWidget *parent);

signals:
    void commitRequested(void);
    void widgetChanged(void);
    void entryChanged(void);

private slots:
    void handleInternalChange(void);

private:
    const QString _initialValue;
    QWidget *_editWidget;
    QLabel *_errorLabel;
    QLabel *_formLabel;
    QString _formLabelText;
    QString _errorMsg;
    const QString _unitsStr;
};
