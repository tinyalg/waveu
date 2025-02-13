#pragma once

#include <stdint.h>

namespace tinyalg::waveu {

enum LUTSize {
    LUT_16 = 16,
    LUT_32 = 32,
    LUT_64 = 64,
    LUT_128 = 128,
    LUT_256 = 256,
    LUT_512 = 512,
    LUT_1024 = 1024,
    LUT_2048 = 2048,
};

#ifdef CONFIG_WAVEU_LUT_TYPE_INT16
    typedef int16_t lut_type_t;
#elif CONFIG_WAVEU_LUT_TYPE_UINT16
    typedef uint16_t lut_type_t;
#elif CONFIG_WAVEU_LUT_TYPE_UINT8
    typedef uint8_t lut_type_t;
#endif

typedef uint8_t data_buf_type_t;
typedef uint8_t sample_type_t;

typedef struct {
    bool data;
    bool terminationTrigger;
} data_generation_msg_type_t;

typedef struct {
    bool data;
    bool terminationTrigger;
} data_output_msg_type_t;

} // namespace tinyalg::waveu
