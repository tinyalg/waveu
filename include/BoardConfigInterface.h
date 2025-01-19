#pragma once

namespace tinyalg::waveu {

/**
 * @brief Interface for board-specific configurations required for waveform generation.
 * 
 * This interface defines the methods required to configure and manage hardware components 
 * such as DACs, GPIOs, and timers. It serves as a contract for concrete implementations 
 * tailored to specific hardware platforms.
 */
class BoardConfigInterface {
public:
    /**
     * @brief Virtual destructor for proper cleanup of derived classes.
     */
    virtual ~BoardConfigInterface() = default;

    /**
     * @brief Initialize the DAC (Digital-to-Analog Converter) for waveform output.
     * 
     * This method sets up the DAC with the necessary configuration parameters 
     * for generating waveforms.
     */
    virtual void initializeDac() = 0;

    /**
     * @brief Configure the GPIO (General Purpose Input/Output) pins required for the board.
     * 
     * Sets up the GPIOs used for controlling peripherals or interacting with external components.
     */
    virtual void setupGpio() = 0;

    /**
     * @brief Prepare the timer for managing waveform generation timing.
     * 
     * This method initializes and configures the timer, ensuring it operates 
     * with the correct frequency and settings.
     */
    virtual void prepareTimer() = 0;

    /**
     * @brief Start the timer to begin waveform generation.
     * 
     * This method activates the timer, enabling waveform output to commence.
     */
    virtual void startTimer() = 0;

    /**
     * @brief Stop the timer to halt waveform generation.
     * 
     * This method deactivates the timer, pausing waveform output.
     */
    virtual void stopTimer() = 0;

    /**
     * @brief Clean up the timer to halt waveform generation.
     * 
     * This method clean up the timer resources.
     */
    virtual void cleanupTimer() = 0;


    /**
     * @brief Resets the waveform generator.
     * 
     * This method is called to reset the internal state of the waveform
     * generator, typically to a known initial state.
     */
    virtual void reset() = 0;
};

}
