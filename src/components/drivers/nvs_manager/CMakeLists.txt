# CMakeLists.txt for nvs_manager component
#
# nvs_manager is treated as a low-level support/driver component because it wraps
# NVS flash initialization. It exposes only nvs_manager_init() to the app.

idf_component_register(
    SRCS "nvs_manager.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES nvs_flash
)
