// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MessageWindow/LoggerChannel.hpp"
#include <Poco/SplitterChannel.h>

LoggerChannel::LoggerChannel(QObject *parent):
    QObject(parent),
    _logger(Poco::Logger::get("")),
    _oldLevel(_logger.getLevel()),
    _oldChannel(_logger.getChannel()),
    _splitter(new Poco::SplitterChannel())
{
    _logger.setLevel("information");
    _logger.setChannel(_splitter);
    _splitter->addChannel(_oldChannel);
    _splitter->addChannel(this);
}

LoggerChannel::~LoggerChannel(void)
{
    return;
}

void LoggerChannel::disconnect(void)
{
    _logger.setLevel(_oldLevel);
    _logger.setChannel(_oldChannel);
}
