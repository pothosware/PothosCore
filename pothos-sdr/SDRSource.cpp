// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SDRBlock.hpp"
#include "SDRSourceBufferManager.hpp"

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

    Pothos::BufferManager::Sptr getOutputBufferManager(const std::string &, const std::string &domain)
    {
        //Try to use a DMA buffer manager when the upstream domain is unspecified
        //and there is only one stream channel and the hardware supports this feature.
        if (domain.empty() and _channels.size() == 1 and _device->getNumDirectAccessBuffers(_stream) > 0)
        {
            this->_manager = std::shared_ptr<SDRSourceBufferManager>(new SDRSourceBufferManager(_device, _stream));
            return this->_manager;
        }
        throw Pothos::PortDomainError();
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
        int ret = 0;
        int flags = 0;
        long long timeNs = 0;
        const size_t numElems = this->workInfo().minOutElements;
        if (numElems == 0) return;
        const long timeoutUs = this->workInfo().maxTimeoutNs/1000;

        //read: dispatch calls for DMA or user-provided memory
        if (_manager)
        {
            size_t handle = 0;
            const void *addrs[1]; //only 1 ch supported for now
            ret = _device->acquireReadBuffer(_stream, handle, addrs, flags, timeNs, timeoutUs);
            if (ret > 0) _manager->updateFront(handle, addrs[0]);
        }
        else
        {
            const auto &buffs = this->workInfo().outputPointers;
            ret = _device->readStream(_stream, buffs.data(), numElems, flags, timeNs, timeoutUs);
        }

        //handle error
        if (ret <= 0)
        {
            //consider this to mean that the HW produced size 0 transfer
            //the flags and time may be valid, but we are discarding here
            if (ret == 0) return this->yield();
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

        //discontinuity signaled but ok packet? post time on next call
        if ((flags & SOAPY_SDR_END_ABRUPT) != 0) _postTime = true;
    }

private:
    bool _postTime;
    std::shared_ptr<SDRSourceBufferManager> _manager;
};

static Pothos::BlockRegistry registerSDRSource(
    "/sdr/source", &SDRSource::make);
