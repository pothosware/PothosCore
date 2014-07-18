// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <QObject>
#include <Poco/Channel.h>
#include <Poco/Message.h>

class LoggerChannel : public QObject, public Poco::Channel
{
    Q_OBJECT
public:
    LoggerChannel(QObject *parent);

    ~LoggerChannel(void);

    void log(const Poco::Message &msg)
    {
        emit receivedLogMessage(msg);
    }

signals:
    void receivedLogMessage(const Poco::Message &);
};
