In order to keep things standardized and organized for an ESP-IDF framework. Using Visual Studio Code, and all that jazz.

This is essential for custom folders like `drivers` and `features` that will live inside the directory. Which will be called `components`. The build system automatically looks inside a `components` folder at the root level, which makes folder management much easier. 
## The ideal project structure
```Plaintext
my_esp32_project/
├── CMakeLists.txt                 (Root CMakeLists - you already have this)
├── main/
│   ├── CMakeLists.txt             (Main component CMakeLists)
│   └── main.c                     (Your main application code)
└── components/
    ├── drivers/
    │   ├── CMakeLists.txt         (Drivers component CMakeLists)
    │   ├── include/
    │   │   └── my_driver.h        (Driver header files)
    │   └── my_driver.c            (Driver source files)
    └── features/
        ├── CMakeLists.txt         (Features component CMakeLists)
        ├── include/
        │   └── my_feature.h       (Feature header files)
        └── my_feature.c           (Feature source files)
```
## Configuring the `CMakelists.txt` files
### 1. Root `CMakelists.txt`
The boilerplates for new projects automatically create a `CMakelists.txt` in the root folder that looks like this
```CMake
cmake_minimum_required(VERSION 3.22)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(my_esp32_project)
```
### 2. Component `CMakeLists.txt` inside of `drivers/` and `features/`
Every subdirectory inside `components/` must have its own CMakeLists.txt to register its source files and header directories.
Thus, a `CMakeLists.txt` will be inside both `drivers/` and `features/`
```CMake
idf_component_register(SRCS "my_driver.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver)
```
```CMake
idf_component_register(SRCS "my_features.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver)
```
Note:
Since peripheral drivers are modularized, there are more than just GPIO, I2C, SPI, or ADC. You have to replace driver with the list of names.
e.g., `REQUIRES esp_driver_gpio esp_driver_i2c` etc.
Refer to the [Peripherals API](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/index.html)
### 3. Main `CMakeLists.txt` (Inside `main/`)
`main.c` needs to know that it depends on such components so that it can include their header files. You do this using the `REQUIRES` argument.
```CMake
idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES drivers features)
```
## Example Use for Code
Now that the build system knows where everything is, you can seamlessly include your headers in `main.c`
```C
#include <stdio.h>
#include "my_driver.h"   // Found automatically from components/drivers/include/
#include "my_feature.h"  // Found automatically from components/features/include/

void app_main(void) {
    // Call your driver and feature functions here
}
```
## References
[ESP32 DevKit V1 Explained for Beginners](https://www.youtube.com/watch?v=n19DJIPB1sE&t=209s)
[DOIT ESP32-DevKit-V1](https://docs.zephyrproject.org/latest/boards/others/doit_esp32_devkit_v1/doc/index.html)

