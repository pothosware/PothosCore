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
        if (numElems == 0) return;
        const long timeoutUs = this->workInfo().maxTimeoutNs/1000;
        const auto &buffs = this->workInfo().inputPointers;
        const int ret = _device->writeStream(_stream, buffs.data(), numElems, flags, timeNs, timeoutUs);

        //TODO labels

        if (ret > 0) for (auto input : this->inputs()) input->consume(ret);
        else if (ret == SOAPY_SDR_TIMEOUT) return this->yield();
        else throw Pothos::Exception("SDRSink::work()", "writeStream "+std::to_string(ret));
    }
};

static Pothos::BlockRegistry registerSDRSink(
    "/sdr/sink", &SDRSink::make);
