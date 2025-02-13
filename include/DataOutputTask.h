#pragma once

#include "ESP32Config.h"

namespace tinyalg::waveu {

extern data_transfer_task_args_t data_transfer_task_args;
extern void waveformDataOutputTask(void *args);

}
