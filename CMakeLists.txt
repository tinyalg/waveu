idf_component_register(
        SRCS   
                "WaveuHelper.cpp"
                "LUTHelper.cpp"
                "ESP32Config.cpp" 
                "Queues.cpp"
                "PhaseGenerator.cpp"
                "Semaphores.cpp"
                "TaskDelete.cpp"

        INCLUDE_DIRS "include"

        REQUIRES esp_driver_gpio esp_timer esp_driver_dac)
