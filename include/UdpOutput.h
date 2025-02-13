#pragma once

namespace tinyalg::waveu {

class UdpOutput {
private:
    static int sock;
    static struct sockaddr_in dest_addr;
    static const char* TAG;
public:
    static void initialize();
    static void write(void* buffer, size_t size);
    static void stop();
};

}
