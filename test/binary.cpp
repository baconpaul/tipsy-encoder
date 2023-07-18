/*
 * Tests for the simple binary <-> float and back. Although it is a bit expensive
 * this does test the entire 2^24 bit range.
 */

#include "catch2.hpp"
#include "tipsy/binary-to-float.h"

TEST_CASE("Binary to Float in range across all binaries")
{
    // This test can be a touch slow but works fine
    float f;

    for (int i = 0; i < 256; i++)
        for (int j = 0; j < 256; j++)
            for (int k = 0; k < 256; k++)
            {
                f = tipsy::FloatBytes(i, j, k);
                REQUIRE(f > -10.f);
                REQUIRE(f < 10.f);
                REQUIRE(std::isfinite(f));
                REQUIRE(!std::isnan(f));
            }
}

TEST_CASE("Binary to Float Decodable with Fidelity")
{
    // This test can be a touch slow but works fine
    float f;

    for (int i = 0; i < 256; i++)
        for (int j = 0; j < 256; j++)
            for (int k = 0; k < 256; k++)
            {
                auto fb = tipsy::FloatBytes(i,j,k);
                REQUIRE(fb.first() == i);
                REQUIRE(fb.second() == j);
                REQUIRE(fb.third() == k);
                f = fb.f;
                REQUIRE(tipsy::FirstByte(f) == i);
                REQUIRE(tipsy::SecondByte(f) == j);
                REQUIRE(tipsy::ThirdByte(f) == k);
            }
}