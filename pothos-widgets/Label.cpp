// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <QLabel>

/***********************************************************************
 * |PothosDoc Label
 *
 * The Label widget display's a formatted string.
 * The display value can be set through one of the set*Value() slots.
 * Display values can be strings, floating point, and integers.
 *
 * |category /Widgets
 * |keywords label
 *
 * |param formatStr [Formatter] The format string with a %1 to sub in the value.
 * |default "Value: %1"
 * |preview disable
 *
 * |mode graphWidget
 * |factory /widgets/label()
 * |setter setFormatStr(formatStr)
 **********************************************************************/
class Label : public QLabel, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(void)
    {
        return new Label();
    }

    Label(void)
    {
        this->setFormatStr("Value: %1");
        this->registerCall(this, POTHOS_FCN_TUPLE(Label, widget));
        this->registerCall(this, POTHOS_FCN_TUPLE(Label, setFormatStr));
        this->registerCall(this, POTHOS_FCN_TUPLE(Label, setStringValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(Label, setFloatValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(Label, setIntValue));
    }

    QWidget *widget(void)
    {
        return this;
    }

    void setFormatStr(const QString &formatStr)
    {
        if (not formatStr.contains("%1")) throw Pothos::Exception("Label format string missing %1");
        _formatStr = formatStr;
        this->setText(_formatStr);
    }

    void setStringValue(const QString &value)
    {
        this->setText(_formatStr.arg(value));
    }

    void setFloatValue(const double value)
    {
        this->setText(_formatStr.arg(value));
    }

    void setIntValue(const int value)
    {
        this->setText(_formatStr.arg(value));
    }

private:
    QString _formatStr;
};

static Pothos::BlockRegistry registerAdd(
    "/widgets/label", &Label::make);

#include "Label.moc"
