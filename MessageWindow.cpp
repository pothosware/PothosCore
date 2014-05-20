// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include <QTabWidget>
#include <QTextEdit>
#include <QScrollArea>
#include <Poco/Channel.h>
#include <Poco/Logger.h>
#include <Poco/Message.h>
#include <Poco/DateTimeFormatter.h>
#include <iostream>

class MessageWindowChannel : public QObject, public Poco::Channel
{
    Q_OBJECT
public:
    MessageWindowChannel(QObject *parent):
        QObject(parent)
    {
        //TODO install channel splitter
        Poco::Logger::get("").setLevel("information");
        Poco::Logger::get("").setChannel(this);
    }

    ~MessageWindowChannel(void)
    {
        //TODO uninstall channel splitter
        //also mutex for making channel changes
        Poco::Logger::get("").setChannel(nullptr);
    }

    void log(const Poco::Message &msg)
    {
        emit receivedLogMessage(msg);
    }

signals:
    void receivedLogMessage(const Poco::Message &);
};

class MessageWindowTab : public QScrollArea
{
    Q_OBJECT
public:
    MessageWindowTab(QWidget *parent):
        QScrollArea(parent),
        _channel(new MessageWindowChannel(this)),
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

        poco_information(Poco::Logger::get("Test"), "this is a test info message");
        poco_notice(Poco::Logger::get("Test"), "this is a test notice message");
        poco_warning(Poco::Logger::get("Test"), "this is a test warning message");
        poco_error(Poco::Logger::get("Test"), "this is a test error message");
    }

private slots:
    void handleLogMessage(const Poco::Message &msg)
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

private:
    MessageWindowChannel *_channel;
    QTextEdit *_text;
};

class MessageWindow : public QTabWidget
{
    Q_OBJECT
public:
    MessageWindow(QWidget *parent):
        QTabWidget(parent)
    {
        this->setMovable(true);
        this->setUsesScrollButtons(true);
        this->setTabPosition(QTabWidget::West);
        this->addTab(new MessageWindowTab(this), "");
    }
};

QWidget *makeMessageWindow(QWidget *parent)
{
    return new MessageWindow(parent);
}

#include "MessageWindow.moc"
