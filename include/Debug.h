#pragma once

#include "driver/gpio.h"
#include "sdkconfig.h"

#ifdef CONFIG_WAVEU_DEBUG_PRODUCER_GPIO
#define DEBUG_PRODUCER_GPIO CONFIG_WAVEU_DEBUG_PRODUCER_GPIO_NUM
#define DEBUG_PRODUCER_GPIO_SET_LEVEL(level) ESP_ERROR_CHECK(gpio_set_level((gpio_num_t)DEBUG_PRODUCER_GPIO, (uint32_t)(level)))
#else
#define DEBUG_PRODUCER_GPIO_SET_LEVEL(level) ((void)0) // No-op
#endif

#ifdef CONFIG_WAVEU_DEBUG_CONSUMER_GPIO
#define DEBUG_CONSUMER_GPIO CONFIG_WAVEU_DEBUG_CONSUMER_GPIO_NUM
#define DEBUG_CONSUMER_GPIO_SET_LEVEL(level) ESP_ERROR_CHECK(gpio_set_level((gpio_num_t)DEBUG_CONSUMER_GPIO, (uint32_t)(level)))
#else
#define DEBUG_CONSUMER_GPIO_SET_LEVEL(level) ((void)0) // No-op
#endif
