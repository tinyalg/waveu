#include <stdatomic.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/dac_continuous.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "BoardConfig.h"
#include "DataTypes.h"
#include "ESP32Config.h"
#include "Queues.h"
#include "Semaphores.h"
#include "DataTypes.h"
#include "Debug.h"

namespace tinyalg::waveu {

atomic_bool timer_callback_stop_request = false;
atomic_bool discard_buffer_before_stop = false;

static data_transfer_task_args_t data_transfer_task_args;
static void waveformDataOutputTask(void *args) {
    data_transfer_task_args_t *task_args = (data_transfer_task_args_t*)args;

    while (1) {
        data_output_msg_type_t receivedData;
        // Wait for notification, then calculate the next buffer.
        xQueueReceive(dataOutputQueue, &receivedData, portMAX_DELAY);

        // When triggered, delete this task itself.
        if (receivedData.terminationTrigger) {
            ESP_LOGI(ESP32Config::TAG, "Stopping waveformDataOutputTask...");
            vTaskDelete(NULL);
        }

        if (receivedData.data) {
            // Wait for the producer to fill the current buffer
            if (xSemaphoreTake(pingBufferSemaphore, portMAX_DELAY) == pdTRUE) {
                DEBUG_CONSUMER_GPIO_SET_LEVEL(1);

                size_t bytes_loaded;
                ESP_ERROR_CHECK(dac_continuous_write(task_args->cont_handle,
                                                    (uint8_t *)ESP32Config::pingDataBuffer,
                                                    ESP32Config::LEN_DATA_BUFFER,
                                                    &bytes_loaded,
                                                    task_args->timeout_ms));
                if (bytes_loaded != ESP32Config::LEN_DATA_BUFFER) {
                    ESP_LOGE(ESP32Config::TAG, "pingDataBuffer loaded immaturely: bytes_loaded=%d", bytes_loaded);
                }

                DEBUG_CONSUMER_GPIO_SET_LEVEL(0);
                // Notify the producer that the current buffer is ready for refill
                xSemaphoreGive(pingBufferSemaphore);
            }
        } else {
            // Wait for the producer to fill the current buffer
            if (xSemaphoreTake(pongBufferSemaphore, portMAX_DELAY) == pdTRUE) {

                DEBUG_CONSUMER_GPIO_SET_LEVEL(1);

                size_t bytes_loaded;        
                ESP_ERROR_CHECK(dac_continuous_write(task_args->cont_handle,
                                                    (uint8_t *)ESP32Config::pongDataBuffer,
                                                    ESP32Config::LEN_DATA_BUFFER,
                                                    &bytes_loaded,
                                                    task_args->timeout_ms));
                if (bytes_loaded != ESP32Config::LEN_DATA_BUFFER) {
                    ESP_LOGE(ESP32Config::TAG, "pongDataBuffer loaded immaturely: bytes_loaded=%d", bytes_loaded);
                }

                DEBUG_CONSUMER_GPIO_SET_LEVEL(0);
                // Notify the producer that the current buffer is ready for refill
                xSemaphoreGive(pongBufferSemaphore);
            }
        }
    } // while (1)
}

esp_timer_handle_t ESP32Config::timer_handle = nullptr;

const char* ESP32Config::TAG = "Waveu-ESP32Config";

data_buf_type_t ESP32Config::pingDataBuffer[LEN_DATA_BUFFER] = { 0 }; 
data_buf_type_t ESP32Config::pongDataBuffer[LEN_DATA_BUFFER] = { 0 };

ESP32Config::timer_callback_args_t ESP32Config::timer_callback_args = {};

ESP32Config::ESP32Config() {}

ESP32Config::~ESP32Config() {
    ESP_LOGD(TAG, "Running the destructor ~ESP32Config()...");
    ESP_ERROR_CHECK_WITHOUT_ABORT(dac_continuous_disable(cont_handle));
    ESP_ERROR_CHECK_WITHOUT_ABORT(dac_continuous_del_channels(cont_handle));
}

// Implementation of ESP32-specific DAC initialization
void ESP32Config::initializeDac() {
    // ESP32-specific DAC initialization code
    dac_continuous_config_t cont_cfg = {

#ifdef CONFIG_WAVEU_DAC_CHANNEL_BOTH
        .chan_mask = DAC_CHANNEL_MASK_ALL,
#elif  CONFIG_WAVEU_DAC_CHANNEL_CH0
        .chan_mask = DAC_CHANNEL_MASK_CH0,
#elif  CONFIG_WAVEU_DAC_CHANNEL_CH1
        .chan_mask = DAC_CHANNEL_MASK_CH1,
#endif
        .desc_num = ESP32Config::DAC_DMA_DESC_NUM,
        .buf_size = ESP32Config::DAC_DMA_BUF_SIZE,
        .freq_hz = ESP32Config::SAMPLE_RATE,
        .offset = 0,
        .clk_src = DAC_DIGI_CLK_SRC_DEFAULT,     // If the frequency is out of range, try 'DAC_DIGI_CLK_SRC_APLL'
        /* Assume the data in buffer is 'A B C D E F'
            * DAC_CHANNEL_MODE_SIMUL:
            *      - channel 0: A B C D E F
            *      - channel 1: A B C D E F
            * DAC_CHANNEL_MODE_ALTER:
            *      - channel 0: A C E
            *      - channel 1: B D F
            */
#ifdef CONFIG_WAVEU_CHANNEL_MODE_SIMUL
        .chan_mode = DAC_CHANNEL_MODE_SIMUL,
#elif  CONFIG_WAVEU_CHANNEL_MODE_ALTER
        .chan_mode = DAC_CHANNEL_MODE_ALTER,
#endif
    };

    // Allocate continuous channel
    ESP_ERROR_CHECK(dac_continuous_new_channels(&cont_cfg, &cont_handle));
    //Enable the channels in the group
    ESP_ERROR_CHECK(dac_continuous_enable(cont_handle));
}

// Implementation of ESP32-specific GPIO setup
void ESP32Config::setupGpio() {
    // ESP32-specific GPIO setup code
    // For instance, configure any GPIOs that need to be set up for DAC output

    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;

    uint64_t pin_bit_mask = 0;
#ifdef CONFIG_WAVEU_DEBUG_PRODUCER_GPIO
    pin_bit_mask |= 1ULL<<CONFIG_WAVEU_DEBUG_PRODUCER_GPIO_NUM;
#endif

#ifdef CONFIG_WAVEU_DEBUG_CONSUMER_GPIO
    pin_bit_mask |= 1ULL<<CONFIG_WAVEU_DEBUG_CONSUMER_GPIO_NUM;
#endif

    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = pin_bit_mask;

    //disable pull-down mode
    io_conf.pull_down_en = (gpio_pulldown_t)0;
    //disable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t)0;

    if (pin_bit_mask != 0) {
        //configure GPIO with the given settings
        gpio_config(&io_conf);
    }
}

/// @brief 
void ESP32Config::prepareTimer() {

    timer_callback_args.cont_handle = cont_handle;
    timer_callback_args.sampleRate = 42;
    timer_callback_args.lenDataBuffer = LEN_DATA_BUFFER;

    // Create & start a timer.
    esp_timer_create_args_t timer_args = {
        .callback = &ESP32Config::timerCallback,
        .arg = &timer_callback_args,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "DAC Timer",
        .skip_unhandled_events = false,
    };

    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &ESP32Config::timer_handle));

    // Invoke the consumer task.
    int foever = -1;
    data_transfer_task_args.timeout_ms = foever;
    data_transfer_task_args.cont_handle = cont_handle;
    UBaseType_t uxPriority = CONFIG_WAVEU_CONSUMER_TASK_PRIORITY;
    BaseType_t xCoreID = 1;
#ifdef CONFIG_WAVEU_CONSUMER_TASK_CORE_AFFINITY
    xCoreID = 0;
#endif
    xTaskCreatePinnedToCore(waveformDataOutputTask, "waveformDataOutputTask", 4096, (void *)&data_transfer_task_args, uxPriority, NULL, xCoreID);
    ESP_LOGI(TAG, "waveformDataOutputTask to on core %d at priority %d.",
                                                                        xCoreID, uxPriority);
}

void ESP32Config::startTimer() {
    // Disable stop request
    atomic_store(&timer_callback_stop_request, false);

    // Start the timer
    ESP_ERROR_CHECK(esp_timer_start_periodic(ESP32Config::timer_handle, TIMER_PERIOD));

    // Wait for some time to ensure waveform output before immediate stop()
    vTaskDelay(pdMS_TO_TICKS(TIMER_PERIOD / 1000) * 3 + 1);
}

void ESP32Config::stopTimer() {
    // Signal stop request
    atomic_store(&timer_callback_stop_request, true);

    // Wait for one timer period to ensure the callback is not running
    vTaskDelay(pdMS_TO_TICKS(TIMER_PERIOD / 1000) + 1);

    // Stop the timer
    ESP_ERROR_CHECK(esp_timer_stop(ESP32Config::timer_handle));
}

void ESP32Config::cleanupTimer() {
    //ESP_ERROR_CHECK_WITHOUT_ABORT(esp_timer_stop(ESP32Config::timer_handle));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_timer_delete(ESP32Config::timer_handle));
}

void ESP32Config::reset() {
    ESP_LOGD(TAG, "reset() called");
    // Signal how the data buffer is handled.
    atomic_store(&discard_buffer_before_stop, true);
}

void ESP32Config::timerCallback(void *args) {
    //timer_callback_args_t* callback_args = static_cast<timer_callback_args_t*>(args);

    static bool firstTime = true;
    static bool prepareData0 = true;  // Selector of double buffer
    static int callCount = 0;
    callCount++;

    if (atomic_load(&timer_callback_stop_request)) {
        // Gracefully exit if stop is requested
        return;
    }

    // Send which buffer to use as queue
    // Prepare data for the next call.
    data_generation_msg_type_t data_generation_msg = {
        .data = prepareData0,
        .terminationTrigger = false,
    };
    if (xQueueSend(dataGenerationQueue, (void *)&data_generation_msg, portMAX_DELAY) != pdPASS) {
        ESP_LOGW(TAG, "Queue is full. Data drop occurred.(%d)", callCount);
    }

    // Switch to the new data filled by previous request
    prepareData0 = !prepareData0;

    if (firstTime || atomic_load(&discard_buffer_before_stop)) {
        firstTime = false;
        atomic_store(&discard_buffer_before_stop, false);
        ESP_LOGD(TAG, "Skipped the alternate data buffer immediately after start()");
        return;
    } else {
        data_output_msg_type_t data_output_msg = {
            .data = prepareData0,
            .terminationTrigger = false,
        };
        if (xQueueSend(dataOutputQueue, (void *)&data_output_msg, portMAX_DELAY) != pdPASS) {
            ESP_LOGW(TAG, "Queue is full. Data drop occurred.(%d)", callCount);
        }
    }
}
    
} // namespace tinyalg::waveu
