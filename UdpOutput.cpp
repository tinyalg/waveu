#include "esp_log.h"
#include "lwip/sockets.h" // for socket
#include "UdpOutput.h"

#define UDP_PORT       8000
#define UDP_IP         "192.168.0.2"  // Change this to your PC's local IP

namespace tinyalg::waveu {
    const char* UdpOutput::TAG = "UdpOutput";

    int UdpOutput::sock = 0;

    struct sockaddr_in UdpOutput::dest_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(8000),
        .sin_addr = {INADDR_ANY}
    };

    void UdpOutput::initialize() {
        // Initialize UDP socket
        dest_addr.sin_addr.s_addr = inet_addr(UDP_IP);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(UDP_PORT);

        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(UdpOutput::TAG, "Failed to create socket");
            vTaskDelete(NULL);
            return;
        }

        ESP_LOGI(UdpOutput::TAG, "UDP socket created, sending to %s:%d", UDP_IP, UDP_PORT);
    }

    void UdpOutput::write(void* buffer, size_t size) {
        // Send UDP packet
        int sent_bytes = sendto(sock, buffer, size, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (sent_bytes < 0) {
            ESP_LOGE(UdpOutput::TAG, "Failed to send UDP packet");
        } else {
            //ESP_LOGI(TAG, "Sent %d sine wave samples via UDP", BATCH_SIZE);
        }
    }

    void UdpOutput::stop() {
    }

}
