#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "Queues.h"
#include "Semaphores.h"
#include "ESP32Config.h"
#include "Debug.h"
#ifdef CONFIG_WAVEU_DATA_SINK_DAC
#include "DacOutput.h"
#else /* CONFIG_WAVEU_DATA_SINK_UDP */
#include "UdpOutput.h"
#endif

namespace tinyalg::waveu {

data_transfer_task_args_t data_transfer_task_args;
void waveformDataOutputTask(void *args) {
    data_transfer_task_args_t *task_args = (data_transfer_task_args_t*)args;

  #ifdef CONFIG_WAVEU_DATA_SINK_DAC
    DacOutput::initialize(ESP32Config::SAMPLE_RATE);
  #else /* CONFIG_WAVEU_DATA_SINK_UDP */
    UdpOutput::initialize();
  #endif

    static uint32_t packet_counter = 0;
    while (1) {
        data_output_msg_type_t receivedData;
        // Wait for notification, then calculate the next buffer.
        xQueueReceive(dataOutputQueue, &receivedData, portMAX_DELAY);

        // When triggered, delete this task itself.
        if (receivedData.terminationTrigger) {
            ESP_LOGI(ESP32Config::TAG, "Stopping waveformDataOutputTask...");
          #ifdef CONFIG_WAVEU_DATA_SINK_DAC
            DacOutput::cleanup();
          #endif

            vTaskDelete(NULL);
        }

        if (receivedData.data) {
            // Wait for the producer to fill the current buffer
            if (xSemaphoreTake(pingBufferSemaphore, portMAX_DELAY) == pdTRUE) {
                DEBUG_CONSUMER_GPIO_SET_LEVEL(1);
              #ifdef CONFIG_WAVEU_DATA_SINK_DAC
                DacOutput::write((uint8_t *)ESP32Config::pingDataBuffer.data, ESP32Config::LEN_DATA_BUFFER);
              #else /* CONFIG_WAVEU_DATA_SINK_UDP */
                ESP32Config::pingDataBuffer.packet_count = packet_counter++;  // Increment packet count
                ESP32Config::pingDataBuffer.timestamp = esp_timer_get_time();  // Capture timestamp
                UdpOutput::write(&ESP32Config::pingDataBuffer, sizeof(ESP32Config::pingDataBuffer));
              #endif
                DEBUG_CONSUMER_GPIO_SET_LEVEL(0);
                // Notify the producer that the current buffer is ready for refill
                xSemaphoreGive(pingBufferSemaphore);
            }
        } else {
            // Wait for the producer to fill the current buffer
            if (xSemaphoreTake(pongBufferSemaphore, portMAX_DELAY) == pdTRUE) {

                DEBUG_CONSUMER_GPIO_SET_LEVEL(1);
              #ifdef CONFIG_WAVEU_DATA_SINK_DAC
                DacOutput::write((uint8_t *)ESP32Config::pongDataBuffer.data, ESP32Config::LEN_DATA_BUFFER);
              #else /* CONFIG_WAVEU_DATA_SINK_UDP */
                ESP32Config::pongDataBuffer.packet_count = packet_counter++;  // Increment packet count
                ESP32Config::pongDataBuffer.timestamp = esp_timer_get_time();  // Capture timestamp
                UdpOutput::write(&ESP32Config::pongDataBuffer, sizeof(ESP32Config::pongDataBuffer));
              #endif
                DEBUG_CONSUMER_GPIO_SET_LEVEL(0);
                // Notify the producer that the current buffer is ready for refill
                xSemaphoreGive(pongBufferSemaphore);
            }
        }
    } // while (1)
}

}
