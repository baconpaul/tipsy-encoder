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
    int minAt[3]{0, 0, 0};
    float maxValue = std::numeric_limits<float>::min();
    int maxAt[3]{0, 0, 0};

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
                REQUIRE(tipsy::isValidDataEncoding(f));
                REQUIRE(std::isfinite(f));
                REQUIRE(!std::isnan(f));
            }

    REQUIRE(!tipsy::isValidDataEncoding(tipsy::maximumEncodedFloat() + 0.001));
    REQUIRE(!tipsy::isValidDataEncoding(tipsy::minimumEncodedFloat() - 0.001));
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
                auto fb = tipsy::FloatBytes(i, j, k);
                REQUIRE(fb.first() == i);
                REQUIRE(fb.second() == j);
                REQUIRE(fb.third() == k);
                f = fb.f;
                REQUIRE(tipsy::FirstByte(f) == i);
                REQUIRE(tipsy::SecondByte(f) == j);
                REQUIRE(tipsy::ThirdByte(f) == k);
            }
}

TEST_CASE("Constructors and Utilities")
{
    SECTION("Empty CTOR")
    {
        auto f = tipsy::FloatBytes();
        REQUIRE(f.first() == 0);
        REQUIRE(f.second() == 0);
        REQUIRE(f.third() == 0);
    }

    SECTION("tipsy::threeBytesToFloat")
    {
        char d[3]{17, 28, 42};
        auto f = tipsy::threeBytesToFloat(d[0], d[1], d[2]);
        auto q = tipsy::FloatBytes(f);
        REQUIRE(q.first() == d[0]);
        REQUIRE(q.second() == d[1]);
        REQUIRE(q.third() == d[2]);
    }

    SECTION("FB 2 byte constructor")
    {
        auto f = tipsy::FloatBytes(84, 23);
        REQUIRE(f.first() == 84);
        REQUIRE(f.second() == 23);
        REQUIRE(f.third() == 0);
    }
}