#pragma once
#ifndef TIPSY_ENCODER_BINARY_TO_FLOAT_H
#define TIPSY_ENCODER_BINARY_TO_FLOAT_H
/*
 * binary-to-float.h
 *
 * Low level encoding to take 3 unsigned chars and project them to a single valid float
 * and vice versa. The values used are designed for compatability with VCV Rack voltage standards.
 *
 * The encoding we use is as follows:
 *
 * An IEEE float is: lowest 23 bits are fraction, next 8 are exponent, last is sign bit.
 *
 */
#include <cstdint>

namespace tipsy
{

constexpr const unsigned char BYTE_MASK = 0xff;
constexpr const unsigned char LOW_7_MASK = 0x7f;
constexpr const unsigned char BIT_8_MASK = 0x80;
constexpr const unsigned char EXPONENT_FILL = 0x3f;

union FloatBytes
{
    unsigned char bytes[4];
    float f;

    operator float() { return f; }

    unsigned char first() { return bytes[0]; }
    unsigned char second() { return bytes[1]; }
    unsigned char third() { return (bytes[2] & LOW_7_MASK) | (bytes[3] & BIT_8_MASK); }

    FloatBytes(float f)
    {
        this->f = f;
    }

    FloatBytes()
    {
        bytes[0] = 0;
        bytes[1] = 0;
        bytes[2] = 0;
        bytes[3] = 0x3f;
    }

    FloatBytes(unsigned char b1, unsigned char b2)
    {
        bytes[0] = b1;
        bytes[1] = b2;
        bytes[2] = 0;
        bytes[3] = EXPONENT_FILL;
    }

    FloatBytes(uint16_t value)
    {
        bytes[0] = value & BYTE_MASK;
        bytes[1] = (value >> 8) & BYTE_MASK;
        bytes[2] = 0;
        bytes[3] = EXPONENT_FILL;
    }

    static bool isRepresentable(uint32_t value)
    {
        return 0 == (value & (uint32_t)0xFF000000);
    }

    FloatBytes(uint32_t value)
    {
        // Not all uint32_t values are representable in this format
        // Should this state raise an exception? 
        assert(isRepresentable(value));
        bytes[0] = value & BYTE_MASK;
        bytes[1] = (value >> 8) & BYTE_MASK;
        unsigned char b3 = (value >> 16) & BYTE_MASK;
        bytes[2] = b3 & LOW_7_MASK;
        bytes[3] = (b3 & BIT_8_MASK) | EXPONENT_FILL;
    }

    FloatBytes(unsigned char b1, unsigned char b2, unsigned char b3) {
        bytes[0] = b1;
        bytes[1] = b2;
        bytes[2] = b3 & LOW_7_MASK;
        bytes[3] = (b3 & BIT_8_MASK) | EXPONENT_FILL;
    }
};

inline uint16_t uint16_FromFloat(float f)
{
    auto fb = FloatBytes(f);
    return fb.bytes[0] | (fb.bytes[1] << 8);
}

inline uint32_t uint32_FromFloat(float f)
{
    auto fb = FloatBytes(f);
    return fb.first() | (fb.second() << 8) | (fb.third() << 16);
}

inline float threeBytesToFloat(unsigned char b1, unsigned char b2, unsigned char b3)
{
    return FloatBytes(b1, b2, b3).f;
}

inline unsigned char FirstByte(float f)
{
    return FloatBytes(f).first();
}

inline unsigned char SecondByte(float f)
{
    return FloatBytes(f).second();
}

inline unsigned char ThirdByte(float f)
{
    return FloatBytes(f).third();
}

}
#endif // TIPSY_ENCODER_BINARY_TO_FLOAT_H
