# ModusToolbox™ Machine Learning Middleware Library Release Notes
The ModusToolbox™ ML Middleware Library is a set of helper functions to handle the Machine Learning models and capability to receive stream validation data.

Please refer to the [README.md](./README.md) for a complete description of the ModusToolbox™ ML Middleware Library.

## What's Included?
This release of the ModusToolbox™ ML Middleware library includes support for the following functionality:
* utils - functions to help handling the inputs/outputs of a machine learning model
* model - functions to initialize and inference a ML model
* stream - functions to stream validation data

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

The MTB-ML Middleware library supports Windows, Linux and Mac OS.

## What Changed?
#### v1.0.0
* Initial release

### Supported Software and Tools
This version of the was validated for compatibility with the following Software and Tools:

| Software and Tools                        | Version |
| :---                                      | :----:  |
| ML Inference                              | 1.2.0   |
| ML Core tools                             | 1.2.0   |
| GCC Compiler                              | 10.3.1  |
| ARM Compiler 6                            | 6.13    |
| IAR Compiler                              | 8.42.1  |
| Cmake                                     | 3.14    |
| PSoC 6 Board Support Package (BSP)        | 2.2.0   |
| PSoC 6 Peripheral Driver Library (PDL)    | 2.3.0   |
| CyBridge                                  | 3.2.0   |

Minimum required ModusToolbox™ Software Environment: v2.4

### More information
Use the following links for more information, as needed:
* [Cypress Semiconductor, an Infineon Technologies Company](http://www.cypress.com)
* [Cypress Semiconductor GitHub](https://github.com/Infineon)
* [ModusToolbox](https://www.cypress.com/products/modustoolbox)

---
© Cypress Semiconductor Corporation, 2021.
