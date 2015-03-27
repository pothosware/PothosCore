// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QWidget>
#include <QString>
#include <QLabel>
#include <QTimer>
#include <Poco/JSON/Object.h>
#include <string>

class QLabel;
class QTimer;

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

    //! Set the background color of the edit widget
    void setBackgroundColor(const QColor color);

    /*!
     * Make a label that will track the widget's status.
     * This label might be used in a QFormLayout for example.
     * Label ownership goes to the caller.
     */
    QLabel *makeFormLabel(const QString &text, QWidget *parent);

    //! Cancel user-entry events - cancel pending timer signals
    void cancelEvents(void);

    //! Flush user-entry events - triggers pending timer signals
    void flushEvents(void);

signals:

    //! user pressed enter in an entry box -- force properties commit
    void commitRequested(void);

    //! any kind of immediate widget change other than text entry
    void widgetChanged(void);

    //! a text entry event from the user occurred
    void entryChanged(void);

private slots:
    void handleWidgetChanged(void);
    void handleEntryChanged(void);
    void handleCommitRequested(void);

private:
    void updateInternals(void);
    const QString _initialValue;
    QWidget *_editWidget;
    QLabel *_errorLabel;
    QLabel *_formLabel;
    QString _formLabelText;
    QString _errorMsg;
    const QString _unitsStr;
    QTimer *_entryTimer;
};
