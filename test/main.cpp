#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2.hpp"

#include "tipsy/tipsy.h"

TEST_CASE("Catch is Installed") { REQUIRE(true); }

TEST_CASE("Version is reachable with tipsy.h")
{
    REQUIRE(tipsy::TIPSY_VERSION_MAJOR >= 0);
    REQUIRE(tipsy::TIPSY_VERSION_MINOR >= 0);
    REQUIRE(tipsy::TIPSY_VERSION_RELEASE >= 0);
}