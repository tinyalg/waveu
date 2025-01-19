#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace tinyalg::waveu {
    QueueHandle_t dataGenerationQueue;
    QueueHandle_t dataOutputQueue;
} // namespace tinyalg::waveu
