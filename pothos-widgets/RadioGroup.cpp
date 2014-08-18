// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Object/Containers.hpp>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <map>

/***********************************************************************
 * |PothosDoc Radio Group
 *
 * The radio group widget allows for selection of a value
 * from a group of radio buttons.
 *
 * |category /Widgets
 * |keywords radio buttons
 *
 * |param title The name of the value displayed by this widget
 * |default "My Radio Value"
 *
 * |param value The initial selection for this radio group.
 * |default 42
 *
 * |param options A list of options to choose from.
 * Options take the form of an Object vector
 * where each entry is another Object vector
 * containing a displayable name, and a value for the option.
 *
 * |default [["Opt0", 42], ["Opt1", "xyz"]]
 *
 * |mode graphWidget
 * |factory /widgets/radio_group()
 * |setter setTitle(title)
 * |setter setOptions(options)
 * |setter setValue(value)
 **********************************************************************/
class RadioGroup : public QGroupBox, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(void)
    {
        return new RadioGroup();
    }

    RadioGroup(void):
        _layout(new QVBoxLayout(this))
    {
        this->setStyleSheet("QGroupBox {font-weight: bold;}");
        this->registerCall(this, POTHOS_FCN_TUPLE(RadioGroup, widget));
        this->registerCall(this, POTHOS_FCN_TUPLE(RadioGroup, setTitle));
        this->registerCall(this, POTHOS_FCN_TUPLE(RadioGroup, setValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(RadioGroup, setOptions));
        this->registerSignal("valueChanged");

        qRegisterMetaType<Pothos::Object>("Pothos::Object");
        qRegisterMetaType<Pothos::ObjectVector>("Pothos::ObjectVector");
    }

    QWidget *widget(void)
    {
        return this;
    }

    void activate(void)
    {
        //emit current value when design becomes active
        this->emitSignalArgs("valueChanged", Pothos::ObjectVector(1, this->value()));
    }

    Pothos::Object value(void) const
    {
        for (auto pair : _radioToOption)
        {
            if (pair.first->isChecked()) return pair.second;
        }
        return Pothos::Object();
    }

    void setValue(const Pothos::Object &value)
    {
        QMetaObject::invokeMethod(this, "__setValue", Qt::QueuedConnection, Q_ARG(Pothos::Object, value));
    }

    void setOptions(const Pothos::ObjectVector &options)
    {
        QMetaObject::invokeMethod(this, "__setOptions", Qt::QueuedConnection, Q_ARG(Pothos::ObjectVector, options));
    }

private slots:

    void __setOptions(const Pothos::ObjectVector &options)
    {
        auto oldValue = this->value();
        this->clear();

        for (const auto &option : options)
        {
            if (not option.canConvert(typeid(Pothos::ObjectVector))) throw Pothos::DataFormatException("RadioGroup::setOptions()", "entry is not ObjectVector");
            auto optPair = option.convert<Pothos::ObjectVector>();
            if (optPair.size() != 2) throw Pothos::DataFormatException("RadioGroup::setOptions()", "entry must be ObjectVector of size == 2");
            if (not optPair.at(0).canConvert(typeid(QString))) throw Pothos::DataFormatException("RadioGroup::setOptions()", "entry[0] must be a string");
            auto title = optPair.at(0).convert<QString>();
            auto value = optPair.at(1);
            auto radio = new QRadioButton(title, this);
            connect(radio, SIGNAL(toggled(bool)), this, SLOT(handleRadioChanged(bool)));
            _radioToOption[radio] = value;
            _layout->addWidget(radio);
        }

        this->setValue(oldValue);
    }

    void __setValue(const Pothos::Object &value)
    {
        for (auto pair : _radioToOption)
        {
            pair.first->setChecked(pair.second.equals(value));
        }
    }

    void handleRadioChanged(bool)
    {
        this->emitSignalArgs("valueChanged", Pothos::ObjectVector(1, this->value()));
    }

private:

    void clear(void)
    {
        for (auto pair : _radioToOption) delete pair.first;
        _radioToOption.clear();
    }

    QVBoxLayout *_layout;
    std::map<QRadioButton *, Pothos::Object> _radioToOption;
};

static Pothos::BlockRegistry registerRadioGroup(
    "/widgets/radio_group", &RadioGroup::make);

#include "RadioGroup.moc"
