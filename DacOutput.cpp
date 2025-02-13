#include "esp_log.h"
#include "driver/dac_continuous.h"
#include "lwip/sockets.h" // for socket
#include "DacOutput.h"

namespace tinyalg::waveu {
    const char* DacOutput::TAG = "DacOutput";

    dac_continuous_handle_t DacOutput::_cont_handle = { 0 };

    void DacOutput::initialize(uint32_t sampleRate) {
            // ESP32-specific DAC initialization code
    dac_continuous_config_t cont_cfg = {

        #ifdef CONFIG_WAVEU_DAC_CHANNEL_BOTH
                .chan_mask = DAC_CHANNEL_MASK_ALL,
        #elif  CONFIG_WAVEU_DAC_CHANNEL_CH0
                .chan_mask = DAC_CHANNEL_MASK_CH0,
        #elif  CONFIG_WAVEU_DAC_CHANNEL_CH1
                .chan_mask = DAC_CHANNEL_MASK_CH1,
        #endif
                .desc_num = DAC_DMA_DESC_NUM,
                .buf_size = DAC_DMA_BUF_SIZE,
                .freq_hz = sampleRate,
                .offset = 0,
                .clk_src = DAC_DIGI_CLK_SRC_DEFAULT,     // If the frequency is out of range, try 'DAC_DIGI_CLK_SRC_APLL'
                /* Assume the data in buffer is 'A B C D E F'
                    * DAC_CHANNEL_MODE_SIMUL:
                    *      - channel 0: A B C D E F
                    *      - channel 1: A B C D E F
                    * DAC_CHANNEL_MODE_ALTER:
                    *      - channel 0: A C E
                    *      - channel 1: B D F
                    */
        #ifdef CONFIG_WAVEU_CHANNEL_MODE_SIMUL
                .chan_mode = DAC_CHANNEL_MODE_SIMUL,
        #elif  CONFIG_WAVEU_CHANNEL_MODE_ALTER
                .chan_mode = DAC_CHANNEL_MODE_ALTER,
        #endif
            };
        
            // Allocate continuous channel
            ESP_ERROR_CHECK(dac_continuous_new_channels(&cont_cfg, &_cont_handle));
            //Enable the channels in the group
            ESP_ERROR_CHECK(dac_continuous_enable(_cont_handle));
    }

    void DacOutput::write(void* buffer, size_t size) {
        size_t bytes_loaded;
        ESP_ERROR_CHECK(dac_continuous_write(_cont_handle,
                                            (uint8_t *)buffer,
                                            size,
                                            &bytes_loaded,
                                            DAC_TIMEOUT_MS));
        if (bytes_loaded != size) {
            ESP_LOGE(TAG, "Buffer loaded immaturely: bytes_loaded=%d", bytes_loaded);
        }
    }

    void DacOutput::cleanup() {
        ESP_ERROR_CHECK_WITHOUT_ABORT(dac_continuous_disable(_cont_handle));
        ESP_ERROR_CHECK_WITHOUT_ABORT(dac_continuous_del_channels(_cont_handle));
    }
}
