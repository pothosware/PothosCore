// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Poco/Types.h>
#include <cstddef>

static inline size_t padUp32(const size_t len)
{
    return (len+3) & ~3;
}

static const Poco::UInt32 mVRL = 0
    | (Poco::UInt32('m') << 24)
    | (Poco::UInt32('V') << 16)
    | (Poco::UInt32('R') << 8)
    | (Poco::UInt32('L') << 0)
;

static const Poco::UInt32 VEND = 0
    | (Poco::UInt32('V') << 24)
    | (Poco::UInt32('E') << 16)
    | (Poco::UInt32('N') << 8)
    | (Poco::UInt32('D') << 0)
;

static const int VITA_SID = (1 << 28);
static const int VITA_EXT = (1 << 29);
static const int VITA_TSF = (1 << 20);

//minimum packet size given headers + footers
static const size_t MIN_PKT_BYTES = 20;

//needed to fit headers and footers
static const size_t HDR_TLR_BYTES = 8*4;

//we need a practical limit because VRL packets can be 3 MiB
static const size_t MAX_PKT_BYTES = 128*1024;
