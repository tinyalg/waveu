#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace tinyalg::waveu {

extern SemaphoreHandle_t pingBufferSemaphore;

extern SemaphoreHandle_t pongBufferSemaphore;

}
