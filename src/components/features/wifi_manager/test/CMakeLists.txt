# CMakeLists.txt for wifi_manager tests
idf_component_register(SRC_DIRS "."
                       INCLUDE_DIRS "."
                       REQUIRES unity wifi_manager app_state)
