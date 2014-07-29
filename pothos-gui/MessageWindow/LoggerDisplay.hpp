// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QScrollArea>
#include <Poco/Message.h>
#include <Poco/AutoPtr.h>

class LoggerChannel;
class QTextEdit;

class LoggerDisplay : public QScrollArea
{
    Q_OBJECT
public:
    LoggerDisplay(QWidget *parent);
    ~LoggerDisplay(void);

private slots:
    void handleLogMessage(const Poco::Message &msg);

private:
    Poco::AutoPtr<LoggerChannel> _channel;
    QTextEdit *_text;
};
