/*
 * Test the protocol implementation
 */

#include "catch2.hpp"
#include "tipsy/tipsy.h"

TEST_CASE("Protocol Encode Simple String")
{
    const char *mimeType{"application/text"};
    const char *metaData{"funfun"};

    const char *message{"I am the very model of a modern major general"};

    tipsy::ProtocolEncoder pe;

    // dont forget the null terminatlr
    pe.initiateMessage(mimeType, metaData, strlen(message) + 1, (unsigned char *)message);
}