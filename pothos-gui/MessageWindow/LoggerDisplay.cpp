// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MessageWindow/LoggerDisplay.hpp"
#include "MessageWindow/LoggerChannel.hpp"
#include <QTextEdit>
#include <Poco/DateTimeFormatter.h>

LoggerDisplay::LoggerDisplay(QWidget *parent):
    QScrollArea(parent),
    _channel(new LoggerChannel(this)),
    _text(new QTextEdit(this))
{
    this->setWidgetResizable(true);
    this->setWidget(_text);
    _text->setReadOnly(true);

    qRegisterMetaType<Poco::Message>("Poco::Message");
    connect(
        _channel, SIGNAL(receivedLogMessage(const Poco::Message &)),
        this, SLOT(handleLogMessage(const Poco::Message &)),
        Qt::QueuedConnection);
}

void LoggerDisplay::handleLogMessage(const Poco::Message &msg)
{
    QString color;
    switch (msg.getPriority())
    {
    case Poco::Message::PRIO_TRACE: color = "black"; break;
    case Poco::Message::PRIO_DEBUG: color = "black"; break;
    case Poco::Message::PRIO_INFORMATION: color = "black"; break;
    case Poco::Message::PRIO_NOTICE: color = "green"; break;
    case Poco::Message::PRIO_WARNING: color = "orange"; break;
    case Poco::Message::PRIO_ERROR: color = "red"; break;
    case Poco::Message::PRIO_CRITICAL: color = "black"; break;
    case Poco::Message::PRIO_FATAL: color = "black"; break;
    default: color = "black";
    }

    const auto timeStr = Poco::DateTimeFormatter::format(msg.getTime(), "%H:%M:%s");

    auto line = QString("<font color=\"%1\"><b>[%2] %3:</b></font> %4<br />").arg(
        color, QString::fromStdString(timeStr),
        QString::fromStdString(msg.getSource()),
        QString::fromStdString(msg.getText()).toHtmlEscaped());

    _text->insertHtml(line);
}
