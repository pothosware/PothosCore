// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <Pothos/Exception.hpp>

#include <memory>
#include <string>

class MemoryMappedBufferContainer
{
public:
    using SPtr = std::shared_ptr<MemoryMappedBufferContainer>;

    static SPtr make(
        const std::string& filepath,
        bool readable,
        bool writable);

    MemoryMappedBufferContainer(
        const std::string& filepath,
        bool readable,
        bool writable);

    ~MemoryMappedBufferContainer();

    void* buffer() const;
    size_t length() const;

private:
    class Impl;

    std::unique_ptr<Impl> _implUPtr;
};
