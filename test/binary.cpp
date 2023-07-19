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

    float minValue = std::numeric_limits<float>::max();
    int minAt[3]{0,0,0};
    float maxValue = std::numeric_limits<float>::min();
    int maxAt[3]{0,0,0};


    REQUIRE(tipsy::minimumEncodedFloat() > -5);
    REQUIRE(tipsy::maximumEncodedFloat() < 5);

    for (int i = 0; i < 256; i++)
        for (int j = 0; j < 256; j++)
            for (int k = 0; k < 256; k++)
            {
                f = tipsy::FloatBytes(i, j, k);
                if (f < minValue)
                {
                    minValue = f;
                    minAt[0] = i;
                    minAt[1] = j;
                    minAt[2] = k;
                }
                if (f > maxValue)
                {
                    maxValue = f;
                    maxAt[0] = i;
                    maxAt[1] = j;
                    maxAt[2] = k;
                }
                REQUIRE(f >= tipsy::minimumEncodedFloat());
                REQUIRE(f <= tipsy::maximumEncodedFloat());
                REQUIRE(std::isfinite(f));
                REQUIRE(!std::isnan(f));
            }

    REQUIRE(minValue == tipsy::minimumEncodedFloat());
    REQUIRE(maxValue == tipsy::maximumEncodedFloat());
    REQUIRE(minAt[0] == 255);
    REQUIRE(minAt[1] == 255);
    REQUIRE(minAt[2] == 255);
    REQUIRE(maxAt[0] == 255);
    REQUIRE(maxAt[1] == 255);
    REQUIRE(maxAt[2] == 127);
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