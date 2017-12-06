// Copyright (c) 2013-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/BufferAccumulator.hpp>
#include <Pothos/Framework/BufferManager.hpp>
#include <Pothos/Util/RingDeque.hpp>
#include <cstring> //memcpy
#include <cassert>
#include <set>
#include <algorithm> //min/max
#include <utility> //move

/***********************************************************************
 * BufferAccumulator implementation
 **********************************************************************/
Pothos::BufferAccumulator::BufferAccumulator(void):
    _queue(64/*arbitrary*/),
    _bytesAvailable(0),
    _inPoolBuffer(false)
{
    //never let the queue become empty -- hold an empty buffer
    if (_queue.empty()) _queue.push_front(BufferChunk());
}

void Pothos::BufferAccumulator::clear(void)
{
    _queue.clear();
    _bytesAvailable = 0;
    _inPoolBuffer = false;
    _pool.clear();

    //never let the queue become empty -- hold an empty buffer
    if (_queue.empty()) _queue.push_front(BufferChunk());
}

/***********************************************************************
 * BufferAccumulator Push implementation
 **********************************************************************/
void Pothos::BufferAccumulator::push(BufferChunk &&buffer)
{
    _bytesAvailable += buffer.length;
    auto &queue = _queue;
    assert(not queue.empty());

    //prepare structures for restoreNextBuffers
    //that will be lost when the buffer is moved
    ManagedBuffer mb;
    mb._impl = buffer.getManagedBuffer()._impl;
    const auto nexts = buffer._nextBuffers;

    //remove possible dummy empty buffer from the front
    if (queue.front().length == 0) queue.pop_front();

    //Resize the queue before pushing if it full of managed buffers.
    //The implementation of set_capacity preserves the queue elements.
    if (queue.full()) queue.set_capacity(queue.size()*2);

    //push the buffer, then perform amalgamation if possible
    queue.push_back(std::move(buffer));
    const size_t backIndex = queue.size() - 1;
    if (queue.size() < 2) goto restoreNextBuffers;

    //Move contiguous chunks as far into the front buffer as possible.
    //This allows the front buffer to contain the largest contiguous section.
    for (int i = backIndex; i >= 1; i--)
    {
        BufferChunk &b = queue[i+0]; //closer to back
        BufferChunk &f = queue[i-1]; //closer to front
        assert(b);
        assert(f);
        const size_t fEnd = f.getEnd();

        if (b.address == fEnd or b.getAlias() == fEnd)
        {
            f.length += b.length;
            b.address += b.length;
            b.length = 0;
        }
        else break;
    }

    //If the back has 0 length and is the same buffer as back+1,
    //then its safe to remove the buffer entirely from the queue.
    {
        BufferChunk &b = queue[backIndex+0];
        BufferChunk &f = queue[backIndex-1];
        assert(b);
        assert(f);
        if (b.getManagedBuffer() == f.getManagedBuffer() and b.length == 0)
        {
            queue.pop_back();
        }
    }

    restoreNextBuffers:
    //this buffer may have been an upstream amalgamation
    //restore its empty next buffers in the queue as well
    //mb is a temporary container that never changes references,
    //we must set it to nullptr before exit to ensure accounting.
    for (size_t i = 0; i < nexts; i++)
    {
        mb._impl = mb._impl->nextBuffer;
        BufferChunk bnext(mb);
        bnext.length = 0;
        if (queue.full()) queue.set_capacity(queue.size()*2);
        queue.push_back(std::move(bnext));
    }
    mb._impl = nullptr;

    assert(not queue.empty());
}

/***********************************************************************
 * BufferAccumulator Pop implementation
 **********************************************************************/
void Pothos::BufferAccumulator::pop(const size_t numBytes)
{
    //remove num bytes from the total count
    assert(_bytesAvailable >= numBytes);
    _bytesAvailable -= numBytes;

    //remove num bytes from the front of the queue
    auto &queue = _queue;
    assert(not queue.empty());
    assert(queue.front().length >= numBytes);
    queue.front().address += numBytes;
    queue.front().length -= numBytes;
    const size_t queueSize = queue.size();

    //If the front buffer is from the pool,
    //and the remainder bytes are in front+1,
    //then pop the front and move into front+1.
    if (
        _inPoolBuffer and queue.size() > 1 and //pool in front
        queue.front().length <= (queue[1].address - queue[1].getBuffer().getAddress()))
    {
        queue[1].address -= queue.front().length;
        queue[1].length += queue.front().length;
        queue.pop_front();
    }

    //pop the front buffer if its now empty
    else if (queue.front().length == 0) queue.pop_front();

    //If we passed the boundary of the front buffer,
    //and the front-1 buffer is contiguous with front,
    //then we can move into the front-1 and pop front.
    else if (queue.size() > 1)
    {
        BufferChunk &f = queue[0];
        BufferChunk &b = queue[1];
        assert(b);
        assert(f);
        const bool fOverBounds = f.address >= (f.getBuffer().getEnd());
        if (fOverBounds and f.getEnd() == b.address)
        {
            b.address -= f.length;
            b.length += f.length;
            queue.pop_front();
        }
    }

    //clear the pool buffer state when the queue size shrinks
    if (_inPoolBuffer and queueSize != queue.size()) _inPoolBuffer = false;

    //pop all of the front-most consumed buffers
    while (not queue.empty() and queue.front().length == 0) queue.pop_front();

    //never let the queue become empty -- hold an empty buffer
    if (queue.empty()) queue.push_front(BufferChunk());
}

/***********************************************************************
 * BufferAccumulator Require implementation
 **********************************************************************/
void Pothos::BufferAccumulator::require(const size_t numBytes)
{
    auto &queue = _queue;
    assert(not queue.empty());

    //dont do anything if the buffer is large enough
    if (queue.front().length >= numBytes) return;

    //or the accumulator itself doesnt have enough bytes -- but can eventually have enough
    //we deduce this by checking if the requirement is larger than the actual buffer size
    //check the queue size because this optimization requires a single contiguous buffer
    if (_bytesAvailable < numBytes and queue.size() == 1 and
        numBytes <= queue.front().getBuffer().getLength()) return;

    //Actually this is ok: assert(not _inPoolBuffer);
    //The smaller pool buffer in front will be absorbed and popped.

    //get a buffer that can hold the required bytes
    auto newBuffer = _pool.get(numBytes);
    newBuffer.dtype = queue.front().dtype;
    size_t newBuffBytes = newBuffer.length;
    newBuffer.length = 0;

    //copy from the queue into the new buffer
    while (not queue.empty())
    {
        //copy the front buffer into the new buffer
        auto &f = queue.front();
        const size_t copyBytes = std::min(newBuffBytes, f.length);
        std::memcpy(
            (void *)(newBuffer.address + newBuffer.length),
            (void *)(f.address), copyBytes);
        newBuffBytes -= copyBytes;
        newBuffer.length += copyBytes;

        //buffer is drained, pop from queue
        if (f.length == copyBytes)
        {
            queue.pop_front();
        }

        //otherwise its a partial -> break
        else
        {
            assert(newBuffBytes == 0);
            f.length -= copyBytes;
            f.address += copyBytes;
            break;
        }
    }

    //finally store the new buffer to the front
    _inPoolBuffer = true;
    queue.push_front(std::move(newBuffer));
}

/***********************************************************************
 * BufferAccumulator debug methods
 **********************************************************************/
size_t Pothos::BufferAccumulator::getUniqueManagedBufferCount(void) const
{
    std::set<Pothos::ManagedBuffer> managedBufferSet;
    for (size_t i = 0; i < _queue.size(); i++)
    {
        if (not _queue[i]) continue;
        if (not _queue[i].getManagedBuffer()) continue;
        managedBufferSet.insert(_queue[i].getManagedBuffer());
    }
    return managedBufferSet.size();
}
