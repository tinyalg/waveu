#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace tinyalg::waveu {

SemaphoreHandle_t pingBufferSemaphore;

SemaphoreHandle_t pongBufferSemaphore;

}
