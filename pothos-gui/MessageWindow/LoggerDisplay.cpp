// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MessageWindow/LoggerDisplay.hpp"
#include "MessageWindow/LoggerChannel.hpp"
#include <QTextEdit>
#include <QScrollBar>
#include <Poco/DateTimeFormatter.h>
#include <iostream>

LoggerDisplay::LoggerDisplay(QWidget *parent):
    QStackedWidget(parent),
    _channel(new LoggerChannel(nullptr)),
    _text(new QTextEdit(this))
{
    this->addWidget(_text);
    _text->setReadOnly(true);

    qRegisterMetaType<Poco::Message>("Poco::Message");
    connect(
        _channel, SIGNAL(receivedLogMessage(const Poco::Message &)),
        this, SLOT(handleLogMessage(const Poco::Message &)),
        Qt::QueuedConnection);
}

LoggerDisplay::~LoggerDisplay(void)
{
    _channel->disconnect();
}

void LoggerDisplay::handleLogMessage(const Poco::Message &msg)
{
    QString color;
    switch (msg.getPriority())
    {
    case Poco::Message::PRIO_INFORMATION: color = "black"; break;
    case Poco::Message::PRIO_NOTICE: color = "green"; break;
    case Poco::Message::PRIO_WARNING: color = "orange"; break;
    case Poco::Message::PRIO_ERROR: color = "red"; break;
    case Poco::Message::PRIO_CRITICAL: color = "red"; break;
    case Poco::Message::PRIO_FATAL: color = "red"; break;
    default: color = "black";
    }

    const auto timeStr = Poco::DateTimeFormatter::format(msg.getTime(), "%H:%M:%s");

    auto body = QString::fromStdString(msg.getText()).toHtmlEscaped();
    if (body.count("\n") > 1) body = "<pre>"+body+"</pre>";

    auto line = QString("<font color=\"%1\"><b>[%2] %3:</b></font> %4<br />").arg(
        color, QString::fromStdString(timeStr),
        QString::fromStdString(msg.getSource()),
        body);

    const bool autoScroll = _text->verticalScrollBar()->value()+50 > _text->verticalScrollBar()->maximum();

    _text->insertHtml(line);

    if (autoScroll)
    {
        auto c =  _text->textCursor();
        c.movePosition(QTextCursor::End);
        _text->setTextCursor(c);
    }
}
