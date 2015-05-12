// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SDRBlock.hpp"
#include "SDRSinkBufferManager.hpp"
#include <algorithm> //min/max

class SDRSink : public SDRBlock
{
public:
    static Block *make(const Pothos::DType &dtype, const std::vector<size_t> &channels)
    {
        return new SDRSink(dtype, channels);
    }

    SDRSink(const Pothos::DType &dtype, const std::vector<size_t> &channels):
        SDRBlock(SOAPY_SDR_TX, dtype, channels),
        _numBuffsToAcquire(0)
    {
        for (size_t i = 0; i < _channels.size(); i++) this->setupInput(i, dtype);
    }

    Pothos::BufferManager::Sptr getInputBufferManager(const std::string &, const std::string &domain)
    {
        return Pothos::BufferManager::Sptr();
        //Try to use a DMA buffer manager when the upstream domain is unspecified
        //and there is only one stream channel and the hardware supports this feature.
        void *buffs[1];
        if (domain.empty() and _channels.size() == 1 and
            _device->getNumDirectAccessBuffers(_stream) > 0 and
            _device->getDirectAccessBufferAddrs(_stream, 0, buffs) == 0
        ){
            poco_information(Poco::Logger::get("SDRSink"), "Using DMA buffer manager");
            _releasedBuffs.resize(_device->getNumDirectAccessBuffers(_stream));
            _manager = std::shared_ptr<SDRSinkBufferManager>(new SDRSinkBufferManager(_device, _stream));
            _numBuffsToAcquire = 0;
            return _manager;
        }
        return Pothos::BufferManager::Sptr();
    }

    /*******************************************************************
     * Streaming implementation
     ******************************************************************/
    void work(void)
    {
        int ret = 0;
        int flags = 0;
        long long timeNs = 0;
        size_t numElems = this->workInfo().minInElements;
        if (numElems == 0) return this->handleReleasedBuffs();

        //parse labels (from input 0)
        for (const auto &label : this->input(0)->labels())
        {
            //found a time label
            if (label.id == "txTime")
            {
                if (label.index == 0) //time for this packet
                {
                    flags |= SOAPY_SDR_HAS_TIME;
                    timeNs = label.data.convert<long long>();
                }
                else //time on the next packet
                {
                    //truncate to not include this time label
                    numElems = label.index;
                    break;
                }
            }
            //found an end label
            if (label.id == "txEnd")
            {
                flags |= SOAPY_SDR_END_BURST;
                numElems = std::min<size_t>(label.index+label.width, numElems);
                break;
            }
        }

        //write: dispatch calls for DMA or user-provided memory
        if (_manager)
        {
            const auto buff = this->input(0)->buffer().getManagedBuffer();
            _device->releaseWriteBuffer(_stream, buff.getSlabIndex(), numElems, flags, timeNs);
            _releasedBuffs[buff.getSlabIndex()] = buff;
            _numBuffsToAcquire++;
            ret = numElems;
        }
        else
        {
            const long timeoutUs = this->workInfo().maxTimeoutNs/1000;
            const auto &buffs = this->workInfo().inputPointers;
            ret = _device->writeStream(_stream, buffs.data(), numElems, flags, timeNs, timeoutUs);
        }

        //handle result
        if (ret > 0) for (auto input : this->inputs()) input->consume(size_t(ret));
        else if (ret == SOAPY_SDR_TIMEOUT) return this->yield();
        else throw Pothos::Exception("SDRSink::work()", "writeStream "+std::to_string(ret));
    }

    void handleReleasedBuffs(void)
    {
        if (_numBuffsToAcquire == 0) return;

        const long timeoutUs = this->workInfo().maxTimeoutNs/1000;
        size_t handle = 0;
        void *addrs[1];
        const int ret = _device->acquireWriteBuffer(_stream, handle, addrs, timeoutUs);
        if (ret == SOAPY_SDR_TIMEOUT) goto done;
        if (ret <= 0)
        {
            throw Pothos::Exception("SDRSink::work()", "acquireWriteBuffer "+std::to_string(ret));
        }

        _releasedBuffs[handle].reset();
        _numBuffsToAcquire--;

        done:
        if (_numBuffsToAcquire != 0) this->yield();
    }

private:
    std::shared_ptr<SDRSinkBufferManager> _manager;
    std::vector<Pothos::ManagedBuffer> _releasedBuffs;
    size_t _numBuffsToAcquire;
};

static Pothos::BlockRegistry registerSDRSink(
    "/sdr/sink", &SDRSink::make);
