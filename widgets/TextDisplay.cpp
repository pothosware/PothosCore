// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <QLabel>
#include <complex>

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
 * |widget StringEntry()
 *
 * |param formatStr [Formatter] The format string with a %1 to sub in the value.
 * |default "%1"
 * |widget StringEntry()
 * |preview disable
 *
 * |param base [Base] The base used for integer formatting.
 * |default 10
 * |option [Binary] 2
 * |option [Octal] 8
 * |option [Decimal] 10
 * |option [Hex] 16
 * |preview disable
 * |widget ComboBox(editable=true)
 *
 * |mode graphWidget
 * |factory /widgets/text_display()
 * |setter setTitle(title)
 * |setter setFormatStr(formatStr)
 * |setter setBase(base)
 **********************************************************************/
class TextDisplay : public QLabel, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(void)
    {
        return new TextDisplay();
    }

    TextDisplay(void):
        _base(10)
    {
        this->setFormatStr("%1");
        this->registerCall(this, POTHOS_FCN_TUPLE(TextDisplay, widget));
        this->registerCall(this, POTHOS_FCN_TUPLE(TextDisplay, setTitle));
        this->registerCall(this, POTHOS_FCN_TUPLE(TextDisplay, setFormatStr));
        this->registerCall(this, POTHOS_FCN_TUPLE(TextDisplay, setBase));
        this->registerCall(this, POTHOS_FCN_TUPLE(TextDisplay, setStringValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(TextDisplay, setFloatValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(TextDisplay, setComplexValue));
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

    void setBase(const size_t base)
    {
        _base = base;
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

    void setComplexValue(const std::complex<double> value)
    {
        _valueStr = QString("%1+%2j").arg(_formatStr.arg(value.real())).arg(_formatStr.arg(value.imag()));
        this->update();
    }

    void setIntValue(const int value)
    {
        _valueStr = _formatStr.arg(value, 0, _base);
        this->update();
    }

private:

    void update(void)
    {
        const auto text = QString("<b>%1:</b> %2").arg(_title.toHtmlEscaped()).arg(_valueStr.toHtmlEscaped());
        QMetaObject::invokeMethod(this, "setText", Qt::QueuedConnection, Q_ARG(QString, text));
    }

    size_t _base;
    QString _title;
    QString _valueStr;
    QString _formatStr;
};

static Pothos::BlockRegistry registerTextDisplay(
    "/widgets/text_display", &TextDisplay::make);

#include "TextDisplay.moc"
