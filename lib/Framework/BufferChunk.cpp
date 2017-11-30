// Copyright (c) 2013-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/BufferChunk.hpp>
#include <cstring> //memcpy

const Pothos::BufferChunk &Pothos::BufferChunk::null(void)
{
    static BufferChunk nullChunk;
    return nullChunk;
}

void Pothos::BufferChunk::append(const BufferChunk &other)
{
    //this is a null buffer, take the input buffers type
    if (not *this)
    {
        //the other buffer is within bounds, copy the reference
        if (other.getEnd() <= other.getBuffer().getEnd())
        {
            *this = other;
            return;
        }

        //otherwise make a new buffer and copy in the contents
        *this = Pothos::BufferChunk(other.dtype, other.elements());
        std::memcpy((void *)this->address, (const void *)other.address, this->length);
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
    auto mb = _managedBuffer._impl;
    if (mb == nullptr) return;

    int lengthRemain = this->getEnd() - getBuffer().getEnd();
    while (lengthRemain > 0)
    {
        mb = mb->nextBuffer;
        if (mb == nullptr) return;
        _nextBuffers++;
        mb->incr();
        lengthRemain -= mb->buffer.getLength();
    }
}

void Pothos::BufferChunk::_decrNextBuffers(void)
{
    auto mb = _managedBuffer._impl;
    if (mb == nullptr) return;

    auto next = mb->nextBuffer;
    while (_nextBuffers != 0)
    {
        mb = next;
        assert(mb != nullptr);
        //store next before possible deletion by decr()
        next = mb->nextBuffer;
        mb->decr();
        _nextBuffers--;
    }
}

#include <Pothos/Managed.hpp>

static auto managedBufferChunk = Pothos::ManagedClass()
    .registerConstructor<Pothos::BufferChunk>()
    .registerConstructor<Pothos::BufferChunk, const size_t>()
    .registerConstructor<Pothos::BufferChunk, const Pothos::DType &, const size_t>()
    .registerField(POTHOS_FCN_TUPLE(Pothos::BufferChunk, address))
    .registerField(POTHOS_FCN_TUPLE(Pothos::BufferChunk, length))
    .registerField(POTHOS_FCN_TUPLE(Pothos::BufferChunk, dtype))
    .registerStaticMethod(POTHOS_FCN_TUPLE(Pothos::BufferChunk, null))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::BufferChunk, elements))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::BufferChunk, setElements))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::BufferChunk, getAlias))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::BufferChunk, getEnd))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::BufferChunk, unique))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::BufferChunk, useCount))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::BufferChunk, clear))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::BufferChunk, append))
    .commit("Pothos/BufferChunk");

#include <Pothos/Object/Serialize.hpp>
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
    Pothos::serialization::BinaryObject bo(t.as<void *>(), t.length);
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
    Pothos::serialization::BinaryObject bo(t.as<void *>(), t.length);
    ar >> bo;
    ar >> t.dtype;
}
}}

template<class Archive>
void Pothos::BufferChunk::serialize(Archive & ar, const unsigned int version)
{
    Pothos::serialization::invokeSplit(ar, *this, version);
}

template void Pothos::BufferChunk::serialize<Pothos::Archive::IStreamArchiver>(Pothos::Archive::IStreamArchiver &, const unsigned int);
template void Pothos::BufferChunk::serialize<Pothos::Archive::OStreamArchiver>(Pothos::Archive::OStreamArchiver &, const unsigned int);

POTHOS_OBJECT_SERIALIZE(Pothos::BufferChunk)
