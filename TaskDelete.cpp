#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "Queues.h"
#include "DataTypes.h"
#include "TaskDelete.h"

namespace tinyalg::waveu {

static const char* TAG = "TaskDelete";

void waveformDataGenerationTaskDelete() {
    data_generation_msg_type_t task_termination_msg = {
        .data = false, // Whichever ture or false
        .terminationTrigger = true, // Signals termination
    };

    if (xQueueSend(dataGenerationQueue, (void *)&task_termination_msg, pdMS_TO_TICKS(1000)) != pdPASS) {
        ESP_LOGW(TAG, "Requesting termination of waveformDataGenerationTask failed.");
    }
}

void waveformDataOutputTaskDelete() {
    data_output_msg_type_t task_termination_msg = {
        .data = false, // Whichever ture or false
        .terminationTrigger = true, // Signals termination
    };

    if (xQueueSend(dataOutputQueue, (void *)&task_termination_msg, pdMS_TO_TICKS(1000)) != pdPASS) {
        ESP_LOGW(TAG, "Requesting termination of waveformDataOutputTask failed.");
    }
}

}
