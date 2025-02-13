#pragma once

#include <type_traits>

#include "BoardConfig.h"
#include "WaveConfig.h"
#include "Wave2Config.h"
#include "WaveConfigArgs.h"
#include "BoardConfigInterface.h"
#include "WaveConfigInterface.h"

namespace tinyalg::waveu {

struct DummyWaveConfig {
    void initialize() {}
};

/**
 * @brief A waveform generator template class for generating and managing waveforms.
 * 
 * This class handles configuration, starting, stopping, and resetting waveform generation
 * in a structured and state-managed manner. It uses board and waveform configurations
 * provided as template parameters.
 */
template <typename BoardConfig, typename WaveConfig, typename Wave2Config = void>
class Waveu {
public:
    /**
     * @brief Ensures the provided configurations are valid at compile time.
     */
    static_assert(std::is_base_of<BoardConfigInterface, BoardConfig>::value,
                  "TargetConfig must derive from TargetConfigInterface");
    static_assert(std::is_base_of<WaveConfigInterface, WaveConfig>::value,
                  "WaveConfig must derive from WaveConfigInterface");
    static_assert(std::is_same_v<Wave2Config, void> || std::is_base_of_v<WaveConfigInterface, Wave2Config>,
                  "Wave2Config must derive from WaveConfigInterface or be void");
    
    BoardConfig brd;
    WaveConfig chan;
    std::conditional_t<!std::is_same_v<Wave2Config, void>, Wave2Config, tinyalg::waveu::DummyWaveConfig> chan2;
    static const char* TAG;

    /**
     * @brief Represents the possible states of the Waveu object.
     * 
     * This enum defines the states of the `Waveu` object during its lifecycle. 
     * It helps track the current status of the object and determines which operations
     * are allowed (e.g., starting, stopping, configuring).
     */
    enum class State
    { 
    
        /**
         * @brief The object is in the idle state and has not been configured yet.
         * 
         * The object is not generating a waveform and is not configured. It is 
         * in its initial state.
         */ 
        Idle, 

        /**
         * @brief The object is configured but not yet started.
         * 
         * The object has been configured with a waveform and is ready to generate
         * a waveform once the `start()` method is called.
         */
        Configured, 
        
        /**
         * @brief The object is currently generating a waveform.
         * 
         * The object is actively generating a waveform. This state is entered 
         * after calling `start()` and can be exited by calling `stop()`.
         */
        Running,

        Stopped
    };

    /**
     * @brief Default constructor for the Waveu class.
     * 
     * Initializes the object in the **Idle** state and sets up internal structures
     * for waveform generation.
     */
    Waveu();

    /**
     * @brief Destructor for the Waveu class.
     * 
     * Cleans up any allocated resources and ensures safe shutdown of the generator.
     */
    ~Waveu();

    /**
     * @brief Retrieves the current state of the waveform generator.
     * 
     * @return The current state of the object as a `Waveu::State` value.
     */
    State getState() const { return currentState; }

    /**
     * @brief Configure the waveform generator with the specified parameters.
     * 
     * Prepares the generator for waveform generation by setting up the necessary configurations.
     * This method can be called when the object is in the **Idle** or **Configured** state.
     * 
     * @param args Configuration arguments for the waveform generator.
     * 
     * @throws InvalidStateTransitionException If called in the **Running** or **Stopped** states.
     */
    void configure(const WaveConfigArgs& args);

    /**
     * @brief Start the waveform generator.
     * 
     * Transitions the object from the **Configured** state to the **Running** state
     * and begins waveform generation.
     * 
     * @throws InvalidStateTransitionException If called in a state other than **Configured**.
     */
    void start();

    /**
     * @brief Stop the waveform generator.
     * 
     * Transitions the object from the **Running** state to the **Stopped** state,
     * halting waveform generation while retaining the configuration.
     * 
     * @throws InvalidStateTransitionException If called in a state other than **Running**.
     */
    void stop();

    /**
     * @brief Reset the waveform generator to prepare it for reconfiguration or restarting.
     * 
     * Transitions the object from the **Stopped** state to the **Configured** state
     * while retaining the existing configuration.
     * 
     * @throws InvalidStateTransitionException If called in a state other than **Stopped**.
     */
    void reset();

    /**
     * @brief FreeRTOS-compatible task function for waveform data generation.
     * 
     * This static function is designed to run as a FreeRTOS task. It processes waveform
     * data generation and scheduling. The function operates on an instance of the `Waveu`
     * object provided via the `args` parameter.
     * 
     * @param args Pointer to the `Waveu` instance.
     */
    static void waveformDataGenerationTask(void *args);

    /**
     * @brief Converts a `Waveu::State` enum value to a string representation.
     * 
     * This static method is useful for logging or debugging purposes by providing
     * human-readable names for the generator's states.
     * 
     * @param state The state to be converted.
     * @return A `const char*` string representing the state.
     */
    static const char* toString(State state);

private:
    /**
     * @brief The current state of the waveform generator.
     * 
     * This variable tracks the lifecycle state of the object and controls transitions.
     */
    State currentState = State::Idle;

    /**
     * @brief Tracks the elapsed time since the generator started.
     * 
     * This value is used for timing-related calculations during waveform generation.
     */
    uint64_t elapsedTime = 0;
};

// Initialize the static member outside the class definition
template <typename PlatformConfig, typename WaveConfig, typename Wave2Config>
const char* Waveu<PlatformConfig, WaveConfig, Wave2Config>::TAG = "Waveu";

} // namespace tinyalg::waveu

#include "WaveuImpl.h"
