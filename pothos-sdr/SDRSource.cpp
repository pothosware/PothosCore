// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SDRBlock.hpp"

class SDRSource : public SDRBlock
{
public:
    static Block *make(const Pothos::DType &dtype, const std::vector<size_t> &channels)
    {
        return new SDRSource(dtype, channels);
    }

    SDRSource(const Pothos::DType &dtype, const std::vector<size_t> &channels):
        SDRBlock(SOAPY_SDR_RX, dtype, channels),
        _postTime(false)
    {
        for (size_t i = 0; i < _channels.size(); i++) this->setupOutput(i, dtype);
    }

    /*******************************************************************
     * Streaming implementation
     ******************************************************************/
    void activate(void)
    {
        SDRBlock::activate();
        _postTime = true;
    }

    void work(void)
    {
        int flags = 0;
        long long timeNs = 0;
        const size_t numElems = this->workInfo().minOutElements;
        if (numElems == 0) return;
        const long timeoutUs = this->workInfo().maxTimeoutNs/1000;
        const auto &buffs = this->workInfo().outputPointers;
        const int ret = _device->readStream(_stream, buffs.data(), numElems, flags, timeNs, timeoutUs);

        //handle error
        if (ret <= 0)
        {
            //got timeout? just call again
            if (ret == SOAPY_SDR_TIMEOUT) return this->yield();
            //got overflow? call again, discontinuity means repost time
            if (ret == SOAPY_SDR_OVERFLOW) _postTime = true;
            if (ret == SOAPY_SDR_OVERFLOW) return this->yield();
            //otherwise throw an exception with the error code
            throw Pothos::Exception("SDRSource::work()", "readStream "+std::to_string(ret));
        }

        //produce output and post pending labels
        for (auto output : this->outputs())
        {
            output->produce(size_t(ret));

            //pending rx configuration labels
            auto &pending = _pendingLabels.at(output->index());
            if (pending.empty()) continue;
            for (const auto &pair : pending)
            {
                output->postLabel(Pothos::Label(pair.first, pair.second, 0));
            }
            pending.clear();
        }

        //post labels from stream data
        if (_postTime and (flags & SOAPY_SDR_HAS_TIME) != 0)
        {
            _postTime = false;
            const Pothos::Label lbl("rxTime", timeNs, 0);
            for (auto output : this->outputs()) output->postLabel(lbl);
        }
        if ((flags & SOAPY_SDR_END_BURST) != 0)
        {
            _postTime = true; //discontinuity: repost time on next receive
            const Pothos::Label lbl("rxEnd", true, ret-1);
            for (auto output : this->outputs()) output->postLabel(lbl);
        }
    }

private:
    bool _postTime;
};

static Pothos::BlockRegistry registerSDRSource(
    "/sdr/source", &SDRSource::make);
