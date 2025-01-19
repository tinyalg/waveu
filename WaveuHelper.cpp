#include "esp_log.h"

#include "Semaphores.h"
#include "Queues.h"
#include "DataTypes.h"
#include "WaveuHelper.h"

namespace tinyalg::waveu {

const char* WaveuHelper::TAG = "WaveuHelper";

bool WaveuHelper::initQueues() {
    size_t queueLength = 1;
    dataGenerationQueue = xQueueCreate(queueLength, sizeof(data_generation_msg_type_t));
    if (dataGenerationQueue == 0) {
        ESP_LOGE(TAG, "dataGenerationQueue cannot be created.");
        return false;
    }
    
    queueLength = 10;
    dataOutputQueue = xQueueCreate(queueLength, sizeof(data_output_msg_type_t));
    if (dataOutputQueue == 0) {
        ESP_LOGE(TAG, "dataOutputQueue cannot be created.");
        return false;
    }

    return true;
}

void WaveuHelper::initSemaphore()
{
    pingBufferSemaphore = xSemaphoreCreateMutex();
    if (pingBufferSemaphore == NULL) {
        ESP_LOGE(TAG, "semaphore creation failure");
    }

    pongBufferSemaphore = xSemaphoreCreateMutex();
    if (pongBufferSemaphore == NULL) {
        ESP_LOGE(TAG, "semaphore creation failure");
    }
}

} // namespace tinyalg::waveu
