# CMakeLists.txt for nvs_manager unit tests

idf_component_register(
    SRCS "test_nvs_manager.c"
    INCLUDE_DIRS "."
    REQUIRES unity nvs_manager
)
