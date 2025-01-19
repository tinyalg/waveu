#include <cstddef> // For size_t
#include <stdexcept> // For exceptions

#include "LUTHelper.h"

namespace tinyalg::waveu {

inline int LUTHelper::calculateIndex_16(size_t phaseValue, size_t numBits) {
    return (phaseValue >> (numBits - 4)) & (16 - 1);
}
inline int LUTHelper::calculateIndex_32(size_t phaseValue, size_t numBits) {
    return (phaseValue >> (numBits - 5)) & (32 - 1);
}
inline int LUTHelper::calculateIndex_64(size_t phaseValue, size_t numBits) {
    return (phaseValue >> (numBits - 6)) & (64 - 1);
}
inline int LUTHelper::calculateIndex_128(size_t phaseValue, size_t numBits) {
    return (phaseValue >> (numBits - 7)) & (128 - 1);
}
inline int LUTHelper::calculateIndex_256(size_t phaseValue, size_t numBits) {
    return (phaseValue >> (numBits - 8)) & (256 - 1);
}
inline int LUTHelper::calculateIndex_512(size_t phaseValue, size_t numBits) {
    return (phaseValue >> (numBits - 9)) & (512 - 1);
}
inline int LUTHelper::calculateIndex_1024(size_t phaseValue, size_t numBits) {
    return (phaseValue >> (numBits - 10)) & (1024 - 1);
}
inline int LUTHelper::calculateIndex_2048(size_t phaseValue, size_t numBits) {
    return (phaseValue >> (numBits - 11)) & (2048 - 1);
}

LUTIndexFunction LUTHelper::getIndexFunction(LUTSize lutSize) {
    switch (lutSize) {
        case LUT_16:
            return calculateIndex_16;
        case LUT_32:
            return calculateIndex_32;
        case LUT_64:
            return calculateIndex_64;
        case LUT_128:
            return calculateIndex_128;
        case LUT_256:
            return calculateIndex_256;
        case LUT_512:
            return calculateIndex_512;
        case LUT_1024:
            return calculateIndex_1024;
        case LUT_2048:
            return calculateIndex_2048;
        // Add more cases for other LUT sizes
        default:
            throw std::invalid_argument("Unsupported LUT size");
    }

}

std::pair<float, float> LUTHelper::adjustAmplitudeAndOffset(float amplitude, float offset) {
    constexpr float MAX_AMPLITUDE = 127.5f;
    constexpr float DAC_MAX = 255.0f;
    constexpr float DAC_MIN = 0.0f;

    // Clamp amplitude
    if (amplitude < DAC_MIN) {
        amplitude = DAC_MIN;
    } else if (amplitude > MAX_AMPLITUDE) {
        amplitude = MAX_AMPLITUDE;
    }

    // Adjust offset
    if (offset < DAC_MIN) {
        offset = DAC_MIN;
    }
    if (offset + amplitude > DAC_MAX) {
        offset = DAC_MAX - amplitude;
    }
    if (offset - amplitude < DAC_MIN) {
        offset = amplitude;
    }

    return std::make_pair(amplitude, offset);
}

}
