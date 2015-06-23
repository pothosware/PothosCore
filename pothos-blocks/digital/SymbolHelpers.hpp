// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <cstdlib>
#include <cstddef>

typedef enum {LSBit, MSBit} BitOrder;

/***********************************************************************
 * Pack bits into arbitrary width symbols
 **********************************************************************/
static inline void bitsToSymbolsMSBit(const size_t width, const unsigned char *in, unsigned char *out, const size_t numSyms)
{
    const unsigned char sampleBit = 0x1;
    for (size_t i = 0; i < numSyms; i++)
    {
        unsigned char symbol = 0;
        for (size_t b = 0; b < width; b++)
        {
            symbol = symbol << 1;
            symbol |= (*in++ != 0) ? sampleBit : 0;
        }
        out[i] = symbol;
    }
}

static inline void bitsToSymbolsLSBit(const size_t width, const unsigned char *in, unsigned char *out, const size_t numSyms)
{
    const unsigned char sampleBit = 1 << (width - 1);
    for (size_t i = 0; i < numSyms; i++)
    {
        unsigned char symbol = 0;
        for (size_t b = 0; b < width; b++)
        {
            symbol = symbol >> 1;
            symbol |= (*in++ != 0) ? sampleBit : 0;
        }
        out[i] = symbol;
    }
}

/***********************************************************************
 * Unpack arbitrary width symbols into bits
 **********************************************************************/
static inline void symbolsToBitsMSBit(const size_t width, const unsigned char *in, unsigned char *out, const size_t numSyms)
{
    const unsigned char sampleBit = 1 << (width - 1);
    for (size_t i = 0; i < numSyms; i++)
    {
        unsigned char symbol = in[i];
        for (size_t b = 0; b < width; b++)
        {
            *out++ = ((sampleBit & symbol) != 0) ? 1 : 0;
            symbol = symbol << 1;
        }
    }
}

static inline void symbolsToBitsLSBit(const size_t width, const unsigned char *in, unsigned char *out, const size_t numSyms)
{
    const unsigned char sampleBit = 0x1;
    for (size_t i = 0; i < numSyms; i++)
    {
        unsigned char symbol = in[i];
        for (size_t b = 0; b < width; b++)
        {
            *out++ = ((sampleBit & symbol) != 0) ? 1 : 0;
            symbol = symbol >> 1;
        }
    }
}

/***********************************************************************
 * Pack arbitrary width symbols into bytes (MSBit order)
 **********************************************************************/
static inline void symbolsToBytesMSBit(const size_t width, const unsigned char *in, unsigned char *out, const size_t numBytes)
{
    switch (width)
    {
    case 1:
        for (size_t i = 0; i < numBytes;)
        {
            out[i++] = (in[0] << 7) | (in[1] << 6) | (in[2] << 5) | (in[3] << 4) | (in[4] << 3) | (in[5] << 2) | (in[6] << 1) | (in[7] << 0);
            in += 8;
        }
        break;
    case 2:
        for (size_t i = 0; i < numBytes;)
        {
            out[i++] = (in[0] << 6) | (in[1] << 4) | (in[2] << 2) | (in[3] << 0);
            in += 4;
        }
        break;
    case 3:
        for (size_t i = 0; i < numBytes;)
        {
            out[i++] = (in[0] << 5) | (in[1] << 2) | (in[2] >> 1); //1 bit remains
            out[i++] = (in[2] << 7) | (in[3] << 4) | (in[4] << 1) | (in[5] >> 2); //2 bits remain
            out[i++] = (in[5] << 6) | (in[6] << 3) | (in[7] >> 0);
            in += 8;
        }
        break;
    case 4:
        for (size_t i = 0; i < numBytes;)
        {
            out[i++] = (in[0] << 4) | (in[1]);
            in += 2;
        }
        break;
    case 5:
        for (size_t i = 0; i < numBytes;)
        {
            out[i++] = (in[0] << 3) | (in[1] >> 2); //2 bits remain
            out[i++] = (in[1] << 6) | (in[2] << 1) | (in[3] >> 4); //4 bits remain
            out[i++] = (in[3] << 4) | (in[4] >> 1); //1 bit remains
            out[i++] = (in[4] << 7) | (in[5] << 2) | (in[6] >> 3); //3 bits remain
            out[i++] = (in[6] << 5) | (in[7] >> 0);
            in += 8;
        }
        break;
    case 6:
        for (size_t i = 0; i < numBytes;)
        {
            out[i++] = (in[0] << 2) | (in[1] >> 4); //4 bits remain
            out[i++] = (in[1] << 4) | (in[2] >> 2); //2 bits remain
            out[i++] = (in[2] << 6) | (in[3] >> 0);
            in += 4;
        }
        break;
    case 7:
        for (size_t i = 0; i < numBytes;)
        {
            out[i++] = (in[0] << 1) | (in[1] >> 6); //6 bits remain
            out[i++] = (in[1] << 2) | (in[2] >> 5); //5 bits remain
            out[i++] = (in[2] << 3) | (in[3] >> 4); //4 bits remain
            out[i++] = (in[3] << 4) | (in[4] >> 3); //3 bits remain
            out[i++] = (in[4] << 5) | (in[5] >> 2); //2 bits remain
            out[i++] = (in[5] << 6) | (in[6] >> 1); //1 bit remains
            out[i++] = (in[6] << 7) | (in[7] >> 0);
            in += 8;
        }
        break;
    case 8:
        for (size_t i = 0; i < numBytes;)
        {
            out[i++] = *in++;
        }
    }
}

/***********************************************************************
 * Pack arbitrary width symbols into bytes (LSBit order)
 **********************************************************************/
static inline void symbolsToBytesLSBit(const size_t width, const unsigned char *in, unsigned char *out, const size_t numBytes)
{
    switch (width)
    {
    case 1:
        for (size_t i = 0; i < numBytes;)
        {
            out[i++] = (in[0] << 0) | (in[1] << 1) | (in[2] << 2) | (in[3] << 3) | (in[4] << 4) | (in[5] << 5) | (in[6] << 6) | (in[7] << 7);
            in += 8;
        }
        break;
    case 2:
        for (size_t i = 0; i < numBytes;)
        {
            out[i++] = (in[0] << 0) | (in[1] << 2) | (in[2] << 4) | (in[3] << 6);
            in += 4;
        }
        break;
    case 3:
        for (size_t i = 0; i < numBytes;)
        {
            out[i++] = (in[0] >> 0) | (in[1] << 3) | (in[2] << 6); //1 bit remains
            out[i++] = (in[2] >> 2) | (in[3] << 1) | (in[4] << 4) | (in[5] << 7); //2 bits remain
            out[i++] = (in[5] >> 1) | (in[6] << 2) | (in[7] << 5);
            in += 8;
        }
        break;
    case 4:
        for (size_t i = 0; i < numBytes;)
        {
            out[i++] = (in[0]) | (in[1] << 4);
            in += 2;
        }
        break;
    case 5:
        for (size_t i = 0; i < numBytes;)
        {
            out[i++] = (in[0] >> 0) | (in[1] << 5); //2 bits remain
            out[i++] = (in[1] >> 3) | (in[2] << 2) | (in[3] << 7); //4 bits remain
            out[i++] = (in[3] >> 1) | (in[4] << 4); //1 bit remains
            out[i++] = (in[4] >> 4) | (in[5] << 1) | (in[6] << 6); //3 bits remain
            out[i++] = (in[6] >> 2) | (in[7] << 3);
            in += 8;
        }
        break;
    case 6:
        for (size_t i = 0; i < numBytes;)
        {
            out[i++] = (in[0] >> 0) | (in[1] << 6); //4 bits remain
            out[i++] = (in[1] >> 2) | (in[2] << 4); //2 bits remain
            out[i++] = (in[2] >> 4) | (in[3] << 2);
            in += 4;
        }
        break;
    case 7:
        for (size_t i = 0; i < numBytes;)
        {
            out[i++] = (in[0] >> 0) | (in[1] << 7); //6 bits remain
            out[i++] = (in[1] >> 1) | (in[2] << 6); //5 bits remain
            out[i++] = (in[2] >> 2) | (in[3] << 5); //4 bits remain
            out[i++] = (in[3] >> 3) | (in[4] << 4); //3 bits remain
            out[i++] = (in[4] >> 4) | (in[5] << 3); //2 bits remain
            out[i++] = (in[5] >> 5) | (in[6] << 2); //1 bit remains
            out[i++] = (in[6] >> 6) | (in[7] << 1);
            in += 8;
        }
        break;
    case 8:
        for (size_t i = 0; i < numBytes;)
        {
            out[i++] = *in++;
        }
    }
}

/***********************************************************************
 * Unpack bytes into arbitrary width symbols (MSBit order)
 **********************************************************************/
static inline void bytesToSymbolsMSBit(const size_t width, const unsigned char *in, unsigned char *out, const size_t numBytes)
{
    switch (width)
    {
    case 1:
        for (size_t i = 0; i < numBytes; i++)
        {
            *out++ = (in[i] >> 7) & 0x1;
            *out++ = (in[i] >> 6) & 0x1;
            *out++ = (in[i] >> 5) & 0x1;
            *out++ = (in[i] >> 4) & 0x1;
            *out++ = (in[i] >> 3) & 0x1;
            *out++ = (in[i] >> 2) & 0x1;
            *out++ = (in[i] >> 1) & 0x1;
            *out++ = (in[i] >> 0) & 0x1;
        }
        break;
    case 2:
        for (size_t i = 0; i < numBytes; i++)
        {
            *out++ = (in[i] >> 6) & 0x3;
            *out++ = (in[i] >> 4) & 0x3;
            *out++ = (in[i] >> 2) & 0x3;
            *out++ = (in[i] >> 0) & 0x3;
        }
        break;
    case 3:
        for (size_t i = 0; i < numBytes; i+=3)
        {
            *out++ = (in[i+0] >> 5) & 0x7; //3 bits
            *out++ = (in[i+0] >> 2) & 0x7; //3 bits
            *out++ = ((in[i+0] << 1) & 0x7) | (in[i+1] >> 7); //2 bits, 1 bit
            *out++ = (in[i+1] >> 4) & 0x7; //3 bits
            *out++ = (in[i+1] >> 1) & 0x7; //3 bits
            *out++ = ((in[i+1] << 2) & 0x7) | (in[i+2] >> 6); //1 bit, 2 bits
            *out++ = (in[i+2] >> 3) & 0x7; //3 bits
            *out++ = (in[i+2] >> 0) & 0x7; //3 bits
        }
        break;
    case 4:
        for (size_t i = 0; i < numBytes; i++)
        {
            *out++ = (in[i] >> 4) & 0xf;
            *out++ = (in[i] >> 0) & 0xf;
        }
        break;
    case 5:
        for (size_t i = 0; i < numBytes; i+=5)
        {
            *out++ = (in[i+0] >> 3) & 0x1f; //5 bits
            *out++ = ((in[i+0] << 2) & 0x1f) | (in[i+1] >> 6); //3 bits, 2 bits
            *out++ = (in[i+1] >> 1) & 0x1f; //5 bits
            *out++ = ((in[i+1] << 4) & 0x1f) | (in[i+2] >> 4); //1 bit, 4 bits
            *out++ = ((in[i+2] << 1) & 0x1f) | (in[i+3] >> 7); //4 bits, 1 bit
            *out++ = (in[i+3] >> 2) & 0x1f; //5 bits
            *out++ = ((in[i+3] << 3) & 0x1f) | (in[i+4] >> 5); //2 bits, 3 bits
            *out++ = (in[i+4] >> 0) & 0x1f; //5 bits
        }
        break;
    case 6:
        for (size_t i = 0; i < numBytes; i+=3)
        {
            *out++ = (in[i+0] >> 2) & 0x3f; //6 bits
            *out++ = ((in[i+0] << 4) & 0x3f) | (in[i+1] >> 4); //2 bits, 4 bits
            *out++ = ((in[i+1] << 2) & 0x3f) | (in[i+2] >> 6); //4 bits, 2 bits
            *out++ = (in[i+2] >> 0) & 0x3f;
        }
        break;
    case 7:
        for (size_t i = 0; i < numBytes; i+=7)
        {
            *out++ = (in[i+0] >> 1) & 0x7f; //7 bits
            *out++ = ((in[i+0] << 6) & 0x7f) | (in[i+1] >> 2); //1 bit, 6 bits
            *out++ = ((in[i+1] << 5) & 0x7f) | (in[i+2] >> 3); //2 bits, 5 bits
            *out++ = ((in[i+2] << 4) & 0x7f) | (in[i+3] >> 4); //3 bits, 4 bits
            *out++ = ((in[i+3] << 3) & 0x7f) | (in[i+4] >> 5); //4 bits, 3 bits
            *out++ = ((in[i+4] << 2) & 0x7f) | (in[i+5] >> 6); //5 bits, 2 bits
            *out++ = ((in[i+5] << 1) & 0x7f) | (in[i+6] >> 7); //6 bits, 1 bit
            *out++ = (in[i+6] >> 0) & 0x7f; //7 bits
        }
        break;
    case 8:
        for (size_t i = 0; i < numBytes; i++)
        {
            *out++ = in[i];
        }
        break;
    }
}

/***********************************************************************
 * Unpack bytes into arbitrary width symbols (LSBit order)
 **********************************************************************/
static inline void bytesToSymbolsLSBit(const size_t width, const unsigned char *in, unsigned char *out, const size_t numBytes)
{
    switch (width)
    {
    case 1:
        for (size_t i = 0; i < numBytes; i++)
        {
            *out++ = (in[i] >> 0) & 0x1;
            *out++ = (in[i] >> 1) & 0x1;
            *out++ = (in[i] >> 2) & 0x1;
            *out++ = (in[i] >> 3) & 0x1;
            *out++ = (in[i] >> 4) & 0x1;
            *out++ = (in[i] >> 5) & 0x1;
            *out++ = (in[i] >> 6) & 0x1;
            *out++ = (in[i] >> 7) & 0x1;
        }
        break;
    case 2:
        for (size_t i = 0; i < numBytes; i++)
        {
            *out++ = (in[i] >> 0) & 0x3;
            *out++ = (in[i] >> 2) & 0x3;
            *out++ = (in[i] >> 4) & 0x3;
            *out++ = (in[i] >> 6) & 0x3;
        }
        break;
    case 3:
        for (size_t i = 0; i < numBytes; i+=3)
        {
            *out++ = (in[i+0] >> 0) & 0x7; //3 bits
            *out++ = (in[i+0] >> 3) & 0x7; //3 bits
            *out++ = (in[i+0] >> 6) | ((in[i+1] << 2) & 0x7); //2 bits, 1 bit
            *out++ = (in[i+1] >> 1) & 0x7; //3 bits
            *out++ = (in[i+1] >> 4) & 0x7; //3 bits
            *out++ = (in[i+1] >> 7) | ((in[i+2] << 1) & 0x7); //1 bit, 2 bits
            *out++ = (in[i+2] >> 2) & 0x7; //3 bits
            *out++ = (in[i+2] >> 5) & 0x7; //3 bits
        }
        break;
    case 4:
        for (size_t i = 0; i < numBytes; i++)
        {
            *out++ = (in[i] >> 0) & 0xf;
            *out++ = (in[i] >> 4) & 0xf;
        }
        break;
    case 5:
        for (size_t i = 0; i < numBytes; i+=5)
        {
            *out++ = (in[i+0] >> 0) & 0x1f; //5 bits
            *out++ = (in[i+0] >> 5) | ((in[i+1] << 3) & 0x1f); //3 bits, 2 bits
            *out++ = (in[i+1] >> 2) & 0x1f; //5 bits
            *out++ = (in[i+1] >> 7) | ((in[i+2] << 1) & 0x1f); //1 bit, 4 bits
            *out++ = (in[i+2] >> 4) | ((in[i+3] << 4) & 0x1f); //4 bits, 1 bit
            *out++ = (in[i+3] >> 1) & 0x1f; //5 bits
            *out++ = (in[i+3] >> 6) | ((in[i+4] << 2) & 0x1f); //2 bit, 3 bits
            *out++ = (in[i+4] >> 3) & 0x1f; //5 bits
        }
        break;
    case 6:
        for (size_t i = 0; i < numBytes; i+=3)
        {
            *out++ = (in[i+0] >> 0) & 0x3f; //6 bits
            *out++ = (in[i+0] >> 6) | ((in[i+1] << 2) & 0x3f); //2 bits, 4 bits
            *out++ = (in[i+1] >> 4) | ((in[i+2] << 4) & 0x3f); //4 bits, 2 bits
            *out++ = (in[i+2] >> 2) & 0x3f;
        }
        break;
    case 7:
        for (size_t i = 0; i < numBytes; i+=7)
        {
            *out++ = (in[i+0] >> 0) & 0x7f; //7 bits
            *out++ = (in[i+0] >> 7) | ((in[i+1] << 1) & 0x7f); //1 bit, 6 bits
            *out++ = (in[i+1] >> 6) | ((in[i+2] << 2) & 0x7f); //2 bits, 5 bits
            *out++ = (in[i+2] >> 5) | ((in[i+3] << 3) & 0x7f); //3 bits, 4 bits
            *out++ = (in[i+3] >> 4) | ((in[i+4] << 4) & 0x7f); //4 bits, 3 bits
            *out++ = (in[i+4] >> 3) | ((in[i+5] << 5) & 0x7f); //5 bits, 2 bits
            *out++ = (in[i+5] >> 2) | ((in[i+6] << 6) & 0x7f); //6 bits, 1 bit
            *out++ = (in[i+6] >> 1) & 0x7f; //7 bits
        }
        break;
    case 8:
        for (size_t i = 0; i < numBytes; i++)
        {
            *out++ = in[i];
        }
        break;
    }
}
