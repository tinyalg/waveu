set(SRC_FILES           "WaveuHelper.cpp"
                        "LUTHelper.cpp"
                        "ESP32Config.cpp" 
                        "Queues.cpp"
                        "PhaseGenerator.cpp"
                        "Semaphores.cpp"
                        "TaskDelete.cpp"
                        "DataOutputTask.cpp"
)  # Common source file

# Determine which source files to include
if (CONFIG_WAVEU_DATA_SINK_DAC)
    list(APPEND SRC_FILES  "DacOutput.cpp")
    message(STATUS "Including data_sink_dac.cpp")
endif()

if (CONFIG_WAVEU_DATA_SINK_UDP)
    list(APPEND SRC_FILES "UdpOutput.cpp")
    message(STATUS "Including data_sink_udp.cpp")
endif()

idf_component_register(SRCS ${SRC_FILES}
                      INCLUDE_DIRS "include"
                      REQUIRES esp_driver_gpio esp_timer esp_driver_dac)
