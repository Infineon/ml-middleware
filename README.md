# ModusToolbox™ Machine Learning Middleware Library

## Overview
The ModusToolbox™ ML Middleware Library is a set of helper functions to handle the Machine Learning models and capability to receive stream validation data.

## Features
* mtb_ml_utils  - functions to help handling the inputs/outputs of a ML model
* mtb_ml_model  - functions to initialize and inference a ML model
* mtb_ml_stream - functions to stream validation data

## Requirements

* [ModusToolbox™ software](https://www.cypress.com/products/modustoolbox-software-environment) v2.4 or greater
* Board Support Package (BSP) minimum required version: 2.2.0
* Programming Language: C
* Associated Parts: See "Supported Kits" section below.

## Supported Kits (make variable 'TARGET')

* [PSoC® 64 Standard Secure - AWS Wi-Fi BT Pioneer Kit (CY8CKIT-064S0S2-4343W)](https://www.cypress.com/documentation/development-kitsboards/psoc-64-standard-secure-aws-wi-fi-bt-pioneer-kit-cy8ckit)
* [PSoC® 62S2 Wi-Fi BT Pioneer KIT (CY8CKIT-062S2-43012)](https://www.cypress.com/documentation/development-kitsboards/psoc-62s2-wi-fi-bt-pioneer-kit-cy8ckit-062s2-43012)

## Supported Toolchains (make variable 'TOOLCHAIN')

* GNU Arm® Embedded Compiler v10.3.1 (`GCC_ARM`)
* Arm compiler v6.13 (`ARM`)
* IAR C/C++ compiler v8.42.1 (`IAR`)

Support following typical ML model:
* MLP - Dense feed-forward network
* CONV1D
* CONV2D
* RNNs - GRU

Support floating-point and fixed-point variants:
* 32-bit floating-point
* 16-bit fixed-point input
* 8-bit fixed-point input
* 16-bit fixed-point weight
* 8-bit fixed-point weight

The ModusToolbox™ ML Middleware library supports Windows, Linux and Mac OS.

## Quick Start Guide

This quick start guide assumes that the environment is configured for ModusToolbox™ applications development. If PSoC64 device is being used for development, it is provisioned as documented in [PSoC 64 Secure MCU Secure Boot SDK User Guide](www.cypress.com/documentation/software-and-drivers/psoc-64-secure-mcu-secure-boot-sdk-user-guide).
### Adding the library

Middleware library is provided in the form of the source code. There are two ways to add library to our project:

* add a dependency file (MTB format) into the 'deps' folder;
* use the Library Manager. It is available under Libraries Tab >  Machine Learning > ml-middleware.

### Using the library - ML model
#### Step 1: Specify the quantization type in ModusToolbox™ makefile

In the COMPONENTS parameter, add one of the following:
* ML_FLOAT32: use 32-bit floating-point for the weights and input data
* ML_INT16x16: use 16-bit fixed-point for the weights and input data
* ML_INT16x8: use 16-bit fixed-point for the input data and 8-bit for the weights
* ML_INT8x8: use 8-bit fixed-point for the weights and input data

  e.g.: COMPONENTS+=ML_INT16x16

#### Step 2: Include MTB NN Model and Create Runtime Object

Ues the helper macros to include the MTB NN model header file and setup MTB NN model data

/* Include MTB NN model header file */
#include MTB_ML_INCLUDE_MODEL_FILE(MODEL_NAME)

/* Setup MTB NN model data */
mtb_ml_model_bin_t model_bin = {MTB_ML_MODEL_BIN_DATA(MODEL_NAME)};

/* Create MTB NN model runtime object */
 cy_rslt_t result = mtb_ml_model_init(&model_bin, NULL, &model_object);

#### Step 3: Optional: Setup Profile Configuration

mtb_ml_model_profile_config(model_object, config);

Where, config can be one of followings:
    CY_ML_PROFILE_DISABLE
    CY_ML_PROFILE_ENABLE_MODEL
    CY_ML_PROFILE_ENABLE_LAYER
    CY_ML_PROFILE_ENABLE_MODEL_PER_FRAME
    CY_ML_PROFILE_ENABLE_LAYER_PER_FRAME
    CY_ML_LOG_ENABLE_MODEL_LOG

#### Step 4: Optional: Set Q-factor if input data is in Q-format

Required for all quantization types, except the ML_FLOAT32
mtb_ml_model_set_input_q_fraction_bits(model_object, frame_q);

#### Step 5: Run Inference

With provided data in "input" buffer, run inference and generate result in "output" buffer.
mtb_ml_model_run(model_object, input, output);

#### Step 6: Optional: Generate Profile Log

Generate profile log on console, based on profile configuration
mtb_ml_model_profile_log(model_object);

#### Step 6: Free All Resources and Delete Runtime Object
mtb_ml_model_deinit(model_object);

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

Note: currently the streaming API supports only UART port (SPI/I2C, USB - TBD) and shares debug port with any debug messages printed by the application. Pass cy_retarget_io_uart_obj created by a call to cy_retarget_io_init() as an interface argument to mtb_ml_stream_init.

3. Call mtb_ml_stream_task() to start the streaming process. On the desktop side start the streaming application ml-coretools-streaming.
4. After the streaming test is done and mtb_ml_stream_task() returns, call mtb_ml_stream_deinit() to release any resources.

### More information
The following resources contain more information:
* [ModusToolbox™ Machine Learning Middleware Library RELEASE.md](./RELEASE.md)
* [MTB ML Middleware API Reference Guide](https://infineon.github.io/ml-middleware/html/index.html)
* [PSoC© 64 Microcontrollers](https://www.cypress.com/products/psoc-64-microcontrollers-arm-cortex-m4m0)
* [Cypress Semiconductor, an Infineon Technologies Company](http://www.cypress.com)
* [Cypress Semiconductor GitHub](https://github.com/Infineon)
* [ModusToolbox™](https://www.cypress.com/products/modustoolbox)

---
© Cypress Semiconductor Corporation, 2021.
