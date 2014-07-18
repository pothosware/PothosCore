// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <QScrollArea>
#include <Poco/Message.h>

class LoggerChannel;
class QTextEdit;

class LoggerDisplay : public QScrollArea
{
    Q_OBJECT
public:
    LoggerDisplay(QWidget *parent);

private slots:
    void handleLogMessage(const Poco::Message &msg);

private:
    LoggerChannel *_channel;
    QTextEdit *_text;
};
