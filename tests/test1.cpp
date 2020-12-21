/*
    aiss4: suffix array via induced sorting

    Copyright (c) 2020, Sebastian Wouters
    All rights reserved.

    This file is part of aiss4, licensed under the BSD 3-Clause License.
    A copy of the License can be found in the file LICENSE in the root
    folder of this project.
*/

#include "tester.hpp"


int main()
{
    const int32_t size = 6;
    const uint8_t orig[size] = { 'b', 'a', 'n', 'a', 'n', 'a' };

    bool success = aiss4::tester("banana", orig, size, true);

    return success ? 0 : 255;
}

