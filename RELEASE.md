# ModusToolbox™ Machine Learning Middleware Library Release Notes
The ModusToolbox™ ML Middleware Library is a set of helper functions to handle the Machine Learning models and capability to receive stream validation data.

Please refer to the [README.md](./README.md) for a complete description of the ModusToolbox™ ML Middleware Library.

## What's Included?
This release of the ModusToolbox™ ML Middleware library includes support for the following functionality:
* utils - functions to help handling the inputs/outputs of a machine learning model
* model - functions to initialize and inference a ML model
* stream - functions to stream validation data

Support floating-point and fixed-point variants:
* 32-bit floating-point
* 16-bit fixed-point input
* 8-bit fixed-point input
* 8-bit fixed-point weight

The MTB-ML Middleware library supports Windows, Linux and Mac OS.

## What Changed?

### v3.1.0

* Implemented support for PSOC Edge™ capabilities:
    * Cortex-M33 (CPU-based inference)
    * NNLite™ NPU integration (Cortex-M33 core only)
    * EthosU U55 NPU integration (Cortex-M55 core only)
* Added LSTM models support
* Implemented NNLite™ NPU RToS integration
* Implemented EthosU U55 NPU RToS integration
* Implemented EthosU U55 NPU cycle count profiling
* MTB-HAL Support added
* Support of ml-inference obsoleted

### v2.0.1

* Added `mtb_ml_utils_model_quantize` & `mtb_ml_utils_convert_tflm_flt_to_int8` functions

### v2.0.0

* Support for TensorFlow Lite Micro
* PSOC6™ as a target platform (Cortex-M4)

### v1.0.0

* Initial release

## Supported Software and Tools
This version of the was validated for compatibility with the following Software and Tools:

| Software and Tools                        | Version       |
| :---                                      | :----:        |
| ML TFLiteU                                | 3.1.0         |
| ML Coretools                              | 3.0.0         |
| GCC Compiler                              | 11.3          |
| ARM Compiler 6                            | 6.22          |
| LLVM Toolchain                            | 19.1.1        |

Minimum required ModusToolbox™ Software Environment: v3.3

## More information
Use the following links for more information, as needed:
* [ModusToolbox™ Machine Learning Design Support](https://www.infineon.com/cms/en/design-support/tools/sdk/modustoolbox-software/modustoolbox-machine-learning/)
* [Cypress Semiconductor, an Infineon Technologies Company](http://www.cypress.com)
* [Cypress Semiconductor GitHub](https://github.com/Infineon)
* [ModusToolbox](https://www.cypress.com/products/modustoolbox)

---
© 2021-2025, Cypress Semiconductor Corporation (an Infineon company) or an affiliate of Cypress Semiconductor Corporation.
