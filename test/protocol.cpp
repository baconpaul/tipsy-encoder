/*
 * Test the protocol implementation
 */

#include "catch2.hpp"
#include "tipsy/tipsy.h"

#include <cstring>
#include <iostream>

TEST_CASE("Protocol Encode Simple String")
{
    const char *mimeType{"application/text"};

    const char *message{"I am the very model of a modern major general"};

    tipsy::ProtocolEncoder pe;

    // dont forget the null terminatlr
    auto status = pe.initiateMessage(mimeType, strlen(message) + 1, (unsigned char *)message);
    assert(status == tipsy::ProtocolEncoder::OK);
    for (int i = 0; i < 50; ++i)
    {
        float nf;
        auto st = pe.getNextMessageFloat(nf);
        std::cout << st << " " << nf;
        if (nf <= 10 && nf >= -10)
        {
            unsigned char d[3];
            tipsy::floatToThreeBytes(nf, d);
            for (int q = 0; q < 3; ++q)
                std::cout << " " << (int)d[q];
        }
        std::cout << std::endl;
    }
}

TEST_CASE("Encode Decode String")
{
    const char *mimeType{"application/text"};

    const char *message{"I am the very model of a modern major general"};

    unsigned char buffer[2048];

    tipsy::ProtocolEncoder pe;
    tipsy::ProtocolDecoder pd;
    pd.provideDataBuffer(buffer, 2048);

    // dont forget the null terminate
    auto status = pe.initiateMessage(mimeType, strlen(message) + 1, (unsigned char *)message);
    assert(status == tipsy::ProtocolEncoder::OK);
    for (int i = 0; i < 50; ++i)
    {
        float nf;
        auto st = pe.getNextMessageFloat(nf);
        auto rf = pd.readFloat(nf);
    }
    REQUIRE(std::string(pd.getMimeType()) == std::string(mimeType));
    REQUIRE(std::string((const char *)buffer) == std::string(message));
}