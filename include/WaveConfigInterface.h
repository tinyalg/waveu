#pragma once

#include "DataTypes.h"
#include "WaveConfigArgs.h"

namespace tinyalg::waveu {

/**
 * @class WaveConfigInterface
 * @brief Interface for configuring and managing waveform generation.
 * 
 * This interface defines the required methods for configuring waveform
 * parameters, initializing resources, generating samples, and managing
 * the waveform's lifecycle.
 */
class WaveConfigInterface {
public:
    /**
     * @brief Virtual destructor.
     * 
     * Ensures proper cleanup of derived class objects when deleted through
     * a `WaveConfigInterface` pointer.
     */
    virtual ~WaveConfigInterface() = default;

    /**
     * @brief Configures waveform parameters.
     * 
     * This method is called to set up the waveform's configuration, such as
     * frequency, amplitude, or any other specific parameters, using the
     * provided arguments.
     * 
     * @param args The configuration arguments for the waveform.
     */
    virtual void configure(const tinyalg::waveu::WaveConfigArgs& args) = 0;

    /**
     * @brief Initializes the waveform generator.
     * 
     * This method is called once to initialize the waveform generator
     * with the provided sample rate. It sets up any internal resources
     * needed for waveform synthesis.
     * 
     * @param sampleRate The sample rate for waveform generation, in Hz.
     */
    virtual void initialize(uint32_t sampleRate) = 0;

    /**
     * @brief Retrieves the next sample of the waveform.
     * 
     * This method is called to generate and return the next sample value
     * of the waveform. The value is typically normalized to fit within
     * 8 bits.
     * 
     * @return The next waveform sample as an 8-bit unsigned integer.
     */
    virtual sample_type_t nextSample() = 0;

#ifdef CONFIG_WAVEU_CHANNEL_MODE_ALTER
    /**
     * @brief Retrieves the next sample of the waveform.
     * 
     * This method is called to generate and return the next sample value
     * of the waveform. The value is typically normalized to fit within
     * 8 bits.
     * 
     * @return The next waveform sample as an 8-bit unsigned integer.
     */
    virtual sample_type_t nextSampleB() = 0;
  #ifdef CONFIG_WAVEU_DATA_SINK_UDP
    /**
     * @brief Retrieves the next sample of the waveform.
     * 
     * This method is called to generate and return the next sample value
     * of the waveform. The value is typically normalized to fit within
     * 8 bits.
     * 
     * @return The next waveform sample as an 8-bit unsigned integer.
     */
    virtual sample_type_t nextSampleC() = 0;

        /**
     * @brief Retrieves the next sample of the waveform.
     * 
     * This method is called to generate and return the next sample value
     * of the waveform. The value is typically normalized to fit within
     * 8 bits.
     * 
     * @return The next waveform sample as an 8-bit unsigned integer.
     */
    virtual sample_type_t nextSampleD() = 0;
  #endif
#endif

    /**
     * @brief Prepares for the next waveform generation cycle.
     * 
     * This method is called before the start of a new waveform generation
     * cycle. It is typically used to reset or update internal states to
     * ensure a seamless transition.
     * @param elapsedTime Elapsed time since the timer started.
     */
    virtual void prepareCycle(double elapsedTime) = 0;

    /**
     * @brief Resets the waveform generator.
     * 
     * This method is called to reset the internal state of the waveform
     * generator, typically to a known initial state.
     */
    virtual void reset() = 0;
};

}
