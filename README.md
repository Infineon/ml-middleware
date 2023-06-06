# ModusToolbox™ Machine Learning Middleware Library

## Overview
The ModusToolbox™ ML Middleware Library is a set of helper functions to handle the Machine Learning models and capability to receive stream validation data.

## Features
* mtb_ml_utils  - functions to help handling the inputs/outputs of a ML model
* mtb_ml_model  - functions to initialize and inference a ML model
* mtb_ml_stream - functions to stream validation data

## Requirements

* [ModusToolbox™ software](https://www.cypress.com/products/modustoolbox-software-environment) v3.0 or greater
* Programming Language: C
* Associated Parts: See "Supported Kits" section below.

## Supported Device Family

* [PSoC™ 6](https://www.infineon.com/cms/en/product/microcontroller/32-bit-psoc-arm-cortex-microcontroller/psoc-6-32-bit-arm-cortex-m4-mcu/)

## Supported Toolchains (make variable 'TOOLCHAIN')

* GNU Arm® Embedded Compiler 10.3-2021.07 (`GCC_ARM`)
* Arm compiler v6.16 (`ARM`)
* IAR C/C++ compiler v9.30.1 (`IAR`)

The ModusToolbox™ ML Middleware library supports the following operating systems:
- Windows 7 and greater
- Ubuntu 18.04 and greater
- macOS Catalina and greater

## Quick Start Guide

This quick start guide assumes that the environment is configured for ModusToolbox™ applications development. If  PSoC™ 64 device is being used for development, it is provisioned as documented in [PSoC™ 64 Secure MCU Secure Boot SDK User Guide](www.cypress.com/documentation/software-and-drivers/psoc-64-secure-mcu-secure-boot-sdk-user-guide).

### Adding the library

Middleware library is provided in the form of the source code. There are two ways to add library to our project:

* add a dependency file (MTB format) into the 'deps' folder;
* use the Library Manager. It is available under Libraries Tab >  Machine Learning > ml-middleware.

### Using the library - ML inference engines

This library assumes that the selected inference engine has been set up.

The ml-tflite-micro and ml-inference libraries are available as a ModusToolbox™ asset. Use the following GitHub links: https://github.com/infineon/ml-tflite-micro, https://github.com/infineon/ml-inference. You can add a dependency file (mtb format) under the `deps` folder or use the `Library Manager` to add it to your application. It is available under Library -> Machine Learning -> ml-tflite-micro/ml-inference.

Refer to the ml-tflite-micro/ml-inference README.md files for further information on the `COMPONENTS` and `DEFINES` that must be added in order to select the preferred inference engine.

### Using CMSIS DSP

Some of the mtb ml utils functions from the ml-middleware library can use the CMSIS-DSP functions to accelerate its execution. To enable it, add `CMSIS_DSP` in your makefile COMPONENTS list and `MTB_ML_HAVING_CMSIS_DSP` must be added to the `DEFINES`.
NOTE: In the next release `MTB_ML_HAVING_CMSIS_DSP` will be deprecated and mtb ml utils functions will only check that the `CMSIS_DSP` component has been added.

### Using the library - ML stream

1. Make sure the application includes header files and selected model:
```c
#include "mtb_ml_stream.h"
#include "mtb_ml_model.h"
#include MTB_ML_INCLUDE_MODEL_FILE(MODEL_NAME)
```

2. Initialize the streaming engine:
```c
    mtb_ml_model_bin_t model_bin = {MTB_ML_MODEL_BIN_DATA(MODEL_NAME)};

    mtb_ml_stream_interface_t interface = {CY_ML_INTERFACE_UART, &cy_retarget_io_uart_obj};

    status = mtb_ml_stream_init(&interface, CY_ML_PROFILE_ENABLE_MODEL_PER_FRAME, &model_bin);
    if(status != MTB_ML_RESULT_SUCCESS)
    {
        printf("ERROR: mtb_ml_stream_init returned error, bailing out\r\n");
        return;
    }
```

Note: currently the streaming API supports only UART port and shares debug port with any debug messages printed by the application. Pass cy_retarget_io_uart_obj created by a call to cy_retarget_io_init() as an interface argument to mtb_ml_stream_init.

3. Call mtb_ml_stream_task() to start the streaming process. On the desktop side start the streaming application ml-coretools-streaming.
4. After the streaming test is done and mtb_ml_stream_task() returns, call mtb_ml_stream_deinit() to release any resources.

### More information
The following resources contain more information:
* [ModusToolbox™ Machine Learning Design Support](https://www.infineon.com/cms/en/design-support/tools/sdk/modustoolbox-software/modustoolbox-machine-learning/)
* [ModusToolbox™ Machine Learning Middleware Library RELEASE.md](./RELEASE.md)
* [MTB ML Middleware API Reference Guide](https://infineon.github.io/ml-middleware/html/index.html)
* [PSoC™ 6](https://www.infineon.com/cms/en/product/microcontroller/32-bit-psoc-arm-cortex-microcontroller/psoc-6-32-bit-arm-cortex-m4-mcu/)
* [Cypress Semiconductor, an Infineon Technologies Company](http://www.cypress.com)
* [Cypress Semiconductor GitHub](https://github.com/Infineon)
* [ModusToolbox™](https://www.cypress.com/products/modustoolbox)

---
© 2021-2022, Cypress Semiconductor Corporation (an Infineon company) or an affiliate of Cypress Semiconductor Corporation.
