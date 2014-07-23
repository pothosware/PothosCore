// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Poco/Thread.h> //sleep
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/Types.h>
#include <random>
#include <chrono>
#include <queue>
#include <algorithm>

class FeederSource : Pothos::Block
{
public:
    FeederSource(const Pothos::DType &dtype)
    {
        this->setupOutput(0, dtype, this->uid()); //unique domain to force copies
        this->registerCall(POTHOS_FCN_TUPLE(FeederSource, feedTestPlan));
        this->registerCall(POTHOS_FCN_TUPLE(FeederSource, feedBuffer));
        this->registerCall(POTHOS_FCN_TUPLE(FeederSource, feedLabel));
        this->registerCall(POTHOS_FCN_TUPLE(FeederSource, feedMessage));
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

        //generate the buffers and elements
        const size_t numBuffs = bufferDist(gen);
        for (size_t bufno = 0; bufno < numBuffs; bufno++)
        {
            const size_t numElems = elementsDist(gen);
            totalElements += numElems;
            Pothos::BufferChunk buff(numElems*elemDType.size());
            for (size_t i = 0; i < numElems; i++)
            {
                auto value = valueDist(gen);
                expectedValues->add(value);
                if (elemDType.size() == 1) buff.as<char *>()[i] = char(value);
                else if (elemDType.size() == 2) buff.as<short *>()[i] = short(value);
                else if (elemDType.size() == 4) buff.as<int *>()[i] = int(value);
                else throw Pothos::AssertionViolationException("FeederSource::feedTestPlan()", "cant handle this dtype: " + elemDType.toString());
            }
            this->feedBuffer(buff);
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
        std::uniform_int_distribution<int> indexDist(0, totalElements-1);

        //generate random labels
        std::vector<unsigned long long> labelIndexes;
        std::map<unsigned long long, Poco::JSON::Object::Ptr> indexToLabelData;
        const size_t numLabels = labelDist(gen);
        for (size_t lblno = 0; lblno < numLabels; lblno++)
        {
            Pothos::Label lbl;
            lbl.index = indexDist(gen);
            auto data = random_string(dataSizeDist(gen));
            lbl.data = Pothos::Object(data);

            if (indexToLabelData.count(lbl.index) != 0) continue; //skip repeated indexes -- harder to check
            this->feedLabel(lbl);

            //record expected values
            Poco::JSON::Object::Ptr expectedLabel(new Poco::JSON::Object());
            expectedLabel->set("index", Poco::UInt64(lbl.index));
            expectedLabel->set("data", data);
            indexToLabelData[lbl.index] = expectedLabel;
            labelIndexes.push_back(lbl.index);
        }

        //load the sorted labels into the expected array
        std::sort(labelIndexes.begin(), labelIndexes.end());
        for (auto index : labelIndexes) expectedLabels->add(indexToLabelData.at(index));
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
            this->feedMessage(Pothos::Object(data));
            expectedMessages->add(data);
        }
    }

    return expectedResult;
}
