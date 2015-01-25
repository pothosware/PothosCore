// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <cstdint>

struct MacHeader
{
    uint16_t crc; //the crc8 checksum
    uint16_t id; //the sender's ID number
    uint16_t seq; //packet sequence number
    uint16_t bytes; //the number of payload bytes
};
