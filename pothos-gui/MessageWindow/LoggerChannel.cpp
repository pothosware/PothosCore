// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MessageWindow/LoggerChannel.hpp"
#include <Poco/SplitterChannel.h>

static const long QUEUE_MAX_DEPTH = 30;

LoggerChannel::LoggerChannel(QObject *parent):
    QObject(parent),
    _logger(Poco::Logger::get("")),
    _oldLevel(_logger.getLevel()),
    _splitter(dynamic_cast<Poco::SplitterChannel *>(_logger.getChannel()))
{
    _logger.setLevel(Poco::Message::PRIO_TRACE); //lowest level -> shows everything
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

void LoggerChannel::log(const Poco::Message &msg)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _queue.push(msg);

    //remove oldest if we exceed the max depth
    if (_queue.size() > QUEUE_MAX_DEPTH) _queue.pop();
}

bool LoggerChannel::pop(Poco::Message &msg)
{
    std::unique_lock<std::mutex> lock(_mutex);
    if (_queue.empty()) return false;
    msg = _queue.front();
    _queue.pop();
    return true;
}
