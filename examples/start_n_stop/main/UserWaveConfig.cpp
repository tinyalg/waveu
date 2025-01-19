#include <cstdint>
#include <cmath> // for sin function
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ESP32Waveu.h"

using tinyalg::waveu::PhaseGenerator;
using tinyalg::waveu::LUTIndexFunction;
using tinyalg::waveu::LUTHelper;
using tinyalg::waveu::LUTSize;
using tinyalg::waveu::lut_type_t;
using tinyalg::waveu::LUT_256;

static const char* TAG = "UserWaveConfig";

// Derived class for user wave arguments
class UserWaveArgs : public tinyalg::waveu::WaveConfigArgs {
public:
    float frequency;

    UserWaveArgs(float frequency) : frequency(frequency) {}
};

class UserWaveConfig : public tinyalg::waveu::WaveConfigInterface {
protected:
    virtual lut_type_t fill_function_sine(LUTSize size, size_t index, float amplitude, float offset) {
        size_t table_size = static_cast<size_t>(size);
        return static_cast<lut_type_t>(
            amplitude * sin(index * 2 * M_PI / (table_size - 1)) + offset + 0.5
        );
    }

    virtual void initializePhaseGenerator(uint32_t sampleRate) {
        if (_phaseGenerator == nullptr) {
            _phaseGenerator = new PhaseGenerator(sampleRate);
        }
    }

    virtual void setPhaseGeneratorFrequency(double freq) {
        _phaseGenerator->setFrequency(freq);
        ESP_LOGI(TAG, "Frequency set to %.02fHz", _phaseGenerator->getFrequency());
    }

    virtual void populateLUTs() {
        float amplitude = 127.5f;
        float offset = 127.5f;
        size_t table_size = static_cast<size_t>(_lutSize);
        for (size_t i = 0; i < table_size; ++i) {
            _lut[i] = fill_function_sine(_lutSize, i, amplitude, offset);
        }
    }

public:
    ~UserWaveConfig() override {
        delete _phaseGenerator; // Clean up
    }

    void initialize(uint32_t sampleRate) override {
        // Initializes the PhaseGenerator with the specified sample rate.
        initializePhaseGenerator(sampleRate);

        // Retrieves a function to calculate the LUT index from a phase.
        _getIndex = LUTHelper::getIndexFunction(_lutSize);
    }

    void configure(const tinyalg::waveu::WaveConfigArgs& args) override {
        const auto& waveArgs = dynamic_cast<const UserWaveArgs&>(args); // Safe because caller ensures type
        double freq = waveArgs.frequency;

        // Sets the frequency and calculates the phase increment.
        setPhaseGeneratorFrequency(freq);
        // Fills the values to the LUT using the supplied function.
        populateLUTs();
    }

    void prepareCycle(double elapsedTime) override {
        _halfAmplitude = !_halfAmplitude;
    }

    uint8_t nextSample() override {
        // Step 1: Advance the phase to the next position.
        _phaseGenerator->updatePhase();

        // Step 2: Retrieve the updated phase value.
        uint32_t currentPhase = _phaseGenerator->getPhase();

        // Step 3: Map the phase value to an appropriate index in the lookup table.
        int lutIndex = _getIndex(currentPhase, PhaseGenerator::N_BITS);

        // Step 4: Fetch the corresponding voltage value (0-255) from the lookup table using the macro.
        lut_type_t digi_val = GET_LUT_VALUE(_lut, lutIndex);

        // Return the voltage value for the sample.
        return (uint8_t)digi_val;
    }

#ifdef CONFIG_WAVEU_CHANNEL_MODE_ALTER
    uint8_t nextSampleB() override {
        // Step b1: Retrieve the updated phase value.
        uint32_t currentPhase = _phaseGenerator->getPhase();

        // Step b2: Map the phase value to an appropriate index in the lookup table.
        int lutIndex = _getIndex(currentPhase, PhaseGenerator::N_BITS);

        // Step b3: Change amplitude
        lut_type_t digi_val = _halfAmplitude ? lutIndex >> 1 : lutIndex;

        // Return the voltage value for the sample.
        return (uint8_t)digi_val;
    }
#endif

    void reset() override {
        // Reinitialize the phase to 0
        _phaseGenerator->reset();
        ESP_LOGI(TAG, "reset() called");
    }

private:
    /// @brief Flag to make half the amplitude
    bool _halfAmplitude = false;

    /// @brief Pointer to PhaseGenerator
    PhaseGenerator* _phaseGenerator = nullptr;

    /// @brief Function pointer to calculate the LUT index 
    LUTIndexFunction _getIndex = nullptr;

    /// @brief LUT : Select an LUT size from the predefined LUTSize's.
    lut_type_t _lut[LUT_256];

    /// @brief LUT size
    LUTSize _lutSize = LUT_256;
};

extern "C" {
    void app_main(void)
    {
        // Initialize waveform generator.
        tinyalg::waveu::ESP32Waveu<UserWaveConfig> waveu;

        // Define waveform arguments (e.g., frequency: 312.5 Hz).
        // This frequency corresponds to five waveform periods
        // within a 16 ms timer cycle, used for timing alignment.
        UserWaveArgs waveArgs(312.5f);

        try {
            // Configure the generator with arguments.
            waveu.configure(waveArgs);

            // Start waveform generation.
            waveu.start();

            // Keep the current frequency active for 20 seconds.
            vTaskDelay(pdMS_TO_TICKS(20000));

            // Stop waveform generation process.
            waveu.stop();

            // Reset the generator for another start.
            waveu.reset();
        } catch (const tinyalg::waveu::InvalidStateTransitionException& e) {
            ESP_LOGE(TAG, "Exception: %s\n", e.what());
        }
        
        // Log completion of the example.
        ESP_LOGI(TAG, "Waveform generation finished.");
    }
}
