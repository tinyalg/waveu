#include <cstdint>
#include <cmath>   // For sin, M_PI
#include <tuple>  // For std::tuple
#include <algorithm>  // For std::max
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "ESP32Waveu.h"

// For a function station_example_main.c
#ifdef __cplusplus
extern "C" {
#endif

void wifi_init_sta(void);

#ifdef __cplusplus
}
#endif

using tinyalg::waveu::PhaseGenerator;
using tinyalg::waveu::LUTIndexFunction;
using tinyalg::waveu::LUTHelper;
using tinyalg::waveu::LUTSize;
using tinyalg::waveu::lut_type_t;
using tinyalg::waveu::LUT_1024;

static const char* TAG = "UserWaveConfig";

/// @brief LUT : Select an LUT size from the predefined LUTSize's.
lut_type_t lutBase[LUT_1024];
lut_type_t lutDouble[LUT_1024];
lut_type_t lutTriple[LUT_1024];

// Derived class for user wave arguments
class UserWaveArgs : public tinyalg::waveu::WaveConfigArgs {
public:
    float amplitudeB;
    float amplitudeD;
    float amplitudeT;

    UserWaveArgs(float amplitudeB, float amplitudeD, float amplitudeT)
        : amplitudeB(amplitudeB), amplitudeD(amplitudeD), amplitudeT(amplitudeT) {}
};

class UserWaveConfig : public tinyalg::waveu::WaveConfigInterface {
protected:

    virtual std::tuple<float, float, float> adjustAmplitudes(float amplitudeB, float amplitudeD, float amplitudeT) {
        // Ensure amplitudes are not negative
        amplitudeB = std::max(0.0f, amplitudeB);
        amplitudeD = std::max(0.0f, amplitudeD);
        amplitudeT = std::max(0.0f, amplitudeT);

        // Compute sum of amplitudes
        float sum = amplitudeB + amplitudeD + amplitudeT;

        // Normalize if sum exceeds 1.0
        if (sum > 1.0f) {
            float scaleFactor = 1.0f / sum;
            amplitudeB *= scaleFactor;
            amplitudeD *= scaleFactor;
            amplitudeT *= scaleFactor;
        }

        return std::make_tuple(amplitudeB, amplitudeD, amplitudeT);
    }

    virtual lut_type_t fill_function_sine(LUTSize size, size_t index, float amplitude = 127.5, float offset = 127.5, float delta = 0) {
        size_t table_size = static_cast<size_t>(size);
        return static_cast<lut_type_t>(
            amplitude * sin(index * 2 * M_PI / (table_size - 1) + delta) + offset + 0.5
        );
    }

    virtual void initializePhaseGenerator(uint32_t sampleRate) {
        if (_phaseGeneratorR == nullptr) {
            _phaseGeneratorR = new PhaseGenerator(sampleRate);
        }
        if (_phaseGeneratorT == nullptr) {
            _phaseGeneratorT = new PhaseGenerator(sampleRate);
        }
        if (_phaseGeneratorF == nullptr) {
            _phaseGeneratorF = new PhaseGenerator(sampleRate);
        }
    }

    virtual void setPhaseGeneratorFrequency(float frequencyR, float frequencyT, float frequencyF) {
        _phaseGeneratorR->setFrequency(frequencyR);
        _phaseGeneratorT->setFrequency(frequencyT);
        _phaseGeneratorF->setFrequency(frequencyF);
        ESP_LOGI(TAG, "Frequency set to %.02fHz, %.02fHz and %.02fHz", _phaseGeneratorR->getFrequency(), 
                                                                       _phaseGeneratorT->getFrequency(), 
                                                                       _phaseGeneratorF->getFrequency());
    }

    virtual void populateLUTs(float amplitudeB, float amplitudeD, float amplitudeT) {
        constexpr float max_amplitude = 127.5f;
        constexpr float offset = 127.5f;
        size_t table_size = static_cast<size_t>(_lutSize);
        for (size_t i = 0; i < table_size; ++i) {
            lutBase[i] = fill_function_sine(_lutSize, i, max_amplitude * amplitudeB, offset);
            lutDouble[i] = fill_function_sine(_lutSize, i, max_amplitude * amplitudeD, offset);
            lutTriple[i] = fill_function_sine(_lutSize, i, max_amplitude * amplitudeT, offset);
        }
    }

public:
    ~UserWaveConfig() override {
        delete _phaseGeneratorR; // Clean up
        delete _phaseGeneratorF; // Clean up
        delete _phaseGeneratorT; // Clean up
    }

    void initialize(uint32_t sampleRate) override {
        // Initializes the PhaseGenerator with the specified sample rate.
        initializePhaseGenerator(sampleRate);

        // Retrieves a function to calculate the LUT index from a phase.
        _getIndex = LUTHelper::getIndexFunction(_lutSize);
    }

    void configure(const tinyalg::waveu::WaveConfigArgs& args) override {
        const auto& waveArgs = dynamic_cast<const UserWaveArgs&>(args); // Safe because caller ensures type

        float amplitudeB = 0.0f;
        float amplitudeD = 0.0f;
        float amplitudeT = 0.0f;

        // Adjusts amplitudes so that the sum <= 1.0.
        std::tie(amplitudeB, amplitudeD, amplitudeT) = adjustAmplitudes(waveArgs.amplitudeB, waveArgs.amplitudeD, waveArgs.amplitudeT);

        // Fills the values to the LUT using the supplied function.
        populateLUTs(amplitudeB, amplitudeD, amplitudeT);

        setPhaseGeneratorFrequency(698.46f, 698.46f*2.0, 698.46f*3.0); // F5, F6, F7
    }

    void prepareCycle(double elapsedTime) override {
        static float previousToneChangeTime = 0.0f;
        const int chordDuration = 1.0f; // 1 second per chord
        static int toneIndex = 0;
        const int numTones = 4;

        if (elapsedTime - previousToneChangeTime > chordDuration) {
            toneIndex = (toneIndex + 1) % numTones; // Cycle through C5 -> E5 -> G5 -> C5E5G5 -> C5
            switch (toneIndex) {
            case 0:
                _notesMask = 0b100;  // C5 ON, E5 OFF, G5 OFF
                break;
            case 1:
                _notesMask = 0b010;
                break;
            case 2:
                _notesMask = 0b111;
                break;
            default:
                _notesMask = 0b001;
                break;
            }
            //ESP_LOGI(TAG, "_notesMask=%d", _notesMask);
            previousToneChangeTime = elapsedTime;
        }
    }

    tinyalg::waveu::sample_type_t nextSample() override {
        // Step 1: Advance the phase to the next position.
        _phaseGeneratorR->updatePhase();

        // Step 2: Retrieve the updated phase value.
        uint32_t currentPhaseR = _phaseGeneratorR->getPhase();

        // Step 3: Map the phase value to an appropriate index in the lookup table.
        int lutIndexR = _getIndex(currentPhaseR, PhaseGenerator::N_BITS);

        // Step 4: Fetch the corresponding voltage value (0-255) from the lookup table using the macro.
        lut_type_t base_val = GET_LUT_VALUE(lutBase, lutIndexR);

        _digi_val[0] = base_val;

        // Return the voltage value for the sample.
        return (uint8_t)_digi_val[0];
    }

    tinyalg::waveu::sample_type_t nextSampleB() override {
        // Step 1: Advance the phase to the next position.
        _phaseGeneratorT->updatePhase();

        // Step 2: Retrieve the updated phase value.
        uint32_t currentPhase = _phaseGeneratorT->getPhase();

        // Step 3: Map the phase value to an appropriate index in the lookup table.
        int lutIndex = _getIndex(currentPhase, PhaseGenerator::N_BITS);

        // Step 4: Fetch the corresponding voltage value (0-255) from the lookup table using the macro.
        lut_type_t double_val = GET_LUT_VALUE(lutDouble, lutIndex);

        _digi_val[1] = double_val;

        // Return the voltage value for the sample.
        return (uint8_t)_digi_val[1];
    }

#ifdef CONFIG_WAVEU_DATA_SINK_UDP
    tinyalg::waveu::sample_type_t nextSampleC() override {
        // Step 1: Advance the phase to the next position.
        _phaseGeneratorF->updatePhase();

        // Step 2: Retrieve the updated phase value.
        uint32_t currentPhaseF = _phaseGeneratorF->getPhase();

        // Step 3: Map the phase value to an appropriate index in the lookup table.
        int lutIndexF = _getIndex(currentPhaseF, PhaseGenerator::N_BITS);

        // Step 4: Fetch the corresponding voltage value (0-255) from the lookup table using the macro.
        lut_type_t triple_val = GET_LUT_VALUE(lutTriple, lutIndexF);

        _digi_val[2] = triple_val;

        // Return the voltage value for the sample.
        return (uint8_t)_digi_val[2];
    }

    tinyalg::waveu::sample_type_t nextSampleD() override {
        return (uint8_t)_digi_val[0] + _digi_val[1] + _digi_val[2];
    }
#endif

    void reset() override {
    }

private:
    /// @brief 3-bit integer for note states
    uint8_t _notesMask = 0b100;  // C5 ON, E5 OFF, G5 OFF

    /// @brief 
    uint8_t _digi_val[3];

    /// @brief Initial frequency
    float _f_start;

    /// @brief Final frequency
    float _f_end;

    /// @brief Total sweep duration
    float _t_duration;

    /// @brief Pointer to PhaseGenerator for linear sweep
    PhaseGenerator* _phaseGeneratorR = nullptr;

    /// @brief Pointer to PhaseGenerator for exponential sweep
    PhaseGenerator* _phaseGeneratorT = nullptr;

    /// @brief Pointer to PhaseGenerator for linear sweep
    PhaseGenerator* _phaseGeneratorF = nullptr;

    /// @brief Function pointer to calculate the LUT index 
    LUTIndexFunction _getIndex = nullptr;

    /// @brief LUT size
    LUTSize _lutSize = LUT_1024;
};

extern "C" {
    void app_main(void)
    {
        //Initialize NVS
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);

        ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
        wifi_init_sta();

        // Initialize the waveform generator with a user-defined configuration.
        //M5StickCPlusWaveu<UserWaveConfig> waveu;
        tinyalg::waveu::ESP32Waveu<UserWaveConfig> waveu;

        float amplitudeB = 1.0;
        float amplitudeD = 1.0;
        float amplitudeT = 1.0;

        UserWaveArgs waveArgs(amplitudeB, amplitudeD, amplitudeT);

        try {
            // Configure the waveform generator once before the loop.
            waveu.configure(waveArgs);
            ESP_LOGI(TAG, "Running Sweep Example");

            // Start the waveform generation process.
            waveu.start();
        } catch (const tinyalg::waveu::InvalidStateTransitionException& e) {
            ESP_LOGE(TAG, "Exception: %s", e.what());
        }

        // Prevent app_main() from exiting, keeping local variables in memory.
        vTaskDelay(portMAX_DELAY);
    }
}
