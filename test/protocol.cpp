/*
 * Test the protocol implementation
 */

#include "catch2.hpp"
#include "tipsy/tipsy.h"

#include <cstring>
#include <iostream>

TEST_CASE("Sentinels In Bound")
{
    REQUIRE(tipsy::kMessageBeginSentinel < 10);
    REQUIRE(tipsy::kMessageBeginSentinel > tipsy::maximumEncodedFloat());
    REQUIRE(tipsy::kVersionSentinel < 10);
    REQUIRE(tipsy::kVersionSentinel > tipsy::maximumEncodedFloat());
    REQUIRE(tipsy::kSizeSentinel < 10);
    REQUIRE(tipsy::kSizeSentinel > tipsy::maximumEncodedFloat());
    REQUIRE(tipsy::kMimeTypeSentinel < 10);
    REQUIRE(tipsy::kMimeTypeSentinel > tipsy::maximumEncodedFloat());
    REQUIRE(tipsy::kBodySentinel < 10);
    REQUIRE(tipsy::kBodySentinel > tipsy::maximumEncodedFloat());
    REQUIRE(tipsy::kEndMessageSentinel < 10);
    REQUIRE(tipsy::kEndMessageSentinel > tipsy::maximumEncodedFloat());
}

TEST_CASE("Protocol Encode Simple String")
{
    INFO("FIXME - make this test assert state transitions");
    const char *mimeType{"application/text"};
    const char *message{"I am the very model of a modern major general"};

    tipsy::ProtocolEncoder pe;

    // don't forget null termination
    auto status = pe.initiateMessage(mimeType, strlen(message) + 1, (unsigned char *)message);
    REQUIRE(status == tipsy::EncoderResult::MESSAGE_INITIATED);
    bool done{false};
    for (int i = 0; i < 50; ++i)
    {
        float nf;
        auto st = pe.getNextMessageFloat(nf);

        if (done)
        {
            REQUIRE(st == tipsy::EncoderResult::DORMANT);
        }
        else
        {
            REQUIRE(((st == tipsy::EncoderResult::ENCODING_MESSAGE) ||
                     (st == tipsy::EncoderResult::MESSAGE_COMPLETE)));
        }
        if (st == tipsy::EncoderResult::MESSAGE_COMPLETE)
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

    // don't forget null termination
    auto status = pe.initiateMessage(mimeType, strlen(message) + 1, (const unsigned char *)message);
    REQUIRE(status == tipsy::EncoderResult::MESSAGE_INITIATED);
    bool gotHeader{false}, gotBody{false};
    for (int i = 0; i < 50; ++i)
    {
        float nf;
        auto st = pe.getNextMessageFloat(nf);
        REQUIRE(!pe.isError(st));
        REQUIRE(nf < 10);
        REQUIRE(nf > -10);
        REQUIRE(tipsy::isValidProtocolEncoding(nf));
        auto rf = pd.readFloat(nf);

        REQUIRE(!tipsy::ProtocolDecoder::isError(rf));

        if (gotBody && gotHeader)
        {
            REQUIRE(rf == tipsy::DecoderResult::DORMANT);
        }
        if (rf == tipsy::DecoderResult::HEADER_READY)
        {
            REQUIRE(std::string(pd.getMimeType()) == std::string(mimeType));
            gotHeader = true;
        }
        if (rf == tipsy::DecoderResult::BODY_READY)
        {
            REQUIRE(std::string((const char *)buffer) == std::string(message));
            gotBody = true;
        }
    }
}

TEST_CASE("Require a MIME type")
{
    tipsy::ProtocolEncoder pe;
    auto status = pe.initiateMessage(nullptr, 0, (const unsigned char *)"");
    REQUIRE(status == tipsy::EncoderResult::ERROR_MISSING_MIME_TYPE);
}

TEST_CASE("Buffer too small")
{
    const char *mimeType{"application/text"};
    const char *message{"I am the very model of a modern major general"};

    unsigned char buffer[20];

    tipsy::ProtocolEncoder pe;
    tipsy::ProtocolDecoder pd;
    pd.provideDataBuffer(buffer, 20);

    // don't forget the terminating null
    auto status = pe.initiateMessage(mimeType, strlen(message) + 1, (const unsigned char *)message);
    REQUIRE(status == tipsy::EncoderResult::MESSAGE_INITIATED);
    bool gotHeader{false}, gotBody{false};
    for (int i = 0; i < 50; ++i)
    {
        float nf;
        auto st = pe.getNextMessageFloat(nf);
        REQUIRE(!pe.isError(st));
        auto rf = pd.readFloat(nf);

        //  we expect this error when the message exceeds the buffer
        if (rf == tipsy::DecoderResult::ERROR_DATA_TOO_LARGE)
        {
            return;
        }
        REQUIRE(!tipsy::ProtocolDecoder::isError(rf));

        if (gotBody && gotHeader)
        {
            REQUIRE(rf == tipsy::DecoderResult::DORMANT);
        }
        if (rf == tipsy::DecoderResult::HEADER_READY)
        {
            REQUIRE(std::string(pd.getMimeType()) == std::string(mimeType));
            gotHeader = true;
        }
        if (rf == tipsy::DecoderResult::BODY_READY)
        {
            REQUIRE(std::string((const char *)buffer) == std::string(message));
            gotBody = true;
        }
    }
}

TEST_CASE("Encode Decode Empty Message")
{
    const char *mimeType{"application/text"};
    const char *message{""};

    unsigned char buffer[2048];

    tipsy::ProtocolEncoder pe;
    tipsy::ProtocolDecoder pd;
    pd.provideDataBuffer(buffer, 2048);

    // don't forget the terminating null
    auto status = pe.initiateMessage(mimeType, strlen(message) + 1, (const unsigned char *)message);
    REQUIRE(status == tipsy::EncoderResult::MESSAGE_INITIATED);
    bool gotHeader{false}, gotBody{false};
    for (int i = 0; i < 50; ++i)
    {
        float nf;
        auto st = pe.getNextMessageFloat(nf);
        REQUIRE(!pe.isError(st));
        auto rf = pd.readFloat(nf);

        REQUIRE(!tipsy::ProtocolDecoder::isError(rf));

        if (gotBody && gotHeader)
        {
            REQUIRE(rf == tipsy::DecoderResult::DORMANT);
        }
        if (rf == tipsy::DecoderResult::HEADER_READY)
        {
            REQUIRE(std::string(pd.getMimeType()) == std::string(mimeType));
            gotHeader = true;
        }
        if (rf == tipsy::DecoderResult::BODY_READY)
        {
            REQUIRE(0 == buffer[0]);
            gotBody = true;
        }
    }
}

TEST_CASE("Test Message Sizes 0 through buffer size")
{
    static constexpr int maxBufferSz{2048};

    static const char *mt{"test/type"};

    unsigned char outB[maxBufferSz], inB[maxBufferSz];
    for (auto bufferSz : {127, 128, 129, 254, 255, 256})
    {
        for (int bs = 0; bs < bufferSz; ++bs)
        {
            DYNAMIC_SECTION("dataSize " << bufferSz << " message Size " << bs)
            {
                memset(inB, 0, sizeof(inB));
                memset(outB, 0, sizeof(outB));

                for (int j = 0; j < bs; ++j)
                {
                    inB[j] = (((unsigned char)j) & 255);
                }

                tipsy::ProtocolEncoder pe;
                tipsy::ProtocolDecoder pd;
                pd.provideDataBuffer(outB, bufferSz);

                // don't forget the terminating null
                auto status = pe.initiateMessage(mt, bs, inB);
                REQUIRE(status == tipsy::EncoderResult::MESSAGE_INITIATED);
                bool gotHeader{false}, gotBody{false};
                for (int i = 0; i < 1000 && !gotBody; ++i)
                {
                    float nf;
                    auto st = pe.getNextMessageFloat(nf);
                    auto rf = pd.readFloat(nf);

                    auto tf = tipsy::FloatBytes(nf);

                    REQUIRE(!tipsy::ProtocolDecoder::isError(rf));

                    if (gotBody && gotHeader)
                    {
                        REQUIRE(rf == tipsy::DecoderResult::DORMANT);
                    }
                    if (rf == tipsy::DecoderResult::HEADER_READY)
                    {
                        REQUIRE(std::string(pd.getMimeType()) == std::string(mt));
                        gotHeader = true;
                    }
                    if (rf == tipsy::DecoderResult::BODY_READY)
                    {
                        REQUIRE(pd.getDataSize() == bs);
                        for (int j = 0; j < bs; ++j)
                        {
                            INFO("Testing at " << j);
                            REQUIRE(outB[j] == inB[j]);
                        }
                        gotBody = true;
                    }
                }
                REQUIRE(gotBody);
            }
        }
    }
}

TEST_CASE("Mime Type Size 0 - 20")
{
    char mimeTypeBuffer[128];
    const char *message{"I am the very model of a modern major general"};

    unsigned char buffer[2048];

    for (int mts = 0; mts <= 20; ++mts)
    {
        for (int k = 0; k < mts; ++k)
            mimeTypeBuffer[k] = (char)('A' + k);
        mimeTypeBuffer[mts] = 0;

        DYNAMIC_SECTION("Mime Type Size = " << mts << " mimetype = [" << mimeTypeBuffer << "]")
        {
            tipsy::ProtocolEncoder pe;
            tipsy::ProtocolDecoder pd;
            pd.provideDataBuffer(buffer, 2048);

            // don't forget null termination
            auto status = pe.initiateMessage(mimeTypeBuffer, strlen(message) + 1,
                                             (const unsigned char *)message);
            REQUIRE(status == tipsy::EncoderResult::MESSAGE_INITIATED);
            bool gotHeader{false}, gotBody{false};
            for (int i = 0; i < 50; ++i)
            {
                float nf;
                auto st = pe.getNextMessageFloat(nf);
                REQUIRE(!pe.isError(st));
                REQUIRE(nf < 10);
                REQUIRE(nf > -10);
                REQUIRE(tipsy::isValidProtocolEncoding(nf));
                auto rf = pd.readFloat(nf);

                REQUIRE(!tipsy::ProtocolDecoder::isError(rf));

                if (gotBody && gotHeader)
                {
                    REQUIRE(rf == tipsy::DecoderResult::DORMANT);
                }
                if (rf == tipsy::DecoderResult::HEADER_READY)
                {
                    REQUIRE(std::string(pd.getMimeType()) == std::string(mimeTypeBuffer));
                    gotHeader = true;
                }
                if (rf == tipsy::DecoderResult::BODY_READY)
                {
                    REQUIRE(std::string((const char *)buffer) == std::string(message));
                    gotBody = true;
                }
            }
            REQUIRE(gotBody);
            REQUIRE(gotHeader);
        }
    }
}

TEST_CASE("Sentinel Display Name")
{
#define CK(s) REQUIRE(std::string("tipsy::") + tipsy::sentinelDisplayName(s) == #s);
    CK(tipsy::kMessageBeginSentinel);
    CK(tipsy::kVersionSentinel);
    CK(tipsy::kSizeSentinel);
    CK(tipsy::kMimeTypeSentinel);
    CK(tipsy::kBodySentinel);
    CK(tipsy::kEndMessageSentinel);

    REQUIRE(tipsy::sentinelDisplayName(0.42) == "NOT_A_SENTINEL");
#undef CK
}

TEST_CASE("Encoder Error Cases")
{
    SECTION("Null Data")
    {
        tipsy::ProtocolEncoder pe;
        auto r = pe.initiateMessage("tst", 1, nullptr);
        REQUIRE(pe.isError(r));
        REQUIRE(r == tipsy::EncoderResult::ERROR_MISSING_DATA);
    }

    SECTION("Too Much Purported Data")
    {
        tipsy::ProtocolEncoder pe;
        unsigned char x[1];
        auto r = pe.initiateMessage("tst", tipsy::kMaxMessageLength + 1, x);
        REQUIRE(pe.isError(r));
        REQUIRE(r == tipsy::EncoderResult::ERROR_MESSAGE_TOO_LARGE);
    }

    SECTION("Too Big a Mime Type")
    {
        std::string mt = "hello there";
        for (int i = 0; i < 5; ++i)
            mt += mt;
        tipsy::ProtocolEncoder pe;
        unsigned char x[1024];
        auto r = pe.initiateMessage(mt.c_str(), 1024, x);
        REQUIRE(pe.isError(r));
        REQUIRE(r == tipsy::EncoderResult::ERROR_MIME_TYPE_TOO_LARGE);
    }

    SECTION("Restart while running")
    {
        const char *mimeType{"application/text"};
        const char *message{"I am the very model of a modern major general"};

        tipsy::ProtocolEncoder pe;

        // don't forget null termination
        auto status = pe.initiateMessage(mimeType, strlen(message) + 1, (unsigned char *)message);
        REQUIRE(status == tipsy::EncoderResult::MESSAGE_INITIATED);
        bool done{false};
        for (int i = 0; i < 3; ++i)
        {
            float nf;
            auto st = pe.getNextMessageFloat(nf);
            REQUIRE(!pe.isError(st));
        }

        status = pe.initiateMessage(mimeType, strlen(message) + 1, (unsigned char *)message);
        REQUIRE(pe.isError(status));
        REQUIRE(status == tipsy::EncoderResult::ERROR_MESSAGE_ALREADY_ACTIVE);
    }

    SECTION("Terminate when Running")
    {
        const char *mimeType{"application/text"};
        const char *message{"I am the very model of a modern major general"};

        tipsy::ProtocolEncoder pe;

        // don't forget null termination
        auto status = pe.initiateMessage(mimeType, strlen(message) + 1, (unsigned char *)message);
        REQUIRE(status == tipsy::EncoderResult::MESSAGE_INITIATED);
        bool done{false};
        for (int i = 0; i < 3; ++i)
        {
            float nf;
            auto st = pe.getNextMessageFloat(nf);
            REQUIRE(!pe.isError(st));
        }

        status = pe.terminateCurrentMessage();
        REQUIRE(!pe.isError(status));
        REQUIRE(status == tipsy::EncoderResult::MESSAGE_TERMINATED);
    }

    SECTION("Terminate before Running")
    {
        const char *mimeType{"application/text"};
        const char *message{"I am the very model of a modern major general"};

        tipsy::ProtocolEncoder pe;

        auto status = pe.terminateCurrentMessage();
        REQUIRE(pe.isError(status));
        REQUIRE(status == tipsy::EncoderResult::ERROR_NO_MESSAGE_ACTIVE);
    }
}