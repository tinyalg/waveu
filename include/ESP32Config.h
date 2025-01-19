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
    dac_continuous_handle_t cont_handle;
    int timeout_ms;
} data_transfer_task_args_t;

// Example: ESP32Config specialization for ESP32
class ESP32Config : public BoardConfigInterface {
public:
    static constexpr uint32_t SAMPLE_RATE = 1000 * KILO; // Sa/s
    static constexpr uint32_t TIMER_PERIOD = 16 * KILO; // us
    static constexpr int DAC_DMA_DESC_NUM = 18;
    static constexpr int DAC_DMA_BUF_SIZE = 4000;

    static constexpr size_t LEN_DATA_BUFFER =
                    (SAMPLE_RATE / KILO) * (TIMER_PERIOD / KILO)
#ifdef CONFIG_WAVEU_CHANNEL_MODE_ALTER
                  * NUM_CHANNELS
#endif
                  * sizeof(uint8_t);    // Length of each buffer

    static data_buf_type_t pingDataBuffer[LEN_DATA_BUFFER];
    static data_buf_type_t pongDataBuffer[LEN_DATA_BUFFER];

private:

    dac_continuous_handle_t cont_handle;

    void initSemaphore();

    typedef struct {
        dac_continuous_handle_t cont_handle;
        uint32_t sampleRate;
        uint32_t lenDataBuffer;
    } timer_callback_args_t;

    static timer_callback_args_t timer_callback_args;

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
