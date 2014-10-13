// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SDRBlock.hpp"

class SDRSink : public SDRBlock
{
public:
    static Block *make(const Pothos::DType &dtype, const std::vector<size_t> &channels)
    {
        return new SDRSink(dtype, channels);
    }

    SDRSink(const Pothos::DType &dtype, const std::vector<size_t> &channels):
        SDRBlock(SOAPY_SDR_TX, channels)
    {
        for (size_t i = 0; i < channels.size(); i++) this->setupInput(i, dtype);
    }

    /*******************************************************************
     * Streaming implementation
     ******************************************************************/
    void work(void)
    {
        int flags = 0;
        const long long timeNs = 0;
        const size_t numElems = this->workInfo().minInElements;
        const long timeoutUs = this->workInfo().maxTimeoutNs/1000;
        const auto &buffs = this->workInfo().inputPointers;
        const int ret = _device->writeStream(_stream, buffs.data(), numElems, flags, timeNs, timeoutUs);

        //TODO labels

        if (ret > 0) for (auto output : this->outputs()) output->produce(numElems);
        else if (ret == SOAPY_SDR_TIMEOUT) return this->yield();
        else poco_error_f1(Poco::Logger::get("SDRSink"), "writeStream %d", ret);
    }
};

static Pothos::BlockRegistry registerSDRSink(
    "/sdr/sink", &SDRSink::make);
