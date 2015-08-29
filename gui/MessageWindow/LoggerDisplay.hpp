// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QStackedWidget>
#include <Poco/Message.h>
#include <Poco/AutoPtr.h>

class LoggerChannel;
class QTextEdit;
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
    QTextEdit *_text;
    QTimer *_timer;
};
