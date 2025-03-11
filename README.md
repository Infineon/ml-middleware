# ModusToolbox™ Machine Learning Middleware Library

## Overview
The ModusToolbox™ ML Middleware Library is a set of helper functions to handle the Machine Learning models and capability to receive stream validation data.

## Features
* mtb_ml        - functions for module initialization
* mtb_ml_utils  - functions to help handling the inputs/outputs of a ML model
* mtb_ml_model  - functions to initialize and inference a ML model
* mtb_ml_stream - functions to stream validation data

## Requirements

* [ModusToolbox™ software](https://www.cypress.com/products/modustoolbox-software-environment) v3.3 or greater
* Programming Language: C
* Associated Parts: See "Supported Kits" section below.

## Supported Device Family

* [PSOC™ 6](https://www.infineon.com/cms/en/product/microcontroller/32-bit-psoc-arm-cortex-microcontroller/psoc-6-32-bit-arm-cortex-m4-mcu/)

## Supported Toolchains (make variable 'TOOLCHAIN')

* GNU Arm® Embedded Compiler 11.3.1.67 (`GCC_ARM`)
* ARM Compiler v6.22 (`ARM`)

The ModusToolbox™ ML Middleware library supports the following operating systems:
- Windows 7 and greater
- Ubuntu 18.04 and greater
- macOS Catalina and greater

## Quick Start Guide

This quick start guide assumes that the environment is configured for ModusToolbox™ applications development. If  PSOC™ 64 device is being used for development, it is provisioned as documented in [PSOC™ 64 Secure MCU Secure Boot SDK User Guide](www.cypress.com/documentation/software-and-drivers/psoc-64-secure-mcu-secure-boot-sdk-user-guide).

### Adding the library

Middleware library is provided in the form of the source code. There are two ways to add library to our project:

* add a dependency file (MTB format) into the 'deps' folder;
* use the Library Manager. It is available under Libraries Tab >  Machine Learning > ml-middleware.

### Using the library - ML inference engines

This library assumes that the selected inference engine has been set up.

The ml-tflite-micro library is available as a ModusToolbox™ asset. Use the following GitHub link: [https://github.com/infineon/ml-tflite-micro](https://github.com/infineon/ml-tflite-micro). You can add a dependency file (mtb format) under the `deps` folder or use the `Library Manager` to add it to your application. It is available under Library -> Machine Learning -> ml-tflite-micro.

Refer to the ml-tflite-micro README.md files for further information on the `COMPONENTS` and `DEFINES` that must be added in order to select the preferred inference engine.

### Using the library - ML stream

1. Make sure the application includes module header file and selected model:
```c
#include "mtb_ml.h"
#include MTB_ML_INCLUDE_MODEL_FILE(MODEL_NAME)
```

2. Initialize the model:
```c
mtb_ml_model_t *model_object;
mtb_ml_model_bin_t model_bin = {MTB_ML_MODEL_BIN_DATA(MODEL_NAME)};
if (mtb_ml_model_init(&model_bin, NULL, &model_object) != MTB_ML_RESULT_SUCCESS)
{
    // Error handling
}
```

3. Initialize the streaming interface:
```c
mtb_ml_stream_interface_t iface;
mtb_data_streaming_interface_t *iface_obj = (mtb_data_streaming_interface_t *) malloc(sizeof(mtb_data_streaming_interface_t));
if(!iface_obj)
{
    // Error handling
}

iface.interface_obj = iface_obj;
```

4. Initialize the stream. On the desktop side, start the streaming application ml-coretools-middleware-streaming. Note that mtb_ml_stream_init does not allocate any memory resources, but is used to initialize communication with the desktop. Therefore, if you wish to run re-run your device application without re-programming it between runs, you must call mtb_ml_stream_init for each run.
```c
/* Initialize stream */
status = mtb_ml_stream_init(&iface, model_object);
if(status != MTB_ML_RESULT_SUCCESS)
{
    // Error handling
}
```
5. Your application then needs to receive input data, and transmit the resulting output data back to the desktop continuosly. Here is a typical use case as an example:
```c
cy_rslt_t stream_task(mtb_ml_stream_interface_t *iface, mtb_ml_model_t *model_object)
{
    cy_rslt_t result;

    /* Alloc RX buf */
    MTB_ML_DATA_T *rx_buf = (MTB_ML_DATA_T *)malloc(iface->input_size * sizeof(MTB_ML_DATA_T));
    if(!rx_buf)
    {
        // Error handling
    }

    for (int i = 0; i < iface->x_data_info.num_of_samples; i++)
    {
        // Streaming input data
        result = mtb_ml_stream_input_data(iface, rx_buf, USER_TIMEOUT_VAL);
        if(result != MTB_ML_RESULT_SUCCESS)
        {
            // Error handling
        }

        // Inferencing
        result = mtb_ml_model_run(model_object, (MTB_ML_DATA_T *) rx_buf);
        if (result != MTB_ML_RESULT_SUCCESS)
        {
            // Error handling
        }

        // Streaming output data
        result = mtb_ml_stream_output_data(iface, model_object->output, tx_timeout_ms);
        if(result != MTB_ML_RESULT_SUCCESS)
        {
            printf("ERROR: Failed to send output data to host\r\n");
            goto ret_err;
        }
    }
}
```
6. Finally, inform the host that your application is complete:
```c
status = mtb_ml_inform_host_done(iface, DEFAULT_TIMEOUT_MS);
if(status != MTB_ML_RESULT_SUCCESS)
{
    // Error handling
}
```

Note: The UART port is shared with the debug port for any messages that are printed by the application.


### More information
The following resources contain more information:
* [ModusToolbox™ Machine Learning Design Support](https://www.infineon.com/cms/en/design-support/tools/sdk/modustoolbox-software/modustoolbox-machine-learning/)
* [ModusToolbox™ Machine Learning Middleware Library RELEASE.md](./RELEASE.md)
* [MTB ML Middleware API Reference Guide](https://infineon.github.io/ml-middleware/html/index.html)
* [PSOC™ 6](https://www.infineon.com/cms/en/product/microcontroller/32-bit-psoc-arm-cortex-microcontroller/psoc-6-32-bit-arm-cortex-m4-mcu/)
* [Cypress Semiconductor, an Infineon Technologies Company](http://www.cypress.com)
* [Cypress Semiconductor GitHub](https://github.com/Infineon)
* [ModusToolbox™](https://www.cypress.com/products/modustoolbox)

---
© 2021-2025, Cypress Semiconductor Corporation (an Infineon company) or an affiliate of Cypress Semiconductor Corporation.
