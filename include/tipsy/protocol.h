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
#include <cstring>

#if __cplusplus >= 201703L
#define TIPSY_NODISCARD [[nodiscard]]
#else
#define TIPSY_NODISCARD
#endif

namespace tipsy
{

// We know that our 3-byte-to-binary encoder encodes numbers strictly in the range
// -10,10 so any valid float outside that range can be used as a sentinel
static constexpr float kMessageBeginSentinel{11.f};
static constexpr float kVersionSentinel{12.f};
static constexpr float kSizeSentinel{13.f};
static constexpr float kMimeTypeSentinel{14.f};
static constexpr float kBodySentinel{15.f};
static constexpr float kEndMessageSentinel{16.f};
static constexpr uint16_t kVersion{0x01};
static constexpr size_t kMaxMimeTypeSize{1024};

struct ProtocolEncoder
{
    enum EncoderResult : uint16_t
    {
        DORMANT = 1,
        ENCODING_MESSAGE,
        MESSAGE_COMPLETE,
        MESSAGE_TERMINATED,

        MESSAGE_INITIATED,

        ERROR_UNKNOWN = 1 << 7,
        ERROR_NO_MESSAGE_ACTIVE,
        ERROR_MESSAGE_TOO_LARGE,
        ERROR_MIME_TYPE_TOO_LARGE,
        ERROR_MESSAGE_ALREADY_ACTIVE
    };

    bool isError(EncoderResult r)
    {
        if (((uint16_t)r) & ((uint16_t)ERROR_UNKNOWN))
            return true;
        return false;
    }

    TIPSY_NODISCARD
    EncoderResult initiateMessage(const char *inMimeType, uint32_t inDataBytes,
                                  const unsigned char *const inData)
    {
        if (inDataBytes > 1 << 23)
        {
            return ERROR_MESSAGE_TOO_LARGE;
        }
        auto ms = strlen(inMimeType) + 1;
        if (ms > kMaxMimeTypeSize)
        {
            return ERROR_MIME_TYPE_TOO_LARGE;
        }
        if (messageActive)
        {
            return ERROR_MESSAGE_ALREADY_ACTIVE;
        }

        messageActive = true;
        mimeType = inMimeType;
        mimeTypeSize = ms;
        data = inData;
        dataBytes = inDataBytes;

        setState(START_MESSAGE);

        return MESSAGE_INITIATED;
    }

    TIPSY_NODISCARD
    EncoderResult getNextMessageFloat(float &f)
    {
        switch (encoderState)
        {
        case NO_MESSAGE:
            f = 0;
            return DORMANT;
            break;
        case START_MESSAGE:
        {
            f = kMessageBeginSentinel;
            pos++;
            if (pos == 3)
            {
                setState(HEADER_VERSION);
            }
            return ENCODING_MESSAGE;
            break;
        }
        case HEADER_VERSION:
        {
            if (pos == 0)
            {
                f = kVersionSentinel;
                pos++;
            }
            else
            {
                unsigned char d[3];
                d[0] = kVersion & 255;
                d[1] = (kVersion >> 8) & 255;
                d[2] = 0;
                f = threeBytesToFloat(d);
                setState(HEADER_SIZE);
            }
            return ENCODING_MESSAGE;
        }
        break;
        case HEADER_SIZE:
        {
            if (pos == 0)
            {
                f = kSizeSentinel;
                pos++;
            }
            else
            {
                unsigned char d[3];
                d[0] = dataBytes & 255;
                d[1] = (dataBytes >> 8) & 255;
                d[2] = (dataBytes >> 16) & 255;

                f = threeBytesToFloat(d);
                setState(HEADER_MIMETYPE);
            }
            return ENCODING_MESSAGE;
        }
        break;
        case HEADER_MIMETYPE:
        {
            if (pos == 0)
            {
                f = kMimeTypeSentinel;
                pos++;
            }
            else if (pos == 1)
            {
                auto sl = mimeTypeSize;
                unsigned char d[3];
                d[0] = sl & 255;
                d[1] = (sl >> 8) & 255;
                d[2] = (sl >> 16) & 255;

                f = threeBytesToFloat(d);
                pos++;
            }
            else
            {
                auto dp = pos - 2;
                if (dp < mimeTypeSize - 3)
                {
                    f = threeBytesToFloat((unsigned char *)(mimeType + dp));

                    pos += 3;
                    if (pos - 1 == mimeTypeSize)
                    {
                        setState(BODY);
                    }
                }
                else
                {
                    char d[3]{0, 0, 0};
                    int i{0};
                    for (; dp < mimeTypeSize; ++dp)
                    {
                        d[i] = mimeType[dp];
                        i++;
                    }
                    f = threeBytesToFloat((unsigned char *)d);
                    setState(BODY);
                }
            }
            return ENCODING_MESSAGE;
        }
        break;
        case BODY:
        {
            if (pos == 0)
            {
                f = kBodySentinel;
                pos++;
            }
            else if (pos - 1 == dataBytes)
            {
                setState(END_MESSAGE);
            }
            else if (pos - 1 < dataBytes - 3)
            {
                f = threeBytesToFloat((unsigned char *)(data + pos - 1));

                pos += 3;
                if (pos - 1 == dataBytes)
                {
                    setState(END_MESSAGE);
                }
            }
            else
            {
                auto dp = pos - 1;
                char d[3]{0, 0, 0};
                int i{0};
                for (; dp < dataBytes; ++dp)
                {
                    d[i] = data[dp];
                    i++;
                }
                f = threeBytesToFloat((unsigned char *)d);
                setState(END_MESSAGE);
            }
            return ENCODING_MESSAGE;
        }
        break;
        case END_MESSAGE:
        {
            f = kEndMessageSentinel;
            setState(NO_MESSAGE);
            return MESSAGE_COMPLETE;
        }
        break;
        }

        return ERROR_UNKNOWN;
    }

    TIPSY_NODISCARD
    EncoderResult terminateCurrentMessage()
    {
        if (encoderState == NO_MESSAGE)
        {
            return ERROR_NO_MESSAGE_ACTIVE;
        }
        setState(NO_MESSAGE);
        return MESSAGE_TERMINATED;
    }

  private:
    bool messageActive{false};
    const char *mimeType{nullptr};
    uint32_t dataBytes{0}, mimeTypeSize{0};
    const unsigned char *data;

    enum EncoderStates
    {
        NO_MESSAGE,
        START_MESSAGE,
        HEADER_VERSION,
        HEADER_SIZE,
        HEADER_MIMETYPE,
        BODY,
        END_MESSAGE
    } encoderState{NO_MESSAGE};

    int pos{0};

    void setState(EncoderStates s)
    {
        encoderState = s;
        pos = 0;
    }
};

struct ProtocolDecoder
{
    enum DecoderResult : uint16_t
    {
        DORMANT = 1,
        PARSING_HEADER,
        HEADER_READY,
        PARSING_BODY,
        BODY_READY,

        ERROR_UNKNOWN = 1 << 8, // set this bit for all the errors
        ERROR_INCOMPATIBLE_VERSION,
        ERROR_MALFORMED_HEADER,
        ERROR_DATA_TOO_LARGE,

        // OK,
        // NOT_OK
    };

    bool isError(DecoderResult r)
    {
        if (((uint16_t)r) & ((uint16_t)ERROR_UNKNOWN))
            return true;
        return false;
    }

    bool provideDataBuffer(unsigned char *pdat, size_t psize)
    {
        if (decoderState == START_BODY)
            return false;

        dataStore = pdat;
        dataStoreSize = psize;

        return true;
    }

    const char *getMimeType() { return mimetype; }

    TIPSY_NODISCARD
    DecoderResult readFloat(float f)
    {
        if (f == kMessageBeginSentinel)
        {
            setState(START_HEADER);
            dataSize = 0;
            memset(mimetype, 0, sizeof(mimetype));
            version = -1;
            return PARSING_HEADER;
        }

        // Use sentinels to force state for next read
        if (f == kVersionSentinel)
        {
            setState(START_VERSION);
            return PARSING_HEADER;
        }
        if (f == kSizeSentinel)
        {
            setState(START_SIZE);
            return PARSING_HEADER;
        }
        if (f == kMimeTypeSentinel)
        {
            setState(START_MIMETYPE);
            return PARSING_HEADER;
        }
        if (f == kBodySentinel)
        {
            setState(START_BODY);
            return HEADER_READY;
        }
        if (f == kEndMessageSentinel)
        {
            setState(DOING_NOTHING);
            return BODY_READY;
        }

        switch (decoderState)
        {
        case DOING_NOTHING:
            return DORMANT;
        case START_HEADER:
            return PARSING_HEADER;
        case START_VERSION:
            if (pos == 0)
            {
                unsigned char d[3];
                floatToThreeBytes(f, d);
                version = d[0] + (d[1] << 8);
                pos++;
                if (version > 0 && version <= kVersion)
                {
                    return PARSING_HEADER;
                }
                else
                {
                    return ERROR_INCOMPATIBLE_VERSION;
                }
            }
            else
            {
                return ERROR_MALFORMED_HEADER;
            }
            break;

        case START_SIZE:
            if (pos == 0)
            {
                unsigned char d[3];
                floatToThreeBytes(f, d);
                dataSize = d[0] + (d[1] << 8) + (d[2] << 16);
                pos++;
                return PARSING_HEADER;
            }
            else
            {
                return ERROR_MALFORMED_HEADER;
            }
            break;

        case START_MIMETYPE:
        {
            if (pos == 0)
            {
                unsigned char d[3];
                floatToThreeBytes(f, d);
                mimetypeSize = d[0] + (d[1] << 8);

                pos++;
                return PARSING_HEADER;
            }
            else
            {
                if (pos > mimetypeSize)
                {
                    return ERROR_MALFORMED_HEADER;
                }
                unsigned char d[3];
                floatToThreeBytes(f, d);

                if (pos >= kMaxMimeTypeSize - 4)
                    return ERROR_DATA_TOO_LARGE;

                auto wp = pos - 1;
                mimetype[wp] = d[0];
                mimetype[wp + 1] = d[1];
                mimetype[wp + 2] = d[2];

                pos += 3;
                return PARSING_HEADER;
            }
            break;
        }
        case START_BODY:
            unsigned char d[3];
            floatToThreeBytes(f, d);

            if (pos < dataStoreSize + 3)
            {
                dataStore[pos] = d[0];
                dataStore[pos + 1] = d[1];
                dataStore[pos + 2] = d[2];
                pos += 3;
                return PARSING_BODY;
            }
            else
            {
                return ERROR_DATA_TOO_LARGE;
            }
            break;
        }

        return ERROR_UNKNOWN;
    }

  private:
    enum DecoderState
    {
        DOING_NOTHING,
        START_VERSION,
        START_HEADER,
        START_SIZE,
        START_MIMETYPE,
        START_BODY
    } decoderState{DOING_NOTHING};
    int pos{0};
    int16_t version;
    uint32_t dataSize;
    char mimetype[kMaxMimeTypeSize];
    uint16_t mimetypeSize;

    unsigned char *dataStore{nullptr};
    int dataStoreSize{0};

    void setState(DecoderState s)
    {
        decoderState = s;
        pos = 0;
    }
};
} // namespace tipsy

#endif // TIPSY_ENCODER_PROTOCOL_H
