// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "MyDoubleSlider.hpp"

MyDoubleSlider::MyDoubleSlider(const Qt::Orientation orientation, QWidget *parent):
    QSlider(orientation, parent)
{
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(handleIntValueChanged(int)));
}

double MyDoubleSlider::value(void) const
{
    int int_norm = QSlider::value()-QSlider::minimum();
    double scale = int_norm/double(QSlider::maximum()-QSlider::minimum());
    double float_norm = scale*(_maximum - _minimum);
    return float_norm + _minimum;
}

void MyDoubleSlider::setValue(const double val)
{
    double float_norm = val - _minimum;
    double scale = float_norm/(_maximum - _minimum);
    int int_norm = int(scale*(QSlider::maximum()-QSlider::minimum()));
    QSlider::setValue(int_norm + QSlider::minimum());
}

void MyDoubleSlider::setMinimum(const double min)
{
    _minimum = min;
    this->updateRange();
}

void MyDoubleSlider::setMaximum(const double max)
{
    _maximum = max;
    this->updateRange();
}

void MyDoubleSlider::setSingleStep(const double val)
{
    _stepSize = val;
    this->updateRange();
}

void MyDoubleSlider::handleIntValueChanged(const int)
{
    emit this->valueChanged(this->value());
}

void MyDoubleSlider::updateRange(void)
{
    auto oldValue = this->value();
    QSlider::setMinimum(_minimum/_stepSize);
    QSlider::setMaximum(_maximum/_stepSize);
    this->setValue(oldValue);
}
