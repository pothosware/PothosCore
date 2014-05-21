// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstring> //memcpy
#include <vector>

class CollectorSink : Pothos::Block
{
public:
    CollectorSink(const Pothos::DType &dtype)
    {
        this->setupInput(0, dtype);
        this->registerCall(POTHOS_FCN_TUPLE(CollectorSink, getBuffer));
        this->registerCall(POTHOS_FCN_TUPLE(CollectorSink, getLabels));
        this->registerCall(POTHOS_FCN_TUPLE(CollectorSink, getMessages));
    }

    static Block *make(const Pothos::DType &dtype)
    {
        return new CollectorSink(dtype);
    }

    Pothos::BufferChunk getBuffer(void)
    {
        return _buffer;
    }

    std::vector<Pothos::Label> getLabels(void)
    {
        return _labels;
    }

    std::vector<Pothos::Object> getMessages(void)
    {
        return _messages;
    }

    void work(void)
    {
        auto inputPort = this->inputs()[0];

        //accumulate the buffer into a bigger buffer
        const auto &buffer = inputPort->buffer();
        if (buffer.length != 0)
        {
            Pothos::BufferChunk biggerBuffer = Pothos::SharedBuffer::make(buffer.length + _buffer.length);
            std::memcpy((void *)biggerBuffer.address, (const void *)_buffer.address, _buffer.length);
            std::memcpy((void *)(biggerBuffer.address+_buffer.length), (const void *)buffer.address, buffer.length);
            _buffer = biggerBuffer;
        }

        //consume buffer
        inputPort->consume(inputPort->elements());

        //store labels -- take all labels since we consume all the buffer
        while (inputPort->labels().begin() != inputPort->labels().end())
        {
            const auto &label = *inputPort->labels().begin();
            _labels.push_back(label);
            inputPort->removeLabel(label);
        }

        //store messages
        while (inputPort->hasMessage())
        {
            _messages.push_back(inputPort->popMessage());
        }
    }

private:
    Pothos::BufferChunk _buffer;
    std::vector<Pothos::Label> _labels;
    std::vector<Pothos::Object> _messages;
};

static Pothos::BlockRegistry registerSocketSink(
    "/blocks/sources/collector_sink", &CollectorSink::make);
