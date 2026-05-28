# CMakeLists.txt for wifi_manager component
idf_component_register(SRCS "wifi_manager.c"
                       INCLUDE_DIRS "include"
                       REQUIRES app_config app_state esp_wifi esp_event esp_netif)
