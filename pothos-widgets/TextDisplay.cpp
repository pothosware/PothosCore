// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <QLabel>

/***********************************************************************
 * |PothosDoc Text Display
 *
 * The text display widget display's a formatted string.
 * The display value can be set through one of the set*Value() slots.
 * Display values can be strings, floating point, and integers.
 *
 * |category /Widgets
 * |keywords text display label
 *
 * |param title The name of the value displayed by this widget
 * |default "My Text Value"
 *
 * |param formatStr [Formatter] The format string with a %1 to sub in the value.
 * |default "%1"
 * |preview disable
 *
 * |mode graphWidget
 * |factory /widgets/text_display()
 * |setter setTitle(title)
 * |setter setFormatStr(formatStr)
 **********************************************************************/
class TextDisplay : public QLabel, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(void)
    {
        return new TextDisplay();
    }

    TextDisplay(void)
    {
        this->setFormatStr("%1");
        this->registerCall(this, POTHOS_FCN_TUPLE(TextDisplay, widget));
        this->registerCall(this, POTHOS_FCN_TUPLE(TextDisplay, setTitle));
        this->registerCall(this, POTHOS_FCN_TUPLE(TextDisplay, setFormatStr));
        this->registerCall(this, POTHOS_FCN_TUPLE(TextDisplay, setStringValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(TextDisplay, setFloatValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(TextDisplay, setIntValue));
    }

    QWidget *widget(void)
    {
        return this;
    }

    void setTitle(const QString &title)
    {
        _title = title;
        this->update();
    }

    void setFormatStr(const QString &formatStr)
    {
        if (not formatStr.contains("%1")) throw Pothos::Exception("Format string missing %1");
        _formatStr = formatStr;
        this->update();
    }

    void setStringValue(const QString &value)
    {
        _valueStr = _formatStr.arg(value.toHtmlEscaped());
        this->update();
    }

    void setFloatValue(const double value)
    {
        _valueStr = _formatStr.arg(value);
        this->update();
    }

    void setIntValue(const int value)
    {
        _valueStr = _formatStr.arg(value);
        this->update();
    }

private:

    void update(void)
    {
        const auto text = QString("<b>%1:</b> %2").arg(_title.toHtmlEscaped()).arg(_valueStr.toHtmlEscaped());
        QMetaObject::invokeMethod(this, "setText", Qt::QueuedConnection, Q_ARG(QString, text));
    }

    QString _title;
    QString _valueStr;
    QString _formatStr;
};

static Pothos::BlockRegistry registerTextDisplay(
    "/widgets/text_display", &TextDisplay::make);

#include "TextDisplay.moc"
