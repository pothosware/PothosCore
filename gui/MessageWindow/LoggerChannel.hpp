// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QObject>
#include <Poco/Channel.h>
#include <Poco/Message.h>
#include <Poco/Logger.h>
#include <Poco/AutoPtr.h>
#include <queue>
#include <mutex>

namespace Poco
{
    class SplitterChannel;
}

class LoggerChannel : public QObject, public Poco::Channel
{
    Q_OBJECT
public:
    LoggerChannel(QObject *parent);

    ~LoggerChannel(void);

    void disconnect(void);

    void log(const Poco::Message &msg);

    bool pop(Poco::Message &msg);

private:
    Poco::Logger &_logger;
    const int _oldLevel;
    Poco::AutoPtr<Poco::SplitterChannel> _splitter;
    std::queue<Poco::Message> _queue;
    std::mutex _mutex;
};
