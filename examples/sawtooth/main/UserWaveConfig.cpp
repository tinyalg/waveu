#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ESP32Waveu.h"

using tinyalg::waveu::PhaseGenerator;

static const char* TAG = "UserWaveConfig";

// Derived class for user wave arguments
class UserWaveArgs : public tinyalg::waveu::WaveConfigArgs {
};

class UserWaveConfig : public tinyalg::waveu::WaveConfigInterface {
public:
    ~UserWaveConfig() override {
        delete _phaseGenerator; // Clean up
    }

    void initialize(uint32_t sampleRate) override {
        // Initializes PhaseGenerator that manages the phase increment and the current phase.
        if (_phaseGenerator == nullptr) {
            _phaseGenerator = new PhaseGenerator(sampleRate);
        }

        // Sets the desired frequency and calculates the phase increment.
        float frequency = 200.0f;
        _phaseGenerator->setFrequency(frequency);
        ESP_LOGI(TAG, "Frequency=%.02f", _phaseGenerator->getFrequency());
    }

    void configure(const tinyalg::waveu::WaveConfigArgs& args) override {
    }

    void prepareCycle(double elapsedTime) override {
    }

    uint8_t nextSample() override {
        // Step 1: Advance the phase to the next position.
        _phaseGenerator->updatePhase();

        // Step 2: Retrieve the updated phase value.
        uint32_t currentPhase = _phaseGenerator->getPhase();

        // Step 3: Generate the waveform.
        
        // Sawtooth wave logic
        uint8_t digi_val = (currentPhase >> (PhaseGenerator::N_BITS - 8)) & 0xFF;

        // Optional: Set to true if you use triangular wave logic
        constexpr bool USE_TRIANGULAR_WAVE = false;
        if (USE_TRIANGULAR_WAVE) { 
            uint32_t msb = (currentPhase >> (PhaseGenerator::N_BITS - 1)) & 1;
            digi_val = msb ? digi_val : 255 - digi_val; 
        }

        // Return the voltage value for the sample.
        return (uint8_t)digi_val;
    }

    void reset() override {
    }

private:
    /// @brief Pointer to PhaseGenerator
    PhaseGenerator* _phaseGenerator = nullptr;
};

extern "C" {
    void app_main(void)
    {
        // Initialize the waveform generator with a user-defined configuration.
        tinyalg::waveu::ESP32Waveu<UserWaveConfig> waveu;

        // Define the waveform arguments, which is empty.
        UserWaveArgs waveArgs;

        try {
            // Configure the waveform generator with the specified arguments.
            waveu.configure(waveArgs);

            // Start the waveform generation process.
            waveu.start();

        } catch (const tinyalg::waveu::InvalidStateTransitionException& e) {
            ESP_LOGE(TAG, "Exception: %s\n", e.what());
        }

        // Prevent app_main() from exiting, keeping local variables in memory.
        vTaskDelay(portMAX_DELAY);
    }
}
