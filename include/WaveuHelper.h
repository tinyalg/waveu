#pragma once

namespace tinyalg::waveu {

class WaveuHelper {
public:
    static const char* TAG;

    /**
     * @brief Initialize the queues.
     * 
     * @return true if all the queues are successfully created.
     * @return false if at least one of the queues cannot be created.
     */
    static bool initQueues();

    /**
     * @brief Initialize semaphores.
     * 
     */
    static void initSemaphore();
};

} // namespace tinyalg::waveu
