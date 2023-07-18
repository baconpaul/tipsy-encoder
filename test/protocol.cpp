/*
 * Test the protocol implementation
 */

#include "catch2.hpp"
#include "tipsy/tipsy.h"

#include <cstring>
#include <iostream>

TEST_CASE("Protocol Encode Simple String")
{
    INFO("FIXME - make this test assert state transitions");
    const char *mimeType{"application/text"};

    const char *message{"I am the very model of a modern major general"};

    tipsy::ProtocolEncoder pe;

    // dont forget the null termination
    auto status = pe.initiateMessage(mimeType, strlen(message) + 1, (unsigned char *)message);
    REQUIRE(status == tipsy::ProtocolEncoder::MESSAGE_INITIATED);
    bool done{false};
    for (int i = 0; i < 50; ++i)
    {
        float nf;
        auto st = pe.getNextMessageFloat(nf);

        if (done)
        {
            REQUIRE(st == tipsy::ProtocolEncoder::DORMANT);
        }
        else
        {
            REQUIRE(((st == tipsy::ProtocolEncoder::ENCODING_MESSAGE) ||
                     (st == tipsy::ProtocolEncoder::MESSAGE_COMPLETE)));
        }
        if (st == tipsy::ProtocolEncoder::MESSAGE_COMPLETE)
        {
            done = true;
        }
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
    REQUIRE(status == tipsy::ProtocolEncoder::MESSAGE_INITIATED);
    bool gotHeader{false}, gotBody{false};
    for (int i = 0; i < 50; ++i)
    {
        float nf;
        auto st = pe.getNextMessageFloat(nf);
        auto rf = pd.readFloat(nf);

        REQUIRE(!pd.isError(rf));

        if (gotBody && gotHeader)
        {
            REQUIRE(rf == tipsy::ProtocolDecoder::DORMANT);
        }
        if (rf == tipsy::ProtocolDecoder::HEADER_READY)
        {
            REQUIRE(std::string(pd.getMimeType()) == std::string(mimeType));
            gotHeader = true;
        }
        if (rf == tipsy::ProtocolDecoder::BODY_READY)
        {
            REQUIRE(std::string((const char *)buffer) == std::string(message));
            gotBody = true;
        }
    }
}