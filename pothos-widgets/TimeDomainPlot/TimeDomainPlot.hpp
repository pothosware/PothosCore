// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Framework.hpp>
#include <QGraphicsView>

class QGraphicsPathItem;

/***********************************************************************
 * |PothosDoc Time Domain Plot
 *
 * The time domain plot displays a live two dimensional plot of input elements vs time.
 *
 * |category /Widgets
 * |keywords time plot
 *
 * |param dtype[Data Type] The data type of the input elements.
 * |option [Complex128] "complex128"
 * |option [Float64] "float64"
 * |option [Complex64] "complex64"
 * |option [Float32] "float32"
 * |option [Complex Int64] "complex_int64"
 * |option [Int64] "int64"
 * |option [Complex Int32] "complex_int32"
 * |option [Int32] "int32"
 * |option [Complex Int16] "complex_int16"
 * |option [Int16] "int16"
 * |option [Complex Int8] "complex_int8"
 * |option [Int8] "int8"
 * |default "float32"
 * |preview disable
 *
 * |mode graphWidget
 * |factory /widgets/time_domain_plot(dtype)
 **********************************************************************/
class TimeDomainPlot : public QGraphicsView, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(const Pothos::DType &dtype)
    {
        return new TimeDomainPlot(dtype);
    }

    TimeDomainPlot(const Pothos::DType &dtype);

    QWidget *widget(void)
    {
        return this;
    }

    void work(void);

signals:
    void newPath(const QPainterPath &);

private slots:
    void handleNewPath(const QPainterPath &);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    QGraphicsPathItem *_plotterElements;
};
