// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QScrollBar>
#include <iostream>

/***********************************************************************
 * |PothosDoc Chat Box
 *
 * The chat box is an instant-messaging style widget
 * with a line entry for outgoing messages,
 * and a text area for the incoming messages.
 *
 * The chat box has an input port for accepting packet messages,
 * and an output port for producing packet messages from the user.
 * The message packets contain a buffer formatted as a UTF8 string.
 *
 * The user's text from the entry box will be echoed to the chat window
 * and a message will be posted to the output port as a packet message.
 * Messages arriving at the input port will be converted to a string
 * and displayed to the chat window.
 *
 * |category /Widgets
 * |keywords message chat IM
 *
 * |param username An identifying display name for this user's messages.
 * |widget StringEntry()
 * |default "User0"
 *
 * |param color An identifying display color for this user's messages.
 * |widget ComboBox(editable=true)
 * |option [Blue] "blue"
 * |option [Red] "red"
 * |option [Green] "green"
 * |option [Orange] "orange"
 * |option [Purple] "purple"
 * |option [Cyan] "cyan"
 * |option [Yellow] "yellow"
 * |default "blue"
 *
 * |mode graphWidget
 * |factory /widgets/chat_box()
 * |setter setUsername(username)
 * |setter setColor(color)
 **********************************************************************/
class ChatBox : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(void)
    {
        return new ChatBox();
    }

    ChatBox(void):
        _chatDisplay(new QTextEdit(this)),
        _chatEntry(new QLineEdit(this)),
        _userLabel(new QLabel(this))
    {
        //layout
        auto vlayout = new QVBoxLayout(this);
        auto hlayout = new QHBoxLayout();
        vlayout->addWidget(_chatDisplay);
        vlayout->addLayout(hlayout);
        hlayout->addWidget(_userLabel);
        hlayout->addWidget(_chatEntry);

        _chatDisplay->setReadOnly(true);

        //widget signals
        connect(_chatEntry, SIGNAL(returnPressed(void)), this, SLOT(handleEntry(void)));

        //calls
        this->registerCall(this, POTHOS_FCN_TUPLE(ChatBox, widget));
        this->registerCall(this, POTHOS_FCN_TUPLE(ChatBox, setUsername));
        this->registerCall(this, POTHOS_FCN_TUPLE(ChatBox, setColor));

        //ports
        this->setupInput("in");
        this->setupOutput("out");
    }

    QWidget *widget(void)
    {
        return this;
    }

    void setUsername(const QString &name)
    {
        _username = name;
        const auto labelText = QString("<b>%1</b>").arg(_username.toHtmlEscaped());
        QMetaObject::invokeMethod(_userLabel, "setText", Qt::QueuedConnection, Q_ARG(QString, labelText));
    }

    void setColor(const QString &color)
    {
        _color = color;
    }

    void work(void)
    {
        auto inPort = this->input("in");
        if (inPort->hasMessage())
        {
            auto msg = inPort->popMessage();
            auto pkt = msg.extract<Pothos::Packet>();
            const QByteArray bytes(pkt.payload.as<const char *>(), pkt.payload.length);
            const QString line(bytes);
            QMetaObject::invokeMethod(this, "handleIncomingLine", Qt::QueuedConnection, Q_ARG(QString, line));
        }
    }

private slots:
    void handleEntry(void)
    {
        auto text = _chatEntry->text();
        _chatEntry->setText("");
        const auto line = QString("<font color=\"%1\"><b>%2:</b></font> %3")
            .arg(_color)
            .arg(_username.toHtmlEscaped())
            .arg(text.toHtmlEscaped());

        this->handleIncomingLine(line);
        if (this->isActive())
        {
            const auto bytes = line.toUtf8();
            const auto sharedBuff = std::make_shared<QByteArray>(bytes);
            Pothos::Packet pkt;
            pkt.payload = Pothos::SharedBuffer(size_t(bytes.data()), bytes.size(), sharedBuff);
            auto outPort = this->output("out");
            outPort->postMessage(pkt);
        }
    }

    void handleIncomingLine(const QString &line)
    {
        const bool autoScroll = _chatDisplay->verticalScrollBar()->value()+50 > _chatDisplay->verticalScrollBar()->maximum();

        _chatDisplay->insertHtml(line+"<br />");

        if (autoScroll)
        {
            auto c =  _chatDisplay->textCursor();
            c.movePosition(QTextCursor::End);
            _chatDisplay->setTextCursor(c);
        }
    }

private:
    QTextEdit *_chatDisplay;
    QLineEdit *_chatEntry;
    QLabel *_userLabel;
    QString _username;
    QString _color;
};

static Pothos::BlockRegistry registerChatBox(
    "/widgets/chat_box", &ChatBox::make);

#include "ChatBox.moc"
