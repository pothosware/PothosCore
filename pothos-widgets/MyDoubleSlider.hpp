// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <QSlider>

class MyDoubleSlider : public QSlider
{
    Q_OBJECT
public:
    MyDoubleSlider(const Qt::Orientation orientation, QWidget *parent = nullptr);

    double value(void) const;

signals:
    void valueChanged(const double);

public slots:
    void setValue(const double val);
    void setMinimum(const double min);
    void setMaximum(const double max);
    void setSingleStep(const double val);

private slots:
    void handleIntValueChanged(const int);

private:
    void updateRange(const double oldValue);

    double _minimum;
    double _maximum;
    double _stepSize;
};
