# CMakeLists.txt - CMake configuration for monitor component unit tests
idf_component_register(SRC_DIRS "."
                       INCLUDE_DIRS "."
                       REQUIRES unity monitor app_state signal_math wifi_manager)
