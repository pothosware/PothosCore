// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <cstdint>
#include <cstddef>

static inline size_t padUp32(const size_t len)
{
    return (len+3) & ~3;
}

static const uint32_t mVRL = 0
    | (uint32_t('m') << 24)
    | (uint32_t('V') << 16)
    | (uint32_t('R') << 8)
    | (uint32_t('L') << 0)
;

static const uint32_t VEND = 0
    | (uint32_t('V') << 24)
    | (uint32_t('E') << 16)
    | (uint32_t('N') << 8)
    | (uint32_t('D') << 0)
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
