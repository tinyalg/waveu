#pragma once

#include <cstdint>

namespace tinyalg::waveu {

/**
 * @class PhaseGenerator
 * @brief Generates phase information for waveform synthesis.
 */
class PhaseGenerator {
public:
    /**
     * @brief Number of bits used for the phase increment.
     * 
     * This constant defines the bit-width of the phase increment
     * used in phase accumulator calculations.
     */
    static const int N_BITS = 32;
    [[deprecated("This variable is obsolete. Use N_BITS instead.")]]
    static const int NUM_BITS_FOR_PHASE_INCREMENT = 32;
    
    /**
     * @brief Constructor to initialize the PhaseGenerator.
     * 
     * @param sampleRate The sample rate of the output signal in Hz.
     */
    explicit PhaseGenerator(uint32_t sampleRate);

    /**
     * @brief Sets the desired frequency for phase generation.
     * 
     * This method calculates and sets the phase increment value
     * based on the specified frequency and the sample rate.
     * 
     * @param frequency The desired frequency in Hz.
     */
    void setFrequency(float frequency);

    /**
     * @brief Updates the phase accumulator.
     * 
     * This method advances the phase accumulator by the phase
     * increment, ensuring it wraps around correctly for periodic waveforms.
     */
    void updatePhase();

    /**
     * @brief Retrieves the current phase value.
     * 
     * The phase value is normalized to the range of a 32-bit unsigned
     * integer, which allows it to represent a full cycle of the waveform.
     * 
     * @return The current phase as a 32-bit unsigned integer.
     */
    uint32_t getPhase() const;

    /**
     * @brief Resets the phase accumulator to the expected phase using the current elapsed time.
     * 
     * This method can be used to reset the phase accumulator, not to have accumulated error
     * overtime.
     */
    void reset(double elapsedTime);

    /**
     * @brief Resets the phase accumulator to zero.
     * 
     * This method resets the phase accumulator, effectively restarting
     * the waveform phase cycle from the beginning.
     */
    void reset();

    float getFrequency();

private:
    uint32_t sampleRate_;       // Sampling rate in Hz
    float frequency_ = 0.0f;    // Frequency in Hz
    uint32_t phaseIncrement_ = 0; // Phase increment for the current frequency
    uint32_t phase_ = 0;        // Current phase accumulator
};

} // namespace tinyalg::waveu
