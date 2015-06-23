// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/Types.h>
#include <random>
#include <chrono>
#include <thread>
#include <queue>
#include <algorithm>

class FeederSource : Pothos::Block
{
public:
    FeederSource(const Pothos::DType &dtype)
    {
        this->setupOutput(0, dtype, this->uid()); //unique domain to force copies
        this->registerCall(this, POTHOS_FCN_TUPLE(FeederSource, feedTestPlan));
        this->registerCall(this, POTHOS_FCN_TUPLE(FeederSource, feedBuffer));
        this->registerCall(this, POTHOS_FCN_TUPLE(FeederSource, feedLabel));
        this->registerCall(this, POTHOS_FCN_TUPLE(FeederSource, feedMessage));
    }

    static Block *make(const Pothos::DType &dtype)
    {
        return new FeederSource(dtype);
    }

    Poco::JSON::Object::Ptr feedTestPlan(const Poco::JSON::Object::Ptr &testPlan);

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
        auto outputPort = this->output(0);

        //do labels first so they remain ahead of buffers
        while (not _labels.empty())
        {
            auto buffElems = _buffers.empty()?0:_buffers.front().length/outputPort->dtype().size();
            if (_labels.front().index >= buffElems + outputPort->totalElements()) break;

            _labels.front().index -= outputPort->totalElements(); //abs -> rel
            outputPort->postLabel(_labels.front());
            _labels.pop();
        }
        while (not _buffers.empty())
        {
            outputPort->postBuffer(_buffers.front());
            _buffers.pop();
            return;
        }
        while (not _messages.empty())
        {
            outputPort->postMessage(_messages.front());
            _messages.pop();
            return;
        }

        //enter backoff + wait for additional user stimulus
        std::this_thread::sleep_for(std::chrono::nanoseconds(this->workInfo().maxTimeoutNs));
        this->yield();
    }

private:
    std::queue<Pothos::BufferChunk> _buffers;
    std::queue<Pothos::Label> _labels;
    std::queue<Pothos::Object> _messages;
};

static Pothos::BlockRegistry registerSocketSink(
    "/blocks/feeder_source", &FeederSource::make);


//http://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
std::string random_string(size_t length)
{
    static const std::string alphanums =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    static std::mt19937 rg(std::chrono::system_clock::now().time_since_epoch().count());
    static std::uniform_int_distribution<> pick(0, alphanums.size() - 1);

    std::string s;

    s.reserve(length);

    while(length--)
        s += alphanums[pick(rg)];

    return s;
}

Poco::JSON::Object::Ptr FeederSource::feedTestPlan(const Poco::JSON::Object::Ptr &testPlan)
{
    Poco::JSON::Object::Ptr expectedResult(new Poco::JSON::Object());

    //results to feed into the block
    std::vector<Pothos::BufferChunk> buffers;
    std::vector<Pothos::Label> labels;
    std::vector<Pothos::Object> messages;

    std::random_device rd;
    std::mt19937 gen(rd());

    //defaults
    auto minTrials = testPlan->optValue<int>("minTrials", 10);
    auto maxTrials = testPlan->optValue<int>("maxTrials", 100);
    auto minSize = testPlan->optValue<int>("minSize", 10);
    auto maxSize = testPlan->optValue<int>("maxSize", 100);

    unsigned long long totalElements = 0;
    if (testPlan->has("enableBuffers"))
    {
        auto elemDType = this->output(0)->dtype();

        //container for expected values
        Poco::JSON::Array::Ptr expectedValues(new Poco::JSON::Array());
        expectedResult->set("expectedValues", expectedValues);

        //create random distributions
        std::uniform_int_distribution<int> bufferDist(
            testPlan->optValue<int>("minBuffers", minTrials),
            testPlan->optValue<int>("maxBuffers", maxTrials));
        std::uniform_int_distribution<int> elementsDist(
            testPlan->optValue<int>("minBufferSize", minSize)/elemDType.size(),
            testPlan->optValue<int>("maxBufferSize", maxSize)/elemDType.size());
        std::uniform_int_distribution<int> valueDist(
            testPlan->optValue<int>("minValue", -(1 << (elemDType.size()*8 - 1))),
            testPlan->optValue<int>("maxValue", (1 << (elemDType.size()*8 - 1))-1));

        //constraints on random numbers of elements produced
        const int totalMultiple = testPlan->optValue<int>("totalMultiple", 1);
        const int bufferMultiple = testPlan->optValue<int>("bufferMultiple", 1);

        //generate the buffers and elements
        const size_t numBuffs = bufferDist(gen);
        for (size_t bufno = 0; bufno < numBuffs; bufno++)
        {
            int numElems = elementsDist(gen);

            //round up to multiple and re-enforce the distribution bounds
            numElems = ((numElems + bufferMultiple - 1)/bufferMultiple)*bufferMultiple;
            if (numElems > elementsDist.b()) numElems -= bufferMultiple;
            if (numElems < elementsDist.a()) numElems += bufferMultiple;

            //pad last buffer to multiple when specified
            if ((bufno+1) == numBuffs)
            {
                const size_t extra = (totalElements + numElems) % totalMultiple;
                if (extra != 0) numElems += totalMultiple - extra;
            }

            totalElements += numElems;
            Pothos::BufferChunk buff(numElems*elemDType.size());
            for (int i = 0; i < numElems; i++)
            {
                auto value = valueDist(gen);
                expectedValues->add(value);
                if (elemDType.size() == 1) buff.as<char *>()[i] = char(value);
                else if (elemDType.size() == 2) buff.as<short *>()[i] = short(value);
                else if (elemDType.size() == 4) buff.as<int *>()[i] = int(value);
                else throw Pothos::AssertionViolationException("FeederSource::feedTestPlan()", "cant handle this dtype: " + elemDType.toString());
            }
            buffers.push_back(buff);
        }
    }

    if (testPlan->has("enableLabels") and totalElements > 0)
    {
        //container for expected labels
        Poco::JSON::Array::Ptr expectedLabels(new Poco::JSON::Array());
        expectedResult->set("expectedLabels", expectedLabels);

        //create random distributions
        std::uniform_int_distribution<int> labelDist(
            testPlan->optValue<int>("minLabels", minTrials),
            testPlan->optValue<int>("maxLabels", maxTrials));
        std::uniform_int_distribution<int> dataSizeDist(
            testPlan->optValue<int>("minLabelSize", minSize),
            testPlan->optValue<int>("maxLabelSize", maxSize));
        std::uniform_int_distribution<unsigned long long> indexDist(0, totalElements-1);

        //generate random label indexes and sort them
        std::vector<unsigned long long> labelIndexes;
        const size_t numLabels = labelDist(gen);
        for (size_t lblno = 0; lblno < numLabels; lblno++)
        {
            auto index = indexDist(gen);
            if (std::find(labelIndexes.begin(), labelIndexes.end(), index) == labelIndexes.end())
            {
                labelIndexes.push_back(index);
            }
        }
        std::sort(labelIndexes.begin(), labelIndexes.end());

        //generate random labels
        for (auto index : labelIndexes)
        {
            Pothos::Label lbl;
            lbl.index = index + this->output(0)->totalElements();
            auto data = random_string(dataSizeDist(gen));
            lbl.data = Pothos::Object(data);
            lbl.id = "id"+std::to_string(lbl.index);
            labels.push_back(lbl);

            //record expected values
            Poco::JSON::Object::Ptr expectedLabel(new Poco::JSON::Object());
            expectedLabel->set("index", Poco::UInt64(index));
            expectedLabel->set("data", data);
            expectedLabel->set("id", lbl.id);
            expectedLabels->add(expectedLabel);
        }
    }

    if (testPlan->has("enableMessages"))
    {
        //container for expected messages
        Poco::JSON::Array::Ptr expectedMessages(new Poco::JSON::Array());
        expectedResult->set("expectedMessages", expectedMessages);

        //create random distributions
        std::uniform_int_distribution<int> messageDist(
            testPlan->optValue<int>("minMessages", minTrials),
            testPlan->optValue<int>("maxMessages", maxTrials));
        std::uniform_int_distribution<int> dataSizeDist(
            testPlan->optValue<int>("minMessageSize", minSize),
            testPlan->optValue<int>("maxMessageSize", maxSize));

        const size_t numMessages = messageDist(gen);
        for (size_t msgno = 0; msgno < numMessages; msgno++)
        {
            auto data = random_string(dataSizeDist(gen));
            messages.push_back(Pothos::Object(data));
            expectedMessages->add(data);
        }
    }

    //feed the generated data
    for (const auto &lbl : labels) this->feedLabel(lbl);
    for (const auto &buff : buffers) this->feedBuffer(buff);
    for (const auto &msg : messages) this->feedMessage(msg);

    return expectedResult;
}
