#pragma once

#include <cstdint>
#include <functional> // For std::function
#include "DataTypes.h"

namespace tinyalg::waveu {

#define GET_LUT_VALUE(lut, index) ((lut)[(index)])

using LUTIndexFunction = int(*)(size_t phaseValue, size_t numBits);

class LUTHelper {
public:
    static inline int calculateIndex_16(size_t phaseValue, size_t numBits);
    static inline int calculateIndex_32(size_t phaseValue, size_t numBits);
    static inline int calculateIndex_64(size_t phaseValue, size_t numBits);
    static inline int calculateIndex_128(size_t phaseValue, size_t numBits);
    static inline int calculateIndex_256(size_t phaseValue, size_t numBits);
    static inline int calculateIndex_512(size_t phaseValue, size_t numBits);
    static inline int calculateIndex_1024(size_t phaseValue, size_t numBits);
    static inline int calculateIndex_2048(size_t phaseValue, size_t numBits);

    /**
     * @brief Retrieves a function pointer to calculate the LUT index for a given LUT size.
     *
     * This method returns a function that computes the appropriate LUT index based on
     * the phase value and the number of bits used for phase increment calculations. The
     * function returned is tailored to the specified LUT size.
     *
     * @param lutSize The size of the Lookup Table (LUT) for which the index function is required.
     *                Valid values include predefined sizes such as LUT_16, LUT_32, etc.
     * 
     * @return A function pointer of type `LUTIndexFunction` that computes the LUT index.
     *         The function returned takes two parameters:
     *         - `phaseValue`: The current phase value to compute the index for.
     *         - `numBits`: The number of bits used in phase increment calculations.
     * 
     * @note The returned function is optimized for the specified LUT size and assumes that
     *       the input parameters are within valid ranges.
     */
    static LUTIndexFunction getIndexFunction(LUTSize lutSize);

    /**
     * @brief Adjusts amplitude and offset to ensure they fit within valid bounds.
     * 
     * This function ensures that amplitude and offset values are valid for an
     * 8-bit DAC. It clamps the amplitude and adjusts the offset to prevent the
     * waveform from exceeding the DAC range [0, 255].
     * 
     * @param amplitude The desired amplitude of the waveform. Must be non-negative
     *                  and will be clamped to a maximum of 127.5.
     * @param offset    The desired center point of the waveform. Adjusted to ensure
     *                  (offset + amplitude <= 255.0) and (offset - amplitude >= 0.0).
     * 
     * @return std::pair<float, float> A pair containing the adjusted amplitude and offset.
     */
    static std::pair<float, float> adjustAmplitudeAndOffset(float amplitude, float offset);
};

} // namespace tinyalg::waveu
