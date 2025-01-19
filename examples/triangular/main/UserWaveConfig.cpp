#include <cstdint>
#include <utility> // For std::pair
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
    float amplitude;
    float offset;

    UserWaveArgs(float frequency, float amplitude, float offset)
        : frequency(frequency), amplitude(amplitude), offset(offset) {}
};

class UserWaveConfig : public tinyalg::waveu::WaveConfigInterface {
protected:
    /**
     * @brief Computes a triangular waveform value for a specific LUT index.
     * 
     * Generates a triangular waveform scaled by the pre-adjusted amplitude and 
     * centered at the pre-adjusted offset. The value alternates symmetrically 
     * within the range [offset - amplitude, offset + amplitude].
     * 
     * @param size The size of the lookup table (LUT), defining the number of samples.
     * @param index The LUT index for which the waveform value is calculated [0, size - 1].
     * @param amplitude The peak deviation of the waveform from the offset (pre-adjusted).
     * @param offset The center value of the waveform (pre-adjusted).
     * 
     * @return lut_type_t The waveform value for the given LUT index, scaled to [0, 255].
     * 
     * @note Assumes `amplitude` and `offset` are valid and pre-adjusted during initialization.
     */
    virtual lut_type_t fill_function_triangular(LUTSize size, size_t index, float amplitude, float offset) {
        size_t table_size = static_cast<size_t>(size);

        // Map index to phase within [0, 1)
        float phase = static_cast<float>(index) / (table_size - 1);

        // Generate the triangular value
        float triangular_value = (phase < 0.5)
            ? (2 * phase)                    // Rising edge
            : (2 * (1 - phase));             // Falling edge

        // Scale and shift to amplitude and offset
        return static_cast<lut_type_t>(
            amplitude * (2 * triangular_value - 1) + offset + 0.5
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

    virtual void populateLUTs(float amplitude, float offset) {
        size_t table_size = static_cast<size_t>(_lutSize);
        for (size_t i = 0; i < table_size; ++i) {
            _lut[i] = fill_function_triangular(_lutSize, i, amplitude, offset);
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

        // Sets the desired frequency and calculates the phase increment.
        setPhaseGeneratorFrequency(waveArgs.frequency);

        // Validates and adjusts amplitude and offset for the 8-bit DAC.
        auto [adjustedAmplitude, adjustedOffset] = LUTHelper::adjustAmplitudeAndOffset(waveArgs.amplitude, waveArgs.offset);

        // Fills the values to the LUT using the supplied function.
        populateLUTs(adjustedAmplitude, adjustedOffset);
    }

    void prepareCycle(double elapsedTime) override {
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

    void reset() override {
    }

private:
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
        // Initialize the waveform generator with a user-defined configuration.
        tinyalg::waveu::ESP32Waveu<UserWaveConfig> waveu;

        // Define the waveform arguments.
        float frequency = 293.66f; // D4 note
        float amplitude = 127.5f;
        float offset = 127.5f;
        UserWaveArgs waveArgs(frequency, amplitude, offset);

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
