// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>

/***********************************************************************
 * |PothosDoc Dynamic Router
 *
 * The dynamic router is a zero-copy switch board for streams.
 * Any input stream can be routed to any output stream.
 * The routing configuration can be changed at runtime.
 *
 * |category /Misc
 * |keywords router
 *
 * |param destinations An array of output port indexes, one per input port.
 * Destinations is an array of integers where each element specifies an output port.
 * An output port of -1 indicates that the input will be consumed and dropped.
 * <ul>
 * <li>Example: [0, 2] -> input0 routes to output0, input1 routes to output2</li>
 * <li>Example: [1, -1] -> input0 routes to output1, input1 is dropped</li>
 * </ul>
 * |default "[0]"
 *
 * |factory /blocks/dynamic_router()
 * |setter setDestinations(destinations)
 **********************************************************************/
class DynamicRouter : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new DynamicRouter();
    }

    DynamicRouter(void)
    {
        this->setupInput(0);
        this->setupOutput(0);
        this->registerCall(POTHOS_FCN_TUPLE(DynamicRouter, setDestinations));
    }

    void setDestinations(const std::string &destJSON)
    {
        Poco::JSON::Parser p; p.parse(destJSON);
        const auto dests = p.getHandler()->asVar().extract<Poco::JSON::Array::Ptr>();
        _destinations.resize(dests->size());
        for (size_t i = 0; i < dests->size(); i++)
        {
            _destinations[i] = dests->getElement<int>(i);
        }
    }

    void work(void)
    {
        for (auto inputPort : this->inputs())
        {
            auto dest = (size_t(inputPort->index()) < _destinations.size())? _destinations.at(inputPort->index()) : -1;
            auto outputPort = (dest > 0)? this->output(dest) : nullptr;

            if (inputPort->hasMessage())
            {
                auto m = inputPort->popMessage();
                if (outputPort != nullptr) outputPort->postMessage(m);
            }

            while (inputPort->labels().begin() != inputPort->labels().end())
            {
                const auto &label = *inputPort->labels().begin();
                if (outputPort != nullptr) outputPort->postLabel(label);
                inputPort->removeLabel(label);
            }

            const auto &buffer = inputPort->buffer();
            if (buffer.length != 0)
            {
                if (outputPort != nullptr) outputPort->postBuffer(buffer);
                inputPort->consume(inputPort->elements());
            }
        }
    }

private:
    std::vector<int> _destinations;
};

static Pothos::BlockRegistry registerDynamicRouter(
    "/blocks/dynamic_router", &DynamicRouter::make);
