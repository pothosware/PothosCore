// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Framework/BufferManager.hpp>
#include <Pothos/Framework/ManagedBuffer.hpp>
#include <Pothos/Framework/BufferChunk.hpp>
#include <Pothos/Framework/Label.hpp>
#include <Pothos/Object/Object.hpp>
#include <Pothos/Exception.hpp>
#include <string>
#include <vector>

/***********************************************************************
 * Messages that will be sent to the actor
 **********************************************************************/
template <typename PortIdType, typename MessageType>
struct PortMessage
{
    PortIdType id;
    MessageType contents;
};

template <typename PortIdType, typename MessageType>
PortMessage<PortIdType, MessageType> makePortMessage(const PortIdType &id, const MessageType &contents)
{
    PortMessage<PortIdType, MessageType> message;
    message.id = id;
    message.contents = contents;
    return message;
}

struct LabeledBuffersMessage
{
    std::vector<Pothos::Label> labels;
    std::vector<Pothos::BufferChunk> buffers;
};

struct TokenizedAsyncMessage
{
    Pothos::ManagedBuffer token;
    Pothos::Object async;
};

struct BufferManagerMessage
{
    Pothos::BufferManager::Sptr manager;
};

struct BufferReturnMessage
{
    Pothos::ManagedBuffer buff;
};

struct BumpWorkMessage
{
    //
};

struct ActivateWorkMessage
{
    //
};

struct DeactivateWorkMessage
{
    //
};

struct ShutdownActorMessage
{
    //
};

struct RequestPortInfoMessage
{
    bool isInput;
    std::string name;
};

struct RequestWorkerStatsMessage
{
    //
};

struct OpaqueCallMessage
{
    std::string name;
    const Pothos::Object *inputArgs;
    size_t numArgs;
};

struct OpaqueCallResultMessage
{
    Pothos::Object obj;
    std::shared_ptr<Pothos::Exception> error;
};
