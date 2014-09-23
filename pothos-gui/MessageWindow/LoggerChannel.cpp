// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MessageWindow/LoggerChannel.hpp"
#include <Poco/SplitterChannel.h>

LoggerChannel::LoggerChannel(QObject *parent):
    QObject(parent),
    _logger(Poco::Logger::get("")),
    _oldLevel(_logger.getLevel()),
    _splitter(dynamic_cast<Poco::SplitterChannel *>(_logger.getChannel()))
{
    _logger.setLevel("trace"); //lowest level -> shows everything
    if (_splitter) _splitter->addChannel(this);
    else poco_error(Poco::Logger::get("PothosGui.LoggerChannel"), "expected SplitterChannel");
}

LoggerChannel::~LoggerChannel(void)
{
    return;
}

void LoggerChannel::disconnect(void)
{
    _logger.setLevel(_oldLevel);
    if (_splitter) _splitter->removeChannel(this);
}
