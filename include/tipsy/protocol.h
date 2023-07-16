/*
 * Given an encoder for raw binary, we need a communications protocol. This sort of
 * lays one out with a stateful encoder and decoder object.
 *
 * We have to take great care to have these be audio thread safe, so allocation and copies
 * are things we try hard not to do. Please carefully read the comments on functions to make
 * sure you get the ownership correct.
 */

#ifndef TIPSY_ENCODER_PROTOCOL_H
#define TIPSY_ENCODER_PROTOCOL_H

#include <cstdint>

#if __cplusplus >= 201703L
#define TIPSY_NODISCARD [[nodiscard]]
#else
#define TIPSY_NODISCARD
#endif

namespace tipsy
{

// We know that our 3-byte-to-binary encoder encodes numbers strictly in the range
// -10,10 so any valid float outside that range can be used as a sentinel
static constexpr float MESSAGE_BEGIN{100.f};

struct ProtocolEncoder
{
    enum EncoderState
    {
        OK
    };

    TIPSY_NODISCARD
    EncoderState initiateMessage(const char *mimeType, const char *messageMetadata,
                                 uint32_t dataBytes, const unsigned char *const data)
    {
        return OK;
    }

    TIPSY_NODISCARD
    EncoderState getNextMessageFloat(float &f);

    TIPSY_NODISCARD
    EncoderState terminateCurrentMessage();
};

struct ProtocolDecoder
{
    enum DecoderState
    {
        OK
    };

    TIPSY_NODISCARD
    DecoderState readFloat(float);
};
} // namespace tipsy

#endif // TIPSY_ENCODER_PROTOCOL_H
