// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "ConstellationDisplay.hpp"
#include <Pothos/Framework.hpp>
#include <iostream>

/***********************************************************************
 * |PothosDoc Constellation
 *
 * The periodogram plot displays a live constellation plot of a complex signal.
 *
 * |category /Widgets
 * |keywords plot constellation scatter
 *
 * |param title The title of the plot
 * |default "Constellation"
 * |widget StringEntry()
 *
 * |param displayRate[Display Rate] How often the plotter updates.
 * |default 10.0
 * |units updates/sec
 *
 * |param numPoints[Num Points] The number of points per plot capture.
 * |default 1024
 *
 * |param autoScale[Auto-Scale] Enable automatic scaling for the vertical axis.
 * |default false
 * |option [Auto scale] true
 * |option [Use limits] false
 * |preview disable
 * |tab Axis
 *
 * |param xRange[X-Axis Range] The minimum and maximum values for the X-Axis.
 * When auto scale is off, this parameter controls the horizontal axis.
 * |default [-1.5, 1.5]
 * |preview disable
 * |tab Axis
 *
 * |param yRange[Y-Axis Range] The minimum and maximum values for the Y-Axis.
 * When auto scale is off, this parameter controls the vertical axis.
 * |default [-1.5, 1.5]
 * |preview disable
 * |tab Axis
 *
 * |param enableXAxis[Enable X-Axis] Show or hide the horizontal axis markers.
 * |option [Show] true
 * |option [Hide] false
 * |default true
 * |preview disable
 * |tab Axis
 *
 * |param enableYAxis[Enable Y-Axis] Show or hide the vertical axis markers.
 * |option [Show] true
 * |option [Hide] false
 * |default true
 * |preview disable
 * |tab Axis
 *
 * |mode graphWidget
 * |factory /widgets/constellation(remoteEnv)
 * |setter setTitle(title)
 * |setter setDisplayRate(displayRate)
 * |setter setNumPoints(numPoints)
 * |setter setAutoScale(autoScale)
 * |setter setXRange(xRange)
 * |setter setYRange(yRange)
 * |setter enableXAxis(enableXAxis)
 * |setter enableYAxis(enableYAxis)
 **********************************************************************/
class Constellation : public Pothos::Topology
{
public:
    static Topology *make(const Pothos::ProxyEnvironment::Sptr &remoteEnv)
    {
        return new Constellation(remoteEnv);
    }

    Constellation(const Pothos::ProxyEnvironment::Sptr &remoteEnv)
    {
        _display.reset(new ConstellationDisplay());

        auto registry = remoteEnv->findProxy("Pothos/BlockRegistry");
        _snooper = registry.callProxy("/blocks/stream_snooper");

        //register calls in this topology
        this->registerCall(this, POTHOS_FCN_TUPLE(Constellation, setDisplayRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(Constellation, setNumPoints));

        //connect to internal display block
        this->connect(this, "setTitle", _display, "setTitle");
        this->connect(this, "setAutoScale", _display, "setAutoScale");
        this->connect(this, "setXRange", _display, "setXRange");
        this->connect(this, "setYRange", _display, "setYRange");
        this->connect(this, "enableXAxis", _display, "enableXAxis");
        this->connect(this, "enableYAxis", _display, "enableYAxis");

        //connect to the internal snooper block
        this->connect(this, "setDisplayRate", _snooper, "setTriggerRate");
        this->connect(this, "setNumPoints", _snooper, "setChunkSize");

        //connect stream ports
        this->connect(this, 0, _snooper, 0);
        this->connect(_snooper, 0, _display, 0);
    }

    Pothos::Object opaqueCallMethod(const std::string &name, const Pothos::Object *inputArgs, const size_t numArgs) const
    {
        //calls that go to the topology
        try
        {
            return Pothos::Topology::opaqueCallMethod(name, inputArgs, numArgs);
        }
        catch (const Pothos::BlockCallNotFound &){}

        //forward everything else to display
        return _display->opaqueCallMethod(name, inputArgs, numArgs);
    }

    void setDisplayRate(const double rate)
    {
        _snooper.callVoid("setTriggerRate", rate);
    }

    void setNumPoints(const size_t num)
    {
        _display->setName(this->getName()+"Display");
        _snooper.callVoid("setName", this->getName()+"Snooper");

        _snooper.callVoid("setChunkSize", num);
    }

private:
    Pothos::Proxy _snooper;
    std::shared_ptr<ConstellationDisplay> _display;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerConstellation(
    "/widgets/constellation", &Constellation::make);
