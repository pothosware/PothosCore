// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <QSlider>

class Slider : public QSlider, public Pothos::Block
{
    Q_OBJECT
public:

    Slider(void):
        QSlider(nullptr)
    {
        this->registerCall(POTHOS_FCN_TUPLE(Slider, getWidget));
    }

    QWidget *getWidget(void)
    {
        return this;
    }

};

#include "Slider.moc"
