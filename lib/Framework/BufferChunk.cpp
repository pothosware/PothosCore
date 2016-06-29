// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/BufferChunk.hpp>
#include <Poco/SingletonHolder.h>
#include <cstring> //memcpy
#include <utility> //move

const Pothos::BufferChunk &Pothos::BufferChunk::null(void)
{
    static Poco::SingletonHolder<BufferChunk> sh;
    return *sh.get();
}

Pothos::BufferChunk::BufferChunk(const size_t numBytes):
    address(0),
    length(numBytes),
    _buffer(Pothos::SharedBuffer::make(numBytes)),
    _nextBuffers(0)
{
    address = _buffer.getAddress();
}

Pothos::BufferChunk::BufferChunk(const DType &dtype, const size_t numElems):
    address(0),
    length(dtype.size()*numElems),
    dtype(dtype),
    _buffer(Pothos::SharedBuffer::make(length)),
    _nextBuffers(0)
{
    address = _buffer.getAddress();
}

Pothos::BufferChunk::BufferChunk(const SharedBuffer &buffer):
    address(buffer.getAddress()),
    length(buffer.getLength()),
    _buffer(buffer),
    _nextBuffers(0)
{
    return;
}

Pothos::BufferChunk::BufferChunk(const ManagedBuffer &buffer):
    address(buffer.getBuffer().getAddress()),
    length(buffer.getBuffer().getLength()),
    _buffer(buffer.getBuffer()),
    _managedBuffer(buffer),
    _nextBuffers(0)
{
    return;
}

Pothos::BufferChunk::BufferChunk(const BufferChunk &other):
    address(other.address),
    length(other.length),
    dtype(other.dtype),
    _buffer(other._buffer),
    _managedBuffer(other._managedBuffer)
{
    _incrNextBuffers();
}

Pothos::BufferChunk::BufferChunk(BufferChunk &&other):
    address(std::move(other.address)),
    length(std::move(other.length)),
    dtype(std::move(other.dtype)),
    _buffer(std::move(other._buffer)),
    _managedBuffer(std::move(other._managedBuffer)),
    _nextBuffers(std::move(other._nextBuffers))
{
    other._nextBuffers = 0;
}

Pothos::BufferChunk::~BufferChunk(void)
{
    _decrNextBuffers();
}

Pothos::BufferChunk &Pothos::BufferChunk::operator=(const BufferChunk &other)
{
    _decrNextBuffers();
    address = other.address;
    length = other.length;
    dtype = other.dtype;
    _buffer = other._buffer;
    _managedBuffer = other._managedBuffer;
    _incrNextBuffers();
    return *this;
}

Pothos::BufferChunk &Pothos::BufferChunk::operator=(BufferChunk &&other)
{
    _decrNextBuffers();
    address = std::move(other.address);
    length = std::move(other.length);
    dtype = std::move(other.dtype);
    _buffer = std::move(other._buffer);
    _managedBuffer = std::move(other._managedBuffer);
    _nextBuffers = std::move(other._nextBuffers);
    other._nextBuffers = 0;
    return *this;
}

void Pothos::BufferChunk::append(const BufferChunk &other)
{
    //this is a null buffer, just copy a reference to other
    if (not *this)
    {
        *this = other;
        return;
    }
    //otherwise allocate and copy two buffers together
    else
    {
        Pothos::BufferChunk accumulator(this->length + other.length);
        accumulator.dtype = this->dtype;
        std::memcpy((void *)accumulator.address, (const void *)this->address, this->length);
        std::memcpy((char *)accumulator.address+this->length, (const void *)other.address, other.length);
        *this = accumulator;
    }
}

void Pothos::BufferChunk::_incrNextBuffers(void)
{
    _nextBuffers = 0;
    auto mb = _managedBuffer;
    if (not mb) return;

    int lengthRemain = this->getEnd() - _buffer.getEnd();
    while (lengthRemain > 0)
    {
        mb = mb.getNextBuffer();
        if (not mb) return;
        _nextBuffers++;
        mb._incrRef();
        lengthRemain -= mb.getBuffer().getLength();
    }
}

void Pothos::BufferChunk::_decrNextBuffers(void)
{
    auto mb = _managedBuffer;
    while (_nextBuffers != 0)
    {
        mb = mb.getNextBuffer();
        assert(mb);
        mb._decrRef();
        _nextBuffers--;
    }
}

#include <Pothos/Managed.hpp>

static auto managedBufferChunk = Pothos::ManagedClass()
    .registerConstructor<Pothos::BufferChunk>()
    .registerConstructor<Pothos::BufferChunk, const size_t>()
    .registerField(POTHOS_FCN_TUPLE(Pothos::BufferChunk, address))
    .registerField(POTHOS_FCN_TUPLE(Pothos::BufferChunk, length))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::BufferChunk, append))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::BufferChunk, elements))
    .commit("Pothos/BufferChunk");

#include <Pothos/Object/Serialize.hpp>
#include <Pothos/serialization/binary_object.hpp>
#include <Poco/Types.h>

namespace Pothos { namespace serialization {
template <class Archive>
void save(Archive & ar, const Pothos::BufferChunk &t, const unsigned int)
{
    const bool is_null = not t;
    ar << is_null;
    if (is_null) return;
    const Poco::UInt32 length = Poco::UInt32(t.length);
    ar << length;
    Pothos::serialization::binary_object bo(t.as<void *>(), t.length);
    ar << bo;
    ar << t.dtype;
}

template <class Archive>
void load(Archive & ar, Pothos::BufferChunk &t, const unsigned int)
{
    t = Pothos::BufferChunk();
    bool is_null = false;
    ar >> is_null;
    if (is_null) return;
    Poco::UInt32 length = 0;
    ar >> length;
    t = Pothos::BufferChunk(size_t(length));
    Pothos::serialization::binary_object bo(t.as<void *>(), t.length);
    ar >> bo;
    ar >> t.dtype;
}
}}

template<class Archive>
void Pothos::BufferChunk::serialize(Archive & ar, const unsigned int version)
{
    Pothos::serialization::split_free(ar, *this, version);
}

template void Pothos::BufferChunk::serialize<Pothos::archive::polymorphic_iarchive>(Pothos::archive::polymorphic_iarchive &, const unsigned int);
template void Pothos::BufferChunk::serialize<Pothos::archive::polymorphic_oarchive>(Pothos::archive::polymorphic_oarchive &, const unsigned int);

POTHOS_OBJECT_SERIALIZE(Pothos::BufferChunk)
