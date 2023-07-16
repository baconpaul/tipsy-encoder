/*
 * binary-to-float.h
 *
 * low level encoding to take 3 unsigned chars and project them to a single valid float
 * and vice versa.
 *
 * The encoding we use is as follows:
 *
 * An IEEE float is: lowest 23 bits are fraction, next 8 are exponent, last is sign bit.
 *
 */

#ifndef TIPSY_ENCODER_BINARY_TO_FLOAT_H
#define TIPSY_ENCODER_BINARY_TO_FLOAT_H

#include <cstdint>

namespace tipsy
{
inline float threeBytesToFloat(const unsigned char * const d)
{
    unsigned char enc[4];
    enc[0] = d[0];
    enc[1] = d[1];
    enc[2] = d[2] & 127;
    enc[3] = (d[2] & 128) | 63;

    float res = *((float *)&(enc[0]));

    return res;
}

inline void floatToThreeBytes(float f, unsigned char * const d)
{
    auto *y = (unsigned char *)&f;

    d[0] = y[0];
    d[1] = y[1];
    d[2] = (y[2] & 127) | (y[3] & 128);
}
}

#endif // TIPSY_ENCODER_BINARY_TO_FLOAT_H
