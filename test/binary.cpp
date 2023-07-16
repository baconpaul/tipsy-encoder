/*
 * Tests for the simple binary <-> float and back. Although it is a bit expensive
 * this does test the entire 2^24 bit range.
 */

#include "catch2.hpp"
#include "tipsy/binary-to-float.h"

TEST_CASE("Binary to Float in range across all binaries")
{
    // This test can be a touch slow but works fine
    unsigned char d[3];
    float f;

    for (int i = 0; i < 256; i++)
        for (int j = 0; j < 256; j++)
            for (int k = 0; k < 256; k++)
            {
                d[0] = i;
                d[1] = j;
                d[2] = k;
                f = tipsy::threeBytesToFloat(d);
                REQUIRE(f > -10);
                REQUIRE(f < 10);
                REQUIRE(std::isfinite(f));
            }
}

TEST_CASE("Binary to Float Decodable with Fidelity")
{
    // This test can be a touch slow but works fine
    unsigned char d[3], e[3];
    float f;

    for (int i = 0; i < 256; i++)
        for (int j = 0; j < 256; j++)
            for (int k = 0; k < 256; k++)
            {
                d[0] = i;
                d[1] = j;
                d[2] = k;
                f = tipsy::threeBytesToFloat(d);
                tipsy::floatToThreeBytes(f, e);
                REQUIRE(e[0] == d[0]);
                REQUIRE(e[1] == d[1]);
                REQUIRE(e[2] == d[2]);
            }
}