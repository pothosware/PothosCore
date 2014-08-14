// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "TimeDomainPlot.hpp"
#include <QGraphicsScene>

TimeDomainPlot::TimeDomainPlot(const Pothos::DType &dtype)
{
    this->registerCall(this, POTHOS_FCN_TUPLE(TimeDomainPlot, widget));
    this->setupInput(0, dtype);

    this->setScene(new QGraphicsScene(this));
}

void TimeDomainPlot::work(void)
{
    auto inPort = this->input(0);

    inPort->consume(inPort->elements());
}

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerTimeDomainPlot(
    "/widgets/time_domain_plot", &TimeDomainPlot::make);
