/*
    aiss4: suffix array via induced sorting

    Copyright (c) 2020, Sebastian Wouters
    All rights reserved.

    This file is part of aiss4, licensed under the BSD 3-Clause License.
    A copy of the License can be found in the file LICENSE in the root
    folder of this project.
*/

#pragma once

#include <stdint.h>
#include <stdlib.h>


namespace aiss4
{


void decode(const int32_t pointer, const uint8_t * encoded, uint8_t * decoded, const int32_t size)
{
    if (pointer < 0 || size < 1 || encoded == NULL || decoded == NULL)
        return;

    int32_t * map  = new int32_t[size];
    int32_t * head = new int32_t[256];
    for (int32_t sym = 0; sym < 256; ++sym)
        head[sym] = 0;

    for (int32_t idx = 0; idx < size; ++idx)
        map[idx] = head[encoded[idx]]++;
    int32_t total = 1; // Sentinel '$'
    for (int32_t sym = 0; sym < 256; ++sym)
    {
        int32_t tmp = head[sym];
        head[sym] = total;
        total += tmp;
    }

    int32_t idx = 0; // map[pointer] + head[$] = 0
    for (int32_t cnt = 0; cnt < size; ++cnt)
    {
        idx = idx < pointer ? idx : idx - 1; // Sentinel '$' not represented in encoded
        uint8_t sym = encoded[idx];
        decoded[size - 1 - cnt] = sym;
        idx = map[idx] + head[sym];
    }

    delete [] map;
    delete [] head;
}


int32_t encode(const uint8_t * orig, const int32_t * suffix, uint8_t * encoded, const int32_t size)
{
    if (size < 1 || orig == NULL || suffix == NULL || encoded == NULL)
        return -1;

    encoded[0] = orig[size - 1];
    int32_t target  =  1;
    int32_t pointer = -1;
    for (int32_t idx = 0; idx < size; ++idx)
    {
        if (suffix[idx] == 0)
            pointer = idx + 1;
        else
            encoded[target++] = orig[suffix[idx] - 1];
    }
    return pointer;
}


} // End of namespace aiss4

