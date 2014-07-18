// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MessageWindow/LoggerChannel.hpp"
#include <Poco/Logger.h>

LoggerChannel::LoggerChannel(QObject *parent):
    QObject(parent)
{
    //TODO install channel splitter
    Poco::Logger::get("").setLevel("information");
    Poco::Logger::get("").setChannel(this);
}

LoggerChannel::~LoggerChannel(void)
{
    //TODO uninstall channel splitter
    //also mutex for making channel changes
    Poco::Logger::get("").setChannel(nullptr);
}
