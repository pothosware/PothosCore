// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Config.hpp>
#include <cstddef> //size_t
#include <cstdint>

//! The exact number of bits when header fields are encoded
#define NUM_HEADER_BITS (2 + (8+12+8)*2)

//percent of sync word to declare peak found
static const double CORR_MAG_PERCENT = 0.7;
static const double CORR_DUR_PERCENT = 0.5;

/***********************************************************************
 * Simple 8-bit checksum routine
 **********************************************************************/
static inline uint8_t checksum8(const uint8_t *p, const size_t len)
{
    uint8_t acc = 0;
    for (size_t i = 0; i < len; i++)
    {
        acc = (acc >> 1) + ((acc & 0x1) << 7); //rotate
        acc += p[i]; //add
    }
    return acc;
}

/***********************************************************************
 * Struct used for encoding header fields
 * These fields are encoded after the phase sync portion
 **********************************************************************/
struct FrameHeaderFields
{
    FrameHeaderFields(void):
        id(0),
        length(0),
        chksum(0),
        error(false)
    {}

    uint8_t id;
    uint16_t length;
    uint8_t chksum;
    bool error;

    uint8_t doChecksum(void) const
    {
        uint8_t bytes[3];
        bytes[0] = id;
        bytes[1] = length & 0xff;
        bytes[2] = length >> 8;
        return checksum8(bytes, sizeof(bytes));
    }
};

/***********************************************************************
 * Encode a 4 bit word into a 8 bits with parity
 * https://en.wikipedia.org/wiki/Hamming_code
 **********************************************************************/
static inline void encodeHamming84(const unsigned char x, char *b)
{
    auto d0 = (x >> 0) & 0x1;
    auto d1 = (x >> 1) & 0x1;
    auto d2 = (x >> 2) & 0x1;
    auto d3 = (x >> 3) & 0x1;

    b[0] = (d0 + d1 + d3) & 0x1;
    b[1] = (d0 + d2 + d3) & 0x1;
    b[2] = (d0) & 0x1;
    b[3] = (d1 + d2 + d3) & 0x1;
    b[4] = (d1) & 0x1;
    b[5] = (d2) & 0x1;
    b[6] = (d3) & 0x1;
    b[7] = (d0 + d1 + d2) & 0x1;
}

/***********************************************************************
 * Decode 8 bits into a 4 bit word with single bit correction
 * Set error true when the result is known to be in error
 **********************************************************************/
static inline unsigned char decodeHamming84(const char *b, bool &error)
{
    auto b0 = b[0];
    auto b1 = b[1];
    auto b2 = b[2];
    auto b3 = b[3];
    auto b4 = b[4];
    auto b5 = b[5];
    auto b6 = b[6];
    auto b7 = b[7];

    auto p0 = (b0 + b2 + b4 + b6) & 0x1;
    auto p1 = (b1 + b2 + b5 + b6) & 0x1;
    auto p2 = (b3 + b4 + b5 + b6) & 0x1;
    auto p3 = (b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7) & 0x1;

    auto parity = (p0 << 0) | (p1 << 1) | (p2 << 2) | (p3 << 3);
    switch (parity & 0xf)
    {
    case 0: break; //no error
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7: error = true; break; //uncorrectable
    case 8: b7 = (~b7) & 0x1; break;
    case 9: b0 = (~b0) & 0x1; break;
    case 10: b1 = (~b1) & 0x1; break;
    case 11: b2 = (~b2) & 0x1; break;
    case 12: b3 = (~b3) & 0x1; break;
    case 13: b4 = (~b4) & 0x1; break;
    case 14: b5 = (~b5) & 0x1; break;
    case 15: b6 = (~b6) & 0x1; break;
    }

    return (b2 << 0) | (b4 << 1) | (b5 << 2) | (b6 << 3);
}

/***********************************************************************
 * Encode header data fields into a bit-buffer
 * The frame inserter will encode the bit-buffer as BPSK
 **********************************************************************/
static inline void encodeHeaderWord(char *bits, const FrameHeaderFields &hdr)
{
    //insert time sync
    *bits++ = 0;
    *bits++ = 1;

    //encode id
    encodeHamming84(hdr.id >> 0, bits+0);
    encodeHamming84(hdr.id >> 4, bits+8);

    //encode length
    encodeHamming84(hdr.length >> 0, bits+16);
    encodeHamming84(hdr.length >> 4, bits+24);
    encodeHamming84(hdr.length >> 8, bits+32);

    //encode checksum
    encodeHamming84(hdr.chksum >> 0, bits+40);
    encodeHamming84(hdr.chksum >> 4, bits+48);
}

/***********************************************************************
 * Decode header data fields from a bit-buffer
 * The frame sync will decode BPSK into the bit-buffer
 **********************************************************************/
static inline void decodeHeaderWord(const char *bits, FrameHeaderFields &hdr)
{
    hdr.error = false;

    //skip time sync
    bits+=2;

    //decode id
    hdr.id = 0;
    hdr.id |= uint8_t(decodeHamming84(bits+0, hdr.error)) << 0;
    hdr.id |= uint8_t(decodeHamming84(bits+8, hdr.error)) << 4;

    //decode length
    hdr.length = 0;
    hdr.length |= uint16_t(decodeHamming84(bits+16, hdr.error)) << 0;
    hdr.length |= uint16_t(decodeHamming84(bits+24, hdr.error)) << 4;
    hdr.length |= uint16_t(decodeHamming84(bits+32, hdr.error)) << 8;

    //decode checksum
    hdr.chksum = 0;
    hdr.chksum |= uint8_t(decodeHamming84(bits+40, hdr.error)) << 0;
    hdr.chksum |= uint8_t(decodeHamming84(bits+48, hdr.error)) << 4;
}
