#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace tinyalg::waveu {
    extern QueueHandle_t dataGenerationQueue;
    extern QueueHandle_t dataOutputQueue;
} // namespace tinyalg::waveu
