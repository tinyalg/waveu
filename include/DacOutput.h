#pragma once

#include "driver/dac_continuous.h"

namespace tinyalg::waveu {

class DacOutput {
private:
    static const char* TAG;
    static dac_continuous_handle_t _cont_handle;

    static constexpr int DAC_DMA_DESC_NUM = 18;
    static constexpr int DAC_DMA_BUF_SIZE = 4000;
    static constexpr int DAC_TIMEOUT_MS = -1; // Block forever

public:
    static void initialize(uint32_t sampleRate);
    static void write(void* buffer, size_t size, int timeout_ms);
    static void write(void* buffer, size_t size);
    static void cleanup();
};

}
