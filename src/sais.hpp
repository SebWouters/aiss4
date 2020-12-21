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
#include <memory.h>

namespace aiss4
{


template <class index_t, class data_t, class idx_t>
void sais_recursion(index_t * suffix, const index_t str_size, const index_t num_lms, const index_t name, const index_t bound);


template <class token_t, class index_t>
void sais_implementation(const token_t * orig, const index_t abc_size, index_t * suffix, const index_t str_size, index_t * work1, index_t * work2)
{
    if (str_size < 2 || abc_size < 2 || orig == NULL || suffix == NULL)
    {
        if (str_size == 1)
            suffix[0] = 0;
        return;
    }

    index_t * head = work1 ? work1 : new index_t[abc_size];
    index_t * locs = work2 ? work2 : new index_t[abc_size];
    const size_t memcpy_head_size = sizeof(index_t) * static_cast<size_t>(abc_size);
    const size_t memcpy_tail_size = sizeof(index_t) * static_cast<size_t>(abc_size - 1);

    // Step 0: Compute bucket heads
    for (index_t chr = 0; chr < abc_size; ++chr){ head[chr] = 0; }
    for (index_t odx = 0; odx < str_size; ++odx){ ++head[orig[odx]]; }
    {
        index_t total = 0;
        index_t tmp;
        for (index_t chr = 0; chr < abc_size; ++chr)
        {
            tmp = head[chr];
            head[chr] = total;
            total += tmp;
        }
    }

    // Step 1:  Place the L prefix indices of LMS substrings (i.e. index of preceding L-type character) at bucket tail
    // Bucket:  [ 0 0 0 0 (L-string) | 0 0 0 0 (S-string) ] --> [ 0 0 0 0 (L-string) | 0 0 a b (S-string) ]
    //          orig[a], orig[b] are L-type prefixes of LMS substrings
    memcpy(locs, head + 1, memcpy_tail_size); locs[abc_size - 1] = str_size;
    index_t num_lms = 0;
    {
        for (index_t sdx = 0; sdx < str_size; ++sdx) { suffix[sdx] = 0; }
        index_t odx = str_size - 1; // orig     index
        token_t cur;                // current  character
        token_t prv = orig[odx--];  // previous character: orig[str_size - 1] is L-type
        while (odx > 0) // orig[0] cannot be LMS
        {
            while (odx >= 0 && (cur = orig[odx]) >= prv){ --odx; prv = cur; } // First S-type is cur = orig[odx]; prv L-type
            while (odx >= 0 && (cur = orig[odx]) <= prv){ --odx; prv = cur; } // First L-type is cur = orig[odx]; prv S-type
            if (odx >= 0)
            {
                suffix[--locs[prv]] = odx; // Index of preceding L-type character
                ++num_lms;
            }
        }
    }

    // Step 2:  Place the prefix indices of L-type characters at bucket head, retain S-type only
    // Bucket:  [ 0 0 0 0 (L-string) | 0 0 a b (S-string) ] --> [ 0 c 0 d (L-string) | 0 0 0 0 (S-string) ]
    //          orig[a], orig[b] are L-type prefixes of LMS substrings.
    //          All inductions from an LMS substring will be at larger indices,
    //              because insertion occurs at head, and L-type implies later bucket.
    //          If the L-prefix has an L-prefix: handled & reset later in the for-loop.
    //          If the L-prefix has an S-prefix: bit-flipped     later in the for-loop.
    //          orig[c], orig[d] are S-type prefixes of L-type strings.
    memcpy(locs, head, memcpy_head_size);
    {
        index_t odx = str_size - 1; // orig   index
        token_t act = orig[odx--];  // active character: orig[str_size - 1] is L-type before '$'
        token_t chk;                // check  character
        index_t * loc = suffix + locs[act]; // bucket location: speed-up w.r.t. suffix[--locs[act]]
        *loc++ = orig[odx] < act ? ~odx : odx;
        for (index_t sdx = 0; sdx < str_size; ++sdx) // suffix index
            if ((odx = suffix[sdx]) > 0) // L-type
            {
                if ((chk = orig[odx--]) != act)
                {
                    locs[act] = loc - suffix;
                    loc = suffix + locs[act = chk];
                }
                *loc++ = orig[odx] < act ? ~odx : odx; // Index preceding L-type character
                suffix[sdx] = 0; // Reset
            }
            else if (odx < 0) // S-type becomes positive
            {
                suffix[sdx] = ~odx;
            }
    }

    // Step 3:  Place the prefix indices of S-type characters at bucket tail, retain LMS only
    // Bucket:  [ 0 c 0 d (L-string) | 0 0 0 0 (S-string) ] --> [ 0 0 0 0 (S-string) | 0 e f 0 (S-string) ]
    //          orig[c], orig[d] are S-type prefixes of L-type strings
    //          All inductions from L-type strings will be at smaller indices,
    //              because insertion occurs at tail, and S-type implies earlier bucket.
    //          If the S-prefix has an S-prefix: handled & reset later in the for-loop.
    //          If the S-prefix has an L-prefix: set to LMS index (negative).
    //          orig[e], orig[f] are initial characters of LMS substrings.    
    memcpy(locs, head + 1, memcpy_tail_size); locs[abc_size - 1] = str_size;
    {
        index_t odx;     // orig index
        token_t act = 0; // active character
        token_t chk;     // check  character
        index_t * loc = suffix + locs[act]; // bucket location: speed-up w.r.t. suffix[--locs[act]]
        for (index_t sdx = str_size - 1; sdx >= 0; --sdx) // suffix index
            if ((odx = suffix[sdx]) > 0) // Check for S-type
            {
                if ((chk = orig[odx--]) != act)
                {
                    locs[act] = loc - suffix;
                    loc = suffix + locs[act = chk];
                }
                *--loc = orig[odx] > act ? ~(odx + 1) : odx; // LMS negative with start index; S-type positive
                suffix[sdx] = 0; // Reset
            }
    }

    // All non-size one LMS prefixes and sentinel are sorted in SA: compute names for reduced problem
    // Step 4: Move LMS odx to front
    {
        index_t lms = 0; // lms  index
        index_t odx;     // orig index
        // 2 * num_lms <= str_size, so while stops before lms == str_size
        while ((odx = suffix[lms]) < 0) { suffix[lms++] = ~odx; }
        if (lms < num_lms)
            for (index_t sdx = lms + 1; sdx < str_size; ++sdx) // suffix index
                if ((odx = suffix[sdx]) < 0)
                {
                    suffix[lms++] = ~odx;
                    suffix[sdx] = 0;
                }
    }

    // Step 5: Store the length of LMS substring orig[odx] at suffix[num_lms + (odx >> 1)]
    //         Character preceding or following LMS character cannot be LMS; 2 * num_lms <= str_size
    {
        index_t odx = str_size - 1; // orig     index
        index_t pdx = odx;          // previous index
        token_t cur;                // current  character
        token_t prv = orig[odx--];  // previous character: orig[str_size - 1] is L-type
        while (odx > 0) // orig[0] cannot be LMS
        {
            while (odx >= 0 && (cur = orig[odx]) >= prv){ --odx; prv = cur; } // First S-type is cur = orig[odx]; prv L-type
            while (odx >= 0 && (cur = orig[odx]) <= prv){ --odx; prv = cur; } // First L-type is cur = orig[odx]; prv S-type
            if (odx >= 0)
            {
                suffix[num_lms + ((odx + 1) >> 1)] = pdx - odx;
                pdx = odx;
            }
        }
    }

    // Step 6: Compute names
    //    - odx = suffix[sdx < num_lms] are ordered LMS substrings
    //            (only substrings - not entire suffixes - are ordered)
    //    - len = suffix[num_lms + (odx + 1) >> 1] contains length
    index_t name = 0;
    {
        index_t prv_pos = str_size;
        index_t prv_len = 0;
        index_t cur_pos;
        index_t cur_len;
        index_t odx;
        bool diff;
        for (index_t lms = 0; lms < num_lms; ++lms)
        {
            cur_pos = suffix[lms];
            cur_len = suffix[num_lms + (cur_pos >> 1)];
            diff = true;
            if (cur_len == prv_len) // Any LMS (except for '$' with length 0) has length at least two
            {
                for (odx = 0; odx < prv_len && orig[cur_pos + odx] == orig[prv_pos + odx]; ++odx) { }
                if (odx == prv_len) { diff = false; }
            }
            /*
                The above is faster than:
                    bool same = cur_len == prv_len;
                    for (index_t odx = 0; same && odx < prv_len; ++odx)
                        same = orig[cur_pos + odx] == orig[prv_pos + odx];
                    if (!same) { ... }
            */
            if (diff)
            {
                ++name;
                prv_pos = cur_pos;
                prv_len = cur_len;
            }
            suffix[num_lms + (cur_pos >> 1)] = name;
        }
    }

    // Step 7: Solve recursion problem
    //         2 * (name - 1) < 2 * num_lms <= str_size (index_t)
    //         --> data_bytes <= index_bytes <= sizeof(index_t)
    const index_t bound = str_size & static_cast<index_t>(1) ? (str_size >> 1) + 1 : (str_size >> 1);
    uint8_t index_bytes = 255;
    if (name == num_lms)
    {
        index_bytes = sizeof(index_t);
        index_t * source = suffix + num_lms;
        index_t lms = 0;
        for (index_t sdx = 0; sdx < bound; ++sdx)
            if (source[sdx] > 0)
                suffix[source[sdx] - 1] = lms++;
    }
    else
    {
        const uint8_t data_bytes = static_cast<size_t>(name - 1) <= static_cast<size_t>( UINT8_MAX) ? 1 :
                                  (static_cast<size_t>(name - 1) <= static_cast<size_t>(UINT16_MAX) ? 2 :
                                  (static_cast<size_t>(name - 1) <= static_cast<size_t>(UINT32_MAX) ? 4 : 8));
        index_bytes = static_cast<size_t>(num_lms) <= static_cast<size_t>( INT8_MAX) ? 1 :
                     (static_cast<size_t>(num_lms) <= static_cast<size_t>(INT16_MAX) ? 2 :
                     (static_cast<size_t>(num_lms) <= static_cast<size_t>(INT32_MAX) ? 4 : 8));
        if (data_bytes == 8)
        {
            // str_size > num_lms > name - 1 > UINT32_MAX > INT32_MAX: recursion index_t (for num_lms) can only be int64_t
            sais_recursion<index_t, uint64_t, int64_t>(suffix, str_size, num_lms, name, bound);
        }
        else if (data_bytes == 4)
        {
            // str_size > num_lms > name - 1 > UINT16_MAX > INT16_MAX: recursion index_t (for num_lms) can only be int32_t or int64_t
            if (index_bytes == 8)
                sais_recursion<index_t, uint32_t, int64_t>(suffix, str_size, num_lms, name, bound);
            else // index_bytes == 4
                sais_recursion<index_t, uint32_t, int32_t>(suffix, str_size, num_lms, name, bound);
        }
        else if (data_bytes == 2)
        {
            // str_size > num_lms > name - 1 > UINT8_MAX > INT8_MAX: recursion index_t (for num_lms) can only be int16_t, int32_t or int64_t
            if (index_bytes == 8)
                sais_recursion<index_t, uint16_t, int64_t>(suffix, str_size, num_lms, name, bound);
            else if (index_bytes == 4)
                sais_recursion<index_t, uint16_t, int32_t>(suffix, str_size, num_lms, name, bound);
            else // index_bytes == 2
                sais_recursion<index_t, uint16_t, int16_t>(suffix, str_size, num_lms, name, bound);
        }
        else // if (data_bytes == 1)
        {
            if (index_bytes == 8)
                sais_recursion<index_t, uint8_t, int64_t>(suffix, str_size, num_lms, name, bound);
            else if (index_bytes == 4)
                sais_recursion<index_t, uint8_t, int32_t>(suffix, str_size, num_lms, name, bound);
            else if (index_bytes == 2)
                sais_recursion<index_t, uint8_t, int16_t>(suffix, str_size, num_lms, name, bound);
            else // index_bytes == 1
                sais_recursion<index_t, uint8_t, int8_t>(suffix, str_size, num_lms, name, bound);
        }
    }

    // Step 8: Build suffix[lms] = P1[SA1[lms]]
    {
        index_t * P1 = suffix + num_lms;
        index_t odx = str_size - 1;
        index_t lms = num_lms;
        token_t cur;
        token_t prv = orig[odx--]; // orig[str_size - 1] is L-type
        while (odx > 0) // orig[0] cannot be LMS
        {
            while (odx >= 0 && (cur = orig[odx]) >= prv){ --odx; prv = cur; } // First S-type is cur = orig[odx]; prv L-type
            while (odx >= 0 && (cur = orig[odx]) <= prv){ --odx; prv = cur; } // First L-type is cur = orig[odx]; prv S-type
            if (odx >= 0)
                P1[--lms] = odx + 1;
        }
        if (index_bytes == 8)
        {
            int64_t * SA1 = reinterpret_cast<int64_t *>(suffix);
            for (index_t lms = num_lms - 1; lms >= 0; --lms)
                suffix[lms] = P1[SA1[lms]];
        }
        else if (index_bytes == 4)
        {
            int32_t * SA1 = reinterpret_cast<int32_t *>(suffix);
            for (index_t lms = num_lms - 1; lms >= 0; --lms)
                suffix[lms] = P1[SA1[lms]];
        }
        else if (index_bytes == 2)
        {
            int16_t * SA1 = reinterpret_cast<int16_t *>(suffix);
            for (index_t lms = num_lms - 1; lms >= 0; --lms)
                suffix[lms] = P1[SA1[lms]];
        }
        else // index_bytes == 1
        {
            int8_t * SA1 = reinterpret_cast<int8_t *>(suffix);
            for (index_t lms = num_lms - 1; lms >= 0; --lms)
                suffix[lms] = P1[SA1[lms]];
        }
    }

    // Step 9: Place the LMS characters at their bucket tails
    //         Assumes the LMS indices are in correct order stored in suffix[0:num_lms];
    //         they can hence only be moved to the right
    memcpy(locs, head + 1, memcpy_tail_size); locs[abc_size - 1] = str_size;
    {
        index_t lms = num_lms - 1; // lms    index
        index_t odx = suffix[lms]; // orig   index
        index_t sdx = str_size;    // suffix index
        token_t act;               // active character
        token_t chk = orig[odx];   // check  character
        index_t end;
        while (lms >= 0)
        {
            end = locs[act = chk];
            while (sdx > end) { suffix[--sdx] = 0; }
            do
            {
                suffix[--sdx] = odx;
                if (--lms < 0){ break; }
                odx = suffix[lms];
            }
            while ((chk = orig[odx]) == act);
        }
        while (sdx > 0) { suffix[--sdx] = 0; }
    }

    // Step 10:  Place the indices of L-type characters at bucket head
    // Bucket:  [ 0 0 0 0 (L-string) | 0 0 a b (S-string) ] --> [ c d e f (L-string) | 0 0 a b (S-string) ]
    // Before:  a, b are LMS substring starts.
    //          a, b > 0 because they induce L-type.
    //          Inductions will be at later indices, because insertion at head, and L-type implies later.
    // After:   orig[c:], orig[d:], orig[e:], orig[f:] are induced L-type strings.
    //          a, b, c, d, e, f < 0 if they induce L-type; a, b, c, d, e, f > 0 if they induce S-type.
    memcpy(locs, head, memcpy_head_size);
    {
        index_t odx = str_size - 1; // orig   index
        token_t act = orig[odx];    // active character: orig[str_size - 1] is L-type before '$'
        token_t chk;                // check  character
        index_t * loc = suffix + locs[act]; // bucket location: speed-up w.r.t. suffix[--locs[act]]
        *loc++ = odx > 0 && orig[odx - 1] < act ? ~odx : odx; // < 0 resp. > 0 means it induces S-type resp. L-type
        for (index_t sdx = 0; sdx < str_size; ++sdx) // suffix index
        {
            odx = suffix[sdx];
            suffix[sdx] = ~odx; // L-type becomes negative, S-type positive
            if (odx > 0) // L-type
            {
                if ((chk = orig[--odx]) != act)
                {
                    locs[act] = loc - suffix;
                    loc = suffix + locs[act = chk];
                }
                *loc++ = odx > 0 && orig[odx - 1] < act ? ~odx : odx; // L-type positive, but after later handling negative
            }
        }
    }

    // Step 11: Place the indices of S-type characters at bucket tail
    // Bucket:  [ c d e f (L-string) | 0 0 a b (S-string) ] --> [ c d e f (L-string) | g h i j (S-string) ]
    // Before:  a, b, c, d, e, f < 0 if they induce L-type; a, b, c, d, e, f > 0 if they induce S-type
    //          Inductions will be at earlier indices, because insertions at tail, and S-type implies earlier.
    // After:   all indices >= 0.
    memcpy(locs, head + 1, memcpy_tail_size); locs[abc_size - 1] = str_size;
    {
        index_t odx;     // orig   index
        token_t act = 0; // active character
        token_t chk;     // check  character
        index_t * loc = suffix + locs[act]; // bucket location: speed-up w.r.t. suffix[--locs[act]]
        for (index_t sdx = str_size - 1; sdx >= 0; --sdx) // suffix index
            if ((odx = suffix[sdx]) > 0) // S-type
            {
                if ((chk = orig[--odx]) != act)
                {
                    locs[act] = loc - suffix;
                    loc = suffix + locs[act = chk];
                }
                *--loc = odx == 0 || orig[odx - 1] > act ? ~odx : odx; // L-type negative, S-type positive
            }
            else
            {
                suffix[sdx] = ~odx; // Make L-type positive
            }
    }

    if (!work1) { delete [] head; }
    if (!work2) { delete [] locs; }
}


template <class index_t, class data_t, class idx_t>
void sais_recursion(index_t * suffix, const index_t str_size, const index_t num_lms, const index_t name, const index_t bound)
{
    int8_t * buffer  = reinterpret_cast<int8_t *>(suffix);
    size_t space_bfr = sizeof(index_t) * static_cast<size_t>(str_size);
    size_t space_sa1 = sizeof(  idx_t) * static_cast<size_t>(num_lms);
    size_t space_s1  = sizeof( data_t) * static_cast<size_t>(num_lms);
    size_t space_w12 = sizeof(  idx_t) * static_cast<size_t>(name);

    index_t * src   = suffix + num_lms;
     data_t * S1    = reinterpret_cast<data_t *>(buffer + space_sa1);
      idx_t * SA1   = reinterpret_cast< idx_t *>(buffer);
      idx_t * work1 = space_sa1 + space_s1 +     space_w12 <= space_bfr ? reinterpret_cast<idx_t *>(buffer + space_sa1 + space_s1) : NULL;
      idx_t * work2 = space_sa1 + space_s1 + 2 * space_w12 <= space_bfr ? work1 + name : NULL;

    index_t lms = 0;
    for (index_t sdx = 0; sdx < bound; ++sdx)
        if (src[sdx] > 0)
            S1[lms++] = static_cast<data_t>(src[sdx] - 1);

    sais_implementation<data_t, idx_t>(S1, static_cast<idx_t>(name), SA1, static_cast<idx_t>(num_lms), work1, work2);
}


void sais(const uint8_t * orig, int64_t * suffix, const int64_t size)
{
    sais_implementation<uint8_t, int64_t>(orig, 256, suffix, size, NULL, NULL);
}


void sais(const uint8_t * orig, int32_t * suffix, const int32_t size)
{
    sais_implementation<uint8_t, int32_t>(orig, 256, suffix, size, NULL, NULL);
}


} // End of namespace aiss4

