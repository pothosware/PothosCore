// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QStackedWidget>
#include <Poco/Message.h>
#include <Poco/AutoPtr.h>

class LoggerChannel;
class QPlainTextEdit;
class QToolButton;
class QTimer;

class LoggerDisplay : public QStackedWidget
{
    Q_OBJECT
public:
    LoggerDisplay(QWidget *parent);
    ~LoggerDisplay(void);

private slots:
    void handleCheckMsgs(void);

private:
    void handleLogMessage(const Poco::Message &msg);

    Poco::AutoPtr<LoggerChannel> _channel;
    QPlainTextEdit *_text;
    QToolButton *_clearButton;
    QTimer *_timer;

    void resizeEvent(QResizeEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
};
