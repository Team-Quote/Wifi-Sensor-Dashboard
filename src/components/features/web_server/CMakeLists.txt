# CMakeLists.txt for web_server component
#
# web_server reads app_state and exposes it through ESP-IDF's lightweight
# HTTP server. It does not own Wi-Fi or telemetry sampling.

idf_component_register(
    SRCS "web_server.c"
    INCLUDE_DIRS "include"
    REQUIRES app_state
    PRIV_REQUIRES esp_http_server log
)