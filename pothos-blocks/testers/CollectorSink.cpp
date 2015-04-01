// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/Format.h>
#include <Poco/Types.h>
#include <cstring> //memcpy
#include <vector>
#include <algorithm> //min/max

class CollectorSink : Pothos::Block
{
public:
    CollectorSink(const Pothos::DType &dtype)
    {
        this->setupInput(0, dtype);
        this->registerCall(this, POTHOS_FCN_TUPLE(CollectorSink, getBuffer));
        this->registerCall(this, POTHOS_FCN_TUPLE(CollectorSink, getLabels));
        this->registerCall(this, POTHOS_FCN_TUPLE(CollectorSink, getMessages));
        this->registerCall(this, POTHOS_FCN_TUPLE(CollectorSink, verifyTestPlan));
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

    void verifyTestPlan(const Poco::JSON::Object::Ptr &expected) const;

    void work(void)
    {
        auto inputPort = this->input(0);

        //accumulate the buffer into a bigger buffer
        const auto &buffer = inputPort->buffer();
        if (buffer.length != 0) _buffer.append(buffer);

        //consume buffer
        inputPort->consume(inputPort->elements());

        //store labels -- take all labels since we consume all the buffer
        while (inputPort->labels().begin() != inputPort->labels().end())
        {
            auto label = *inputPort->labels().begin();
            inputPort->removeLabel(label);
            label.index += inputPort->totalElements(); //rel -> abs
            _labels.push_back(label);
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

static Pothos::BlockRegistry registerCollectorSink(
    "/blocks/collector_sink", &CollectorSink::make);


void CollectorSink::verifyTestPlan(const Poco::JSON::Object::Ptr &expected) const
{
    bool checked = false;

    if (expected->has("expectedValues"))
    {
        if (_buffer.length == 0) throw Pothos::AssertionViolationException("CollectorSink::verifyTestPlan()", "collector buffer is empty");
        if (not (_buffer.dtype == this->input(0)->dtype())) throw Pothos::AssertionViolationException("CollectorSink::verifyTestPlan()",
            Poco::format("Buffer type mismatch: expected %s -> actual %s", this->input(0)->dtype().toString(), _buffer.dtype.toString()));

        auto expectedValues = expected->getArray("expectedValues");
        auto intBuffer = _buffer.convert(typeid(int));
        auto numActualElems = intBuffer.elements();

        for (size_t i = 0; i < std::min(numActualElems, expectedValues->size()); i++)
        {
            const auto value = expectedValues->getElement<int>(i);
            const auto actual = intBuffer.as<const int *>()[i];
            if (value != actual) throw Pothos::AssertionViolationException("CollectorSink::verifyTestPlan()",
                Poco::format("Value check for element %d: expected %d -> actual %d", int(i), value, actual));
        }

        if (numActualElems != expectedValues->size()) throw Pothos::AssertionViolationException("CollectorSink::verifyTestPlan()",
            Poco::format("Check expected %d elements, actual %d elements", int(expectedValues->size()), int(numActualElems)));
        checked = true;
    }

    if (expected->has("expectedLabels"))
    {
        auto expectedLabels = expected->getArray("expectedLabels");
        for (size_t i = 0; i < std::min(_labels.size(), expectedLabels->size()); i++)
        {
            auto lbl = _labels[i];
            auto expectedLabel = expectedLabels->getObject(i);
            auto value = expectedLabel->getValue<std::string>("data");
            auto index = expectedLabel->getValue<Poco::UInt64>("index");
            auto id = expectedLabel->getValue<std::string>("id");
            if (lbl.data.type() != typeid(std::string)) throw Pothos::AssertionViolationException("CollectorSink::verifyTestPlan()",
                "cant handle this label type: " + lbl.data.getTypeString());
            auto actual = lbl.data.extract<std::string>();
            if (lbl.index != index) throw Pothos::AssertionViolationException("CollectorSink::verifyTestPlan()",
                Poco::format("Value check for label index %d: expected %d -> actual %d", int(i), int(index), int(lbl.index)));
            if (lbl.id != id) throw Pothos::AssertionViolationException("CollectorSink::verifyTestPlan()",
                Poco::format("Value check for label id %d: expected '%s' -> actual '%s'", int(i), id, lbl.id));
            if (actual != value) throw Pothos::AssertionViolationException("CollectorSink::verifyTestPlan()",
                Poco::format("Value check for label data %d: expected '%s' -> actual '%s'", int(i), value, actual));
        }

        if (_labels.size() != expectedLabels->size()) throw Pothos::AssertionViolationException("CollectorSink::verifyTestPlan()",
            Poco::format("Check expected %d labels, actual %d labels", int(expectedLabels->size()), int(_labels.size())));
        checked = true;
    }

    if (expected->has("expectedMessages"))
    {
        auto expectedMessages = expected->getArray("expectedMessages");
        for (size_t i = 0; i < std::min(_messages.size(), expectedMessages->size()); i++)
        {
            auto msg = _messages[i];
            auto value = expectedMessages->getElement<std::string>(i);
            if (msg.type() != typeid(std::string)) throw Pothos::AssertionViolationException("CollectorSink::verifyTestPlan()",
                "cant handle this message type: " + msg.getTypeString());
            auto actual = msg.extract<std::string>();
            if (actual != value) throw Pothos::AssertionViolationException("CollectorSink::verifyTestPlan()",
                Poco::format("Value check for message %d: expected %s -> actual %s", int(i), value, actual));
        }

        if (_messages.size() != expectedMessages->size()) throw Pothos::AssertionViolationException("CollectorSink::verifyTestPlan()",
            Poco::format("Check expected %d messages, actual %d messages", int(expectedMessages->size()), int(_messages.size())));
        checked = true;
    }

    if (not checked) throw Pothos::AssertionViolationException("CollectorSink::verifyTestPlan()", "nothing checked!");
}
