// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Object/Containers.hpp>
#include <QPushButton>
#include <map>

/***********************************************************************
 * |PothosDoc Push Button
 *
 * The push button widget emits a specified value when clicked
 * on the "triggered" signal.
 *
 * |category /Widgets
 * |keywords click button
 *
 * |param title The title text displayed on this widget
 * |default "My Push Button"
 * |widget StringEntry()
 *
 * |param args Arguments to pass into the toggled signal.
 * Example: ["test", 42] - the downstream slot takes two arguments.
 * |default []
 *
 * |mode graphWidget
 * |factory /widgets/push_button()
 * |setter setTitle(title)
 * |setter setArgs(args)
 **********************************************************************/
class PushButton : public QPushButton, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(void)
    {
        return new PushButton();
    }

    PushButton(void):
        QPushButton(nullptr)
    {
        this->registerSignal("triggered");
        this->registerCall(this, POTHOS_FCN_TUPLE(PushButton, widget));
        this->registerCall(this, POTHOS_FCN_TUPLE(PushButton, setTitle));
        this->registerCall(this, POTHOS_FCN_TUPLE(PushButton, setArgs));
        this->registerCall(this, POTHOS_FCN_TUPLE(PushButton, getArgs));
        connect(this, SIGNAL(clicked(void)), this, SLOT(handleClicked(void)));
    }

    QWidget *widget(void)
    {
        return this;
    }

    void setTitle(const QString &title)
    {
        QMetaObject::invokeMethod((QPushButton*)this, "handleSetText", Qt::QueuedConnection, Q_ARG(QString, title));
    }

    void setArgs(const Pothos::ObjectVector &args)
    {
        _args = args;
    }

    Pothos::ObjectVector getArgs(void) const
    {
        return _args;
    }

private slots:

    void handleClicked(void)
    {
        this->opaqueCallMethod("triggered", _args.data(), _args.size());
    }

    void handleSetText(const QString &text)
    {
        this->setText(text);
    }

private:

    Pothos::ObjectVector _args;
};

static Pothos::BlockRegistry registerPushButton(
    "/widgets/push_button", &PushButton::make);

#include "PushButton.moc"
