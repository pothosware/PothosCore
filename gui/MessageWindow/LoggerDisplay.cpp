// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //makeIconFromTheme
#include "MessageWindow/LoggerDisplay.hpp"
#include "MessageWindow/LoggerChannel.hpp"
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QToolButton>
#include <QTimer>
#include <Poco/DateTimeFormatter.h>
#include <iostream>

static const long CHECK_MSGS_TIMEOUT_MS = 100;
static const size_t MAX_MSGS_PER_TIMEOUT = 3;
static const size_t MAX_HISTORY_MSGS = 4096;

LoggerDisplay::LoggerDisplay(QWidget *parent):
    QStackedWidget(parent),
    _channel(new LoggerChannel(nullptr)),
    _text(new QPlainTextEdit(this)),
    _clearButton(new QToolButton(_text)),
    _timer(new QTimer(this))
{
    this->addWidget(_text);
    _text->setReadOnly(true);
    _text->setMaximumBlockCount(MAX_HISTORY_MSGS);
    _text->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    _clearButton->hide();
    _clearButton->setIcon(makeIconFromTheme("edit-clear-list"));
    _clearButton->setToolTip(tr("Clear message history"));
    connect(_clearButton, SIGNAL(clicked(void)), _text, SLOT(clear(void)));

    connect(_timer, SIGNAL(timeout(void)), this, SLOT(handleCheckMsgs(void)));
    _timer->start(CHECK_MSGS_TIMEOUT_MS);
}

LoggerDisplay::~LoggerDisplay(void)
{
    _channel->disconnect();
}

void LoggerDisplay::handleCheckMsgs(void)
{
    const bool autoScroll = _text->verticalScrollBar()->value()+50 > _text->verticalScrollBar()->maximum();

    size_t numMsgs = 0;
    Poco::Message msg;
    while (_channel->pop(msg))
    {
        this->handleLogMessage(msg);
        if (++numMsgs == MAX_MSGS_PER_TIMEOUT) break;
    }

    if (numMsgs != 0 and autoScroll)
    {
        auto c =  _text->textCursor();
        c.movePosition(QTextCursor::End);
        _text->setTextCursor(c);
    }
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

    auto line = QString("<font color=\"%1\"><b>[%2] %3:</b></font> %4").arg(
        color, QString::fromStdString(timeStr),
        QString::fromStdString(msg.getSource()),
        body);

    _text->appendHtml(line);
}

void LoggerDisplay::resizeEvent(QResizeEvent *event)
{
    _clearButton->move(_text->viewport()->width()-_clearButton->width(), 0);
    return QStackedWidget::resizeEvent(event);
}

void LoggerDisplay::enterEvent(QEvent *event)
{
    _clearButton->show();
    _clearButton->move(_text->viewport()->width()-_clearButton->width(), 0);
    return QStackedWidget::enterEvent(event);
}

void LoggerDisplay::leaveEvent(QEvent *event)
{
    _clearButton->hide();
    return QStackedWidget::leaveEvent(event);
}
