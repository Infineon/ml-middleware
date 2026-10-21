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

### v3.5.0

* Updated compatibility with the current ModusToolbox ML asset line: `ml-tflite-micro` v3.5.0 and `ml-executorch` v1.2.0
* Added experimental support of ExecuTorch inference backend (`COMPONENT_ML_EXECUTORCH`)
    * inference structures and primitives for Executorch models run-time
    * CPU-only and U55 inference support
    * Pre-computed memory-pool sizes support during conversion in FW for .pte models
* Migrated from LLVM 19 to LLVM 22 and validated
* Fixed U55 cache maintenance for MIMO models by cleaning every input tensor and invalidating every output tensor

### v3.4.0

* Added macOS arm64 native support for the middleware streaming application (no Rosetta required)

### v3.3.0

* Fixed FullyConnected (FC) per-channel quantization in NNLite integration
* Fixed dequantization pointer arithmetic in `mtb_ml_utils_model_dequantize` for correct handling of varying output data type sizes

### v3.2.0

* Implemented Multi-Input/Multi-Output (MIMO) model support. It is available in both TFLM (with interpreter) and TFLM_LESS (pre-interpreted) configurations
* Runtime-quantization params usage added - quantization can be defined both design-time and run-time
* Added RELU_0TO1 and RELU_N1TO1 activation function support
* Functional fixes for U55 cycle count and cache management

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
This version was validated for compatibility with the following Software and Tools:

| Software and Tools                        | Version       |
| :---                                      | :----:        |
| ML TFLite Micro                           | 3.5.0         |
| ML Executorch                             | 1.2.0         |
| ML Coretools                              | 3.4.0         |
| GCC Compiler                              | 14.2          |
| ARM Compiler 6                            | 6.22          |
| LLVM Toolchain                            | 22.1.0        |

Minimum required ModusToolbox™ Software Environment: v3.8

## More information
Use the following links for more information, as needed:
* [ModusToolbox™ Machine Learning Design Support](https://www.infineon.com/cms/en/design-support/tools/sdk/modustoolbox-software/modustoolbox-machine-learning/)
* [Infineon Technologies AG](https://www.infineon.com)
* [Cypress Semiconductor GitHub](https://github.com/Infineon)
* [ModusToolbox](https://www.infineon.com/design-resources/development-tools/sdk/modustoolbox-software)

---
© 2021-2026, Infineon Technologies AG or an affiliate of Infineon Technologies AG.
