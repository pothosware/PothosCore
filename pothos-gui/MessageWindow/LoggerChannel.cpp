// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MessageWindow/LoggerChannel.hpp"
#include <Poco/SplitterChannel.h>

LoggerChannel::LoggerChannel(QObject *parent):
    QObject(parent),
    _splitter(dynamic_cast<Poco::SplitterChannel *>(Poco::Logger::get("").getChannel()))
{
    if (_splitter) _splitter->addChannel(this);
    else poco_error(Poco::Logger::get("PothosGui.LoggerChannel"), "expected SplitterChannel");
}

LoggerChannel::~LoggerChannel(void)
{
    return;
}

void LoggerChannel::disconnect(void)
{
    if (_splitter) _splitter->removeChannel(this);
}
