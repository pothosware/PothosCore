// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <cstdlib>
#include <cstddef>

typedef enum {LSBit, MSBit} BitOrder;

/***********************************************************************
 * Pack arbitrary width symbols into bytes (MSBit order)
 **********************************************************************/
static inline void symbolsToBytesMSBit(const int mod, const unsigned char *in, unsigned char *out, const size_t numBytes)
{
    switch (mod)
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
static inline void symbolsToBytesLSBit(const int mod, const unsigned char *in, unsigned char *out, const size_t numBytes)
{
    switch (mod)
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
