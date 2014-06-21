// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Poco/Thread.h> //sleep
#include <queue>

class FeederSource : Pothos::Block
{
public:
    FeederSource(const Pothos::DType &dtype)
    {
        this->setupOutput(0, dtype, this->uid()); //unique domain to force copies
        this->registerCall(POTHOS_FCN_TUPLE(FeederSource, feedBuffer));
        this->registerCall(POTHOS_FCN_TUPLE(FeederSource, feedLabel));
        this->registerCall(POTHOS_FCN_TUPLE(FeederSource, feedMessage));
    }

    static Block *make(const Pothos::DType &dtype)
    {
        return new FeederSource(dtype);
    }

    void feedBuffer(const Pothos::BufferChunk &buffer)
    {
        _buffers.push(buffer);
    }

    void feedLabel(const Pothos::Label &label)
    {
        _labels.push(label);
    }

    void feedMessage(const Pothos::Object &message)
    {
        _messages.push(message);
    }

    void work(void)
    {
        //do labels first since we return after each type
        if (not _labels.empty())
        {
            this->outputs()[0]->postLabel(_labels.front());
            _labels.pop();
            return;
        }
        if (not _buffers.empty())
        {
            this->outputs()[0]->postBuffer(_buffers.front());
            _buffers.pop();
            return;
        }
        if (not _messages.empty())
        {
            this->outputs()[0]->postMessage(_messages.front());
            _messages.pop();
            return;
        }

        //enter backoff + wait for additional user stimulus
        Poco::Thread::sleep(this->workInfo().maxTimeoutNs/1000000); //ms
        this->yield();
    }

private:
    std::queue<Pothos::BufferChunk> _buffers;
    std::queue<Pothos::Label> _labels;
    std::queue<Pothos::Object> _messages;
};

static Pothos::BlockRegistry registerSocketSink(
    "/blocks/sources/feeder_source", &FeederSource::make);
