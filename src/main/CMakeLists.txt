# CMakeLists.txt - CMake configuration for main application
idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES nvs_manager app_state wifi_manager monitor web_server)
