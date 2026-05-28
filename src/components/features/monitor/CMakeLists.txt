# CMakeLists.txt - CMake configuration for the monitor component
idf_component_register(SRCS "monitor.c"
                       INCLUDE_DIRS "include"
                       REQUIRES app_config app_state signal_math wifi_manager esp_wifi esp_netif esp_timer)
