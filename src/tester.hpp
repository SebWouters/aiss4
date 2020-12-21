/*
    aiss4: suffix array via induced sorting

    Copyright (c) 2020, Sebastian Wouters
    All rights reserved.

    This file is part of aiss4, licensed under the BSD 3-Clause License.
    A copy of the License can be found in the file LICENSE in the root
    folder of this project.
*/

#pragma once

#include "bwt.hpp"
#include "sais.hpp"

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <algorithm>
#include <cassert>


namespace aiss4
{


/*
    O(n^2 * log(n)), with n = str_size
*/
void quicksort(const uint8_t * orig, int32_t * suffix, const int32_t str_size)
{
    for (int32_t idx = 0; idx < str_size; ++idx)
        suffix[idx] = idx;

    std::sort(suffix, suffix + str_size, [str_size, &orig](int32_t left, int32_t right)
        {
            const int32_t limit = str_size - std::max(left, right);
            for (int32_t cnt = 0; cnt < limit; ++cnt)
            {
                uint8_t l = orig[left  + cnt];
                uint8_t r = orig[right + cnt];
                if (l != r)
                    return l < r;
            }
            return left >= right; // if left > right, left terminates first, so '$' character encountered first, so left smaller
        });
}


bool tester(const std::string name, const uint8_t * orig, const int32_t str_size, const bool run_qsort)
{
    std::cout << "Test " << name << std::endl;

    uint8_t  * encoded = new uint8_t[str_size];
    uint8_t  * decoded = new uint8_t[str_size];
    int32_t  * SA1     = run_qsort ? new int32_t[str_size] : NULL;
    int32_t  * SA2     = new int32_t[str_size];

    // Q-sort: own implementation
    int32_t pointer1;
    if (run_qsort)
    {
        auto start = std::chrono::system_clock::now();
        quicksort(orig, SA1, str_size);
        pointer1 = encode(orig, SA1, encoded, str_size);
        auto end = std::chrono::system_clock::now();
        double time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() * 1e-6;
        std::cout << "Time [ms] Q-sort (size = " << str_size << ") = " << time << std::endl;
    }

    // SA-IS
    auto start = std::chrono::system_clock::now();
    sais(orig, SA2, str_size);
    const int32_t pointer2 = encode(orig, SA2, encoded, str_size);
    auto end = std::chrono::system_clock::now();
    double time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() * 1e-6;
    std::cout << "Time [ms] SA-IS  (size = " << str_size << ") = " << time << std::endl;

    bool same = true;
    if (run_qsort)
    {
        same = same && pointer1 == pointer2;
        for (int32_t sdx = 0; same && sdx < str_size; ++sdx)
            same = same && SA1[sdx] == SA2[sdx];
    }

    // Decode
    start = std::chrono::system_clock::now();
    decode(pointer2, encoded, decoded, str_size);
    end = std::chrono::system_clock::now();
    time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() * 1e-6;
    std::cout << "Time [ms] decode (size = " << str_size << ") = " << time << std::endl;

    for (int32_t odx = 0; same && odx < str_size; ++odx)
        same = same && decoded[odx] == orig[odx];

    delete [] encoded;
    delete [] decoded;
    if (run_qsort){ delete [] SA1; }
    delete [] SA2;

    std::cout << "Test " << name << (same ? " success!" : " fail!") << std::endl;
    return same;
}


} // End of namespace aiss4

