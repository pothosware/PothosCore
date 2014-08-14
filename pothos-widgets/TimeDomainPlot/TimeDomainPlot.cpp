// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "TimeDomainPlot.hpp"
#include <QGraphicsScene>
#include <QGraphicsPathItem>

TimeDomainPlot::TimeDomainPlot(const Pothos::DType &dtype):
    _plotterElements(new QGraphicsPathItem())
{
    this->registerCall(this, POTHOS_FCN_TUPLE(TimeDomainPlot, widget));
    this->setupInput(0, dtype);

    this->setScene(new QGraphicsScene(this));
    this->scene()->addItem(_plotterElements);

    qRegisterMetaType<QPainterPath>("QPainterPath");
    connect(this, SIGNAL(newPath(const QPainterPath &)), this, SLOT(handleNewPath(const QPainterPath &)), Qt::QueuedConnection);
}

void TimeDomainPlot::handleNewPath(const QPainterPath &path)
{
    _plotterElements->setPath(path);
}

void TimeDomainPlot::work(void)
{
    auto inPort = this->input(0);

    if (inPort->elements() > 100)
    {
        if (inPort->dtype() == Pothos::DType("float32"))
        {
            auto points = inPort->buffer().as<const float *>();
            QPainterPath path(QPointF(0, points[0]*100));
            for (size_t i = 1; i < inPort->elements(); i++)
            {
                path.lineTo(i, points[i]*100);
            }
            emit this->newPath(path);
        }
    }

    inPort->consume(inPort->elements());
}

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerTimeDomainPlot(
    "/widgets/time_domain_plot", &TimeDomainPlot::make);
