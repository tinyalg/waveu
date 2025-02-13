#pragma once

#include <stdexcept>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "Semaphores.h"
#include "TaskDelete.h"
#include "DataTypes.h"
#include "Debug.h"
#include "InvalidStateTransitionException.h"
#include "DataTypes.h"
#include "Queues.h"
#include "WaveuHelper.h"

namespace tinyalg::waveu {

template <typename BoardConfig, typename WaveConfig, typename Wave2Config>
Waveu<BoardConfig, WaveConfig, Wave2Config>::Waveu() {

        // Initialization of optional second wave.
        if constexpr (!std::is_same<Wave2Config, void>::value) {
            chan2.initializeLUT();
        }
        
        WaveuHelper::initQueues();
        WaveuHelper::initSemaphore();

        // Initialize board-specific components (e.g., DAC, GPIO)
        brd.initializeDac();
        brd.setupGpio();
        chan.initialize(BoardConfig::SAMPLE_RATE);

        // Be sure to call prepareTimer() after allocateBufferArray()
        brd.prepareTimer();

        ESP_LOGI(TAG, "LEN_DATA_BUFFER=%d, SAMPLE_RATE=%d, TIMER_PERIOD=%d",
                 BoardConfig::LEN_DATA_BUFFER, (int)BoardConfig::SAMPLE_RATE, (int)BoardConfig::TIMER_PERIOD);

        UBaseType_t uxPriority = CONFIG_WAVEU_PRODUCER_TASK_PRIORITY;
        BaseType_t xCoreID = 1;
#ifdef CONFIG_WAVEU_PRODUCER_TASK_CORE_AFFINITY
        xCoreID = 0;
#endif

        xTaskCreatePinnedToCore(Waveu::waveformDataGenerationTask, "waveformDataGenerationTask", 4096, (void *)this, uxPriority, NULL, xCoreID);
        ESP_LOGI(TAG, "Started waveformDataGenerationTask on core %d at priority %d.",
                                                                         xCoreID, uxPriority);
    }

template <typename BoardConfig, typename WaveConfig, typename Wave2Config>
Waveu<BoardConfig, WaveConfig, Wave2Config>::~Waveu() {
    ESP_LOGD(TAG, "Running the destructor ~Waveu()...");
    brd.cleanupTimer();
    waveformDataGenerationTaskDelete();
    waveformDataOutputTaskDelete();

    // Clean up the queues.
    vQueueDelete(dataGenerationQueue);
    vQueueDelete(dataOutputQueue);
}

template <typename BoardConfig, typename WaveConfig, typename Wave2Config>
void Waveu<BoardConfig, WaveConfig, Wave2Config>::configure(const WaveConfigArgs& args) {
    if (currentState != State::Idle && currentState != State::Configured) {
        throw InvalidStateTransitionException(std::string("configure() after reset(): currentState=") + toString(currentState));
    }

    chan.configure(args);

    currentState = State::Configured;
}

template <typename BoardConfig, typename WaveConfig, typename Wave2Config>
void Waveu<BoardConfig, WaveConfig, Wave2Config>::start()
{
    if (currentState != State::Configured && currentState != State::Stopped) {
        throw InvalidStateTransitionException(std::string("start() after configure(), stop() or reset(): currentState=") + toString(currentState));
    }

    brd.startTimer();
    currentState = State::Running;
}

template <typename BoardConfig, typename WaveConfig, typename Wave2Config>
void Waveu<BoardConfig, WaveConfig, Wave2Config>::stop()
{
    if (currentState != State::Running) {
        throw InvalidStateTransitionException(std::string("stop() after start(): currentState=") + toString(currentState));
    }
    
    brd.stopTimer();

    currentState = State::Stopped;
}

template <typename BoardConfig, typename WaveConfig, typename Wave2Config>
void Waveu<BoardConfig, WaveConfig, Wave2Config>::reset() {
    if (currentState != State::Stopped) {
        throw InvalidStateTransitionException(std::string("reset() after stop(): currentState=") + toString(currentState));
    }

    chan.reset();
    brd.reset();
    elapsedTime = 0;

    currentState = State::Configured;
}

template <typename BoardConfig, typename WaveConfig, typename Wave2Config>
void Waveu<BoardConfig, WaveConfig, Wave2Config>::waveformDataGenerationTask(void *args) {
    Waveu* instance = static_cast<Waveu*>(args);

    data_generation_msg_type_t receivedData;
    while (1) {
        // Wait for notification, then calculate the next buffer.
        xQueueReceive(dataGenerationQueue, &receivedData, portMAX_DELAY);

        // When triggered, delete this task itself.
        if (receivedData.terminationTrigger) {
            ESP_LOGI(TAG, "Stopping waveformDataGenerationTask...");
            vTaskDelete(NULL);
        }

        constexpr double MICROSECONDS_TO_SECONDS = 1e-6;
        instance->chan.prepareCycle((double)instance->elapsedTime * MICROSECONDS_TO_SECONDS);

#ifdef CONFIG_WAVEU_CHANNEL_MODE_SIMUL
        int nSamples = BoardConfig::LEN_DATA_BUFFER; // Number of samples per milisecond
#endif
#ifdef CONFIG_WAVEU_CHANNEL_MODE_ALTER
        int nSamples = BoardConfig::LEN_DATA_BUFFER / 2; // Number of samples per milisecond
  #ifdef CONFIG_WAVEU_DATA_SINK_UDP
        nSamples /= 2;
  #endif
#endif

        if (receivedData.data) {
            sample_type_t *ptr = BoardConfig::pingDataBuffer.data;
            // Wait for semaphore to ensure the buffer is ready
            if (xSemaphoreTake(tinyalg::waveu::pingBufferSemaphore, portMAX_DELAY) == pdTRUE) {

                DEBUG_PRODUCER_GPIO_SET_LEVEL(1);

                for (int i = 0; i < nSamples; i++) {
                    sample_type_t outputValue = instance->chan.nextSample();
                    *ptr++ = outputValue;  // Channel 0 data
#ifdef CONFIG_WAVEU_CHANNEL_MODE_ALTER
                    outputValue = instance->chan.nextSampleB();
                    *ptr++ = outputValue;  // Channel 1 data
  #ifdef CONFIG_WAVEU_DATA_SINK_UDP
                    outputValue = instance->chan.nextSampleC();
                    *ptr++ = outputValue;  // Channel 1 data

                    outputValue = instance->chan.nextSampleD();
                    *ptr++ = outputValue;  // Channel 1 data
  #endif
#endif
                }

                DEBUG_PRODUCER_GPIO_SET_LEVEL(0);
                
                // Notify the consumer task that the new buffer is ready
                xSemaphoreGive(tinyalg::waveu::pingBufferSemaphore);
            }
        } else {
            sample_type_t *ptr = BoardConfig::pongDataBuffer.data;

            // Wait for semaphore to ensure the buffer is ready
            if (xSemaphoreTake(tinyalg::waveu::pongBufferSemaphore, portMAX_DELAY) == pdTRUE) {

                DEBUG_PRODUCER_GPIO_SET_LEVEL(1);

                for (int i = 0; i < nSamples; i++) {
                    sample_type_t outputValue = instance->chan.nextSample();
                    *ptr++ = outputValue;  // Channel 0 data
#ifdef CONFIG_WAVEU_CHANNEL_MODE_ALTER
                    outputValue = instance->chan.nextSampleB();
                    *ptr++ = outputValue;  // Channel 1 data
  #ifdef CONFIG_WAVEU_DATA_SINK_UDP
                    outputValue = instance->chan.nextSampleC();
                    *ptr++ = outputValue;  // Channel 1 data

                    outputValue = instance->chan.nextSampleD();
                    *ptr++ = outputValue;  // Channel 1 data
  #endif
#endif
                }

                DEBUG_PRODUCER_GPIO_SET_LEVEL(0);

                // Notify the consumer task that the new buffer is ready
                xSemaphoreGive(tinyalg::waveu::pongBufferSemaphore);
            }
        }

        instance->elapsedTime += BoardConfig::TIMER_PERIOD;
    } // while (1)
}

template <typename BoardConfig, typename WaveConfig, typename Wave2Config>
const char* Waveu<BoardConfig, WaveConfig, Wave2Config>::toString(State state) {
    switch (state) {
        case State::Idle:       return "Idle";
        case State::Configured: return "Configured";
        case State::Running:    return "Running";
        case State::Stopped:    return "Stopped";
        default:                return "Unknown";
    }
}

} // namespace tinyalg::waveu
