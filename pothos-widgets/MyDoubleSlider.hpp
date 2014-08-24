// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <QSlider>
#include <qwt_global.h>

class QWT_EXPORT MyDoubleSlider : public QSlider
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
    void updateRange(void);

    double _minimum;
    double _maximum;
    double _stepSize;
};
