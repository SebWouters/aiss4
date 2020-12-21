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


template <class token_t>
void sais_basic_implementation(const token_t * orig, const int32_t abc_size, int32_t * suffix, const int32_t str_size)
{
    if (str_size < 1 || abc_size < 1 || orig == NULL || suffix == NULL)
        return;

    uint8_t * type = new uint8_t[str_size];
    int32_t * head = new int32_t[abc_size];
    int32_t * locs = new int32_t[abc_size];

    for (int32_t sym = 0; sym < abc_size; ++sym){   head[sym] =  0; }
    for (int32_t idx = 0; idx < str_size; ++idx){ suffix[idx] = -1; }

    constexpr uint8_t L_TYPE = 0;
    constexpr uint8_t S_TYPE = 1;
    constexpr uint8_t LMS_TYPE = 2; // S_TYPE if (type[idx]); L_TYPE if (!type[idx])

    // orig[str_size] = '$' is of S_TYPE
    type[str_size - 1] = L_TYPE;
    ++head[orig[str_size - 1]];
    for (int32_t idx = str_size - 2; idx >= 0; --idx)
    {
        ++head[orig[idx]];
        type[idx] = orig[idx] == orig[idx + 1] ? type[idx + 1]
                 : (orig[idx] <  orig[idx + 1] ? S_TYPE : L_TYPE);
        if (!type[idx] && type[idx + 1])
            type[idx + 1] = LMS_TYPE;
    }

    int32_t total = 0;
    for (int32_t sym = 0; sym < abc_size; ++sym)
    {
        int32_t tmp = head[sym];
        head[sym] = total;
        total += tmp;
        locs[sym] = total - 1;
    }

    // Step 1: Place the LMS characters at their bucket tails
    int32_t num_lms = 0;
    for (int32_t idx = 1; idx < str_size - 1; ++idx) // First and last characters cannot be LMS
        if (type[idx] == LMS_TYPE)
        {
            ++num_lms;
            suffix[locs[orig[idx]]--] = idx;
        }

    // Step 2: Place L_TYPE elements preceding suffix elements at bucket heads
    for (int32_t sym = 0; sym < abc_size; ++sym){ locs[sym] = head[sym]; }
    int32_t prev = str_size - 1; // First bucket in suffix is orig[str_size] == '$'; type[str_size - 1] == L_TYPE
    suffix[locs[orig[prev]]++] = prev;
    for (int32_t idx = 0; idx < str_size; ++idx)
    {
        prev = suffix[idx] - 1;
        if (prev >= 0 && !type[prev])
            suffix[locs[orig[prev]]++] = prev;
    }

    // Step 3: Place S_TYPE elements preceding suffix elements at bucket tails
    for (int32_t sym = 0; sym < abc_size - 1; ++sym){ locs[sym] = head[sym + 1] - 1; }
    locs[abc_size - 1] = str_size - 1;
    for (int32_t idx = str_size - 1; idx >= 0; --idx)
    {
        prev = suffix[idx] - 1;
        if (prev >= 0 && type[prev])
            suffix[locs[orig[prev]]--] = prev;
    }
    // First bucket in suffix is orig[str_size] == '$'; type[str_size - 1] == L_TYPE

    // All non-size one LMS prefixes and sentinel are sorted in SA: compute names for reduced problem
    int32_t * S1  = new int32_t[num_lms];
    int32_t * P1  = new int32_t[num_lms]; // into orig for order in newstring
    int32_t * SA1 = new int32_t[num_lms]; // for solving suffix array SA1 of S1

    int32_t name = -1;
    num_lms = 0;
    int32_t prev_pos = str_size;
    int32_t prev_len = 0;
    for (int32_t idx = 0; idx < str_size; ++idx)
    {
        int32_t curr_pos = suffix[idx];
        if (type[curr_pos] == LMS_TYPE)
        {
            int32_t curr_len = str_size - curr_pos;
            for (int32_t run = curr_pos + 2; run < str_size - 1; ++run)
                if (type[run] == LMS_TYPE)
                {
                    curr_len = run - curr_pos;
                    break;
                }

            bool identical = curr_len == prev_len;
            // Last element of LMS is L-type. If all elements in LMS the same, types are the same as well.
            for (int32_t count = 0; identical && count < curr_len; ++count)
                identical = orig[prev_pos + count] == orig[curr_pos + count];

            S1[num_lms] = identical ? name : ++name;
            P1[num_lms++] = curr_pos;

            prev_pos = curr_pos;
            prev_len = curr_len;
        }
    }

    if (++name == num_lms)
    {
        // Each name is unique and indicates position in SA1: bucket sort
        for (int32_t idx = 0; idx < num_lms; ++idx)
            SA1[S1[idx]] = idx;
    }
    else
    {
        for (int32_t idx = 0; idx < str_size; ++idx){ suffix[idx] = -1; }
        for (int32_t idx = 0; idx < num_lms;  ++idx){ suffix[P1[idx]] = S1[idx]; }
        num_lms = 0;
        for (int32_t idx = 0; idx < str_size; ++idx)
        {
            if (suffix[idx] >= 0)
            {
                S1[num_lms] = suffix[idx];
                P1[num_lms++] = idx;
            }
        }
        sais_basic_implementation<int32_t>(S1, name, SA1, num_lms);
    }

    // Step 1: Place the LMS characters at their bucket tails
    for (int32_t idx = 0; idx < str_size;     ++idx){ suffix[idx] = -1; }
    for (int32_t sym = 0; sym < abc_size - 1; ++sym){   locs[sym] = head[sym + 1] - 1; }
    locs[abc_size - 1] = str_size - 1;
    for (int32_t idx = num_lms - 1; idx >= 0; --idx)
    {
        int32_t idx_orig = P1[SA1[idx]];
        suffix[locs[orig[idx_orig]]--] = idx_orig;
    }

    // Step 2: Place L_TYPE elements preceding suffix elements at bucket heads
    for (int32_t sym = 0; sym < abc_size; ++sym){ locs[sym] = head[sym]; }
    prev = str_size - 1; // First bucket in suffix is orig[str_size] == '$'; type[str_size - 1] == L_TYPE
    suffix[locs[orig[prev]]++] = prev;
    for (int32_t idx = 0; idx < str_size; ++idx)
    {
        prev = suffix[idx] - 1;
        if (prev >= 0 && !type[prev])
            suffix[locs[orig[prev]]++] = prev;
    }

    // Step 3: Place S_TYPE elements preceding suffix elements at bucket tails
    for (int32_t sym = 0; sym < abc_size - 1; ++sym){ locs[sym] = head[sym + 1] - 1; }
    locs[abc_size - 1] = str_size - 1;
    for (int32_t idx = str_size - 1; idx >= 0; --idx)
    {
        prev = suffix[idx] - 1;
        if (prev >= 0 && type[prev])
            suffix[locs[orig[prev]]--] = prev;
    }
    // First bucket in suffix is orig[str_size] == '$'; type[str_size - 1] == L_TYPE

    delete [] S1;
    delete [] P1;
    delete [] SA1;

    delete [] head;
    delete [] locs;
    delete [] type;
}


void sais_basic(const uint8_t * orig, int32_t * suffix, const int32_t size)
{
    sais_basic_implementation<uint8_t>(orig, 256, suffix, size);
}


} // End of namespace aiss4

