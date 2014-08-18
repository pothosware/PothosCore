// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Object/Containers.hpp>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <iostream>
#include <map>

/***********************************************************************
 * |PothosDoc Radio Group
 *
 * The radio group widget allows for selection of a value
 * from a group of radio buttons.
 *
 * |category /Widgets
 * |keywords numeric entry spinbox
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

        qRegisterMetaType<Pothos::ObjectVector>("Pothos::ObjectVector");
        connect(this, SIGNAL(optionsChanged(const Pothos::ObjectVector &)),
                this, SLOT(handleOptionsChanged(const Pothos::ObjectVector &)));
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
        for (auto pair : _radioToOption)
        {
            try
            {
                pair.first->setChecked(pair.second.compareTo(value) == 0);
            }
            catch (const Pothos::ObjectCompareError &ex){}
        }
    }

    void setOptions(const Pothos::ObjectVector &options)
    {
        emit this->optionsChanged(options);
    }

signals:

    void optionsChanged(const Pothos::ObjectVector &);

private slots:

    void handleOptionsChanged(const Pothos::ObjectVector &options)
    {
        auto oldValue = this->value();
        this->clear();

        for (const auto &option : options)
        {
            auto optPair = option.convert<Pothos::ObjectVector>();
            auto title = optPair.at(0).convert<QString>();
            auto value = optPair.at(1);
            auto radio = new QRadioButton(title, this);
            connect(radio, SIGNAL(toggled(bool)), this, SLOT(handleRadioChanged(bool)));
            _radioToOption[radio] = value;
            _layout->addWidget(radio);
        }

        this->setValue(oldValue);
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
