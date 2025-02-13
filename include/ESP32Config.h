#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/dac_continuous.h"
#include "esp_timer.h"
#include "BoardConfigInterface.h"
#include "DataTypes.h"
#include "sdkconfig.h"

#define KILO   ( 1000 )
#define NUM_CHANNELS    ( 2 )

namespace tinyalg::waveu {

typedef struct {
    int timeout_ms;
} data_transfer_task_args_t;

// Example: ESP32Config specialization for ESP32
class ESP32Config : public BoardConfigInterface {
public:
    // for DAC
#ifdef CONFIG_WAVEU_DATA_SINK_DAC
    static constexpr uint32_t SAMPLE_RATE = 1000 * KILO; // Sa/s
    static constexpr uint32_t TIMER_PERIOD = 16 * KILO; // us; for DAC
#else /* CONFIG_WAVEU_DATA_SINK_UDP */
    // for UDP
    static constexpr uint32_t SAMPLE_RATE = 96 * KILO; // Sa/s
    static constexpr uint32_t TIMER_PERIOD = 16 * KILO; // us; for UDP
#endif
    static constexpr size_t LEN_DATA_BUFFER =
                    (SAMPLE_RATE / KILO) * (TIMER_PERIOD / KILO)
#ifdef CONFIG_WAVEU_CHANNEL_MODE_ALTER
                  * NUM_CHANNELS
  #ifdef CONFIG_WAVEU_DATA_SINK_UDP
                  * 2
  #endif
#endif
                  * sizeof(sample_type_t);    // Length of each buffer

    typedef struct {
        uint32_t packet_count;  // Simple counter (increments per packet)
        uint64_t timestamp;  // Microsecond timestamp
        sample_type_t data[LEN_DATA_BUFFER]; // Actual waveform data
    } __attribute__((packed)) data_packet_type_t;

    static data_packet_type_t pingDataBuffer;
    static data_packet_type_t pongDataBuffer;

private:
    void initSemaphore();

public:
    static const char* TAG;

    static esp_timer_handle_t timer_handle;

    ESP32Config();
    ~ESP32Config();

    void initializeDac() override;     // Initialize DAC
    void setupGpio() override;         // Setup GPIO pins for DAC output
    void prepareTimer() override;
    void startTimer() override;
    void stopTimer() override;
    void cleanupTimer() override;
    void reset() override;
    static void timerCallback(void *args);
};

} // namespace tinyalg::waveu
