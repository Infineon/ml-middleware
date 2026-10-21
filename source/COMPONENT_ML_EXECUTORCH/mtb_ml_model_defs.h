/******************************************************************************
* File Name: mtb_ml_model_defs.h
*
* Description: This file contains macro defines for ExecutorTorch inference engine
*
*
*******************************************************************************
* (c) 2026, Infineon Technologies AG, or
* an affiliate of Infineon Technologies AG. All rights reserved.
*******************************************************************************
* This software, associated documentation and materials ("Software") is owned
* by Infineon Technologies AG or one of its affiliates ("Infineon") and is
* protected by and subject to worldwide patent protection, worldwide copyright
* laws, and international treaty provisions. Therefore, you may use this
* Software only as provided in the license agreement accompanying the software
* package from which you obtained this Software. If no license agreement
* applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE IS
* PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING,
* BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF THIRD-PARTY RIGHTS
* AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A SPECIFIC
* USE/PURPOSE OR MERCHANTABILITY. Infineon reserves the right to make changes
* to the Software without notice. You are responsible for properly designing,
* programming, and testing the functionality and safety of your intended
* application of the Software, as well as complying with any legal requirements
* related to its use. Infineon does not guarantee that the Software will be
* free from intrusion, data theft or loss, or other breaches ("Security
* Breaches"), and Infineon shall have no liability arising out of any Security
* Breaches. Unless otherwise explicitly approved by Infineon, the Software may
* not be used in any application where a failure of the Product or any
* consequences of the use thereof can reasonably be expected to result in
* personal injury.
*******************************************************************************/

#ifndef __MTB_ML_MODEL_DEFS_H__
#define __MTB_ML_MODEL_DEFS_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "cy_result.h"

/******************************************************************************
 * Typedefs
 *****************************************************************************/
/**
 * A type definition for MTB ML Model's input/output data
 */
#if defined(COMPONENT_ML_INT16x8)
typedef int16_t MTB_ML_DATA_T;
#elif defined(COMPONENT_ML_INT8x8)
typedef int8_t MTB_ML_DATA_T;
#elif defined(COMPONENT_ML_FLOAT32)
typedef float MTB_ML_DATA_T;
#else
/* Run-time type detect */
typedef void MTB_ML_DATA_T;
#endif

/******************************************************************************
 * Macros
 *****************************************************************************/
#define EXPANDSTR(x) x
#define STRINGIFY(x) #x
#define EXPAND_AND_STRINGIFY(x) STRINGIFY(x)
#define CONCAT(X,Y) X##Y

#define INCLUDE_FILE(x,y)                    EXPAND_AND_STRINGIFY(CONCAT(x,y))
#define MODEL_DATA_BIN(x,y)                  EXPANDSTR(CONCAT(x,y))
#define MODEL_DATA_LEN(x,y)                  EXPANDSTR(CONCAT(x,y))

#if defined(COMPONENT_ML_FLOAT32)
#define ML_INCLUDE_MODEL_FILE_IMPL(m)          INCLUDE_FILE(m,_executorch_model_float.h)
#define ML_INCLUDE_MODEL_X_DATA_FILE_IMPL(m)   INCLUDE_FILE(m,_executorch_x_data_float.h)
#define ML_INCLUDE_MODEL_Y_DATA_FILE_IMPL(m)   INCLUDE_FILE(m,_executorch_y_data_float.h)
#elif defined(COMPONENT_ML_INT8x8)
#define ML_INCLUDE_MODEL_FILE_IMPL(m)          INCLUDE_FILE(m,_executorch_model_int8x8.h)
#define ML_INCLUDE_MODEL_X_DATA_FILE_IMPL(m)   INCLUDE_FILE(m,_executorch_x_data_int8x8.h)
#define ML_INCLUDE_MODEL_Y_DATA_FILE_IMPL(m)   INCLUDE_FILE(m,_executorch_y_data_int8x8.h)
#elif defined(COMPONENT_ML_INT16x8)
#define ML_INCLUDE_MODEL_FILE_IMPL(m)          INCLUDE_FILE(m,_executorch_model_int16x8.h)
#define ML_INCLUDE_MODEL_X_DATA_FILE_IMPL(m)   INCLUDE_FILE(m,_executorch_x_data_int16x8.h)
#define ML_INCLUDE_MODEL_Y_DATA_FILE_IMPL(m)   INCLUDE_FILE(m,_executorch_y_data_int16x8.h)
#else
/* Need to do model and regression types inclusion manually, without macros-es.
 * See readme.md for examples. */
#endif

#define MTB_ML_MODEL_METHOD_ALLOCATOR_SIZE(m) MODEL_DATA_LEN(m,_METHOD_ALLOCATOR_SIZE)
#define MTB_ML_MODEL_TEMP_ALLOCATOR_SIZE(m)   MODEL_DATA_LEN(m,_TEMP_ALLOCATOR_SIZE)

#define ML_MODEL_NAME_STR_IMPL(m)            EXPAND_AND_STRINGIFY(m)
#define ML_MODEL_BIN_IMPL(m)                 MODEL_DATA_BIN(m,_model_bin)
#define ML_MODEL_X_DATA_BIN_IMPL(m)          MODEL_DATA_BIN(m,_x_data_bin)
#define ML_MODEL_Y_DATA_BIN_IMPL(m)          MODEL_DATA_BIN(m,_y_data_bin)
#define ML_MODEL_SIZE_IMPL(m)                MODEL_DATA_LEN(m,_MODEL_BIN_LEN)

#define ML_MODEL_BIN_DATA_IMPL(x)            MTB_ML_MODEL_NAME_STR(x), \
                                             MTB_ML_MODEL_BIN(x), \
                                             MTB_ML_MODEL_SIZE(x), \
                                             MTB_ML_MODEL_METHOD_ALLOCATOR_SIZE(x), \
                                             MTB_ML_MODEL_TEMP_ALLOCATOR_SIZE(x)

#define ML_MODEL_INFERENCE_ERROR_IMPL(err) \
    do { \
           printf("Inference error: ExecutorTorch status: %d \r\n", err); \
    } while(0)

#endif  /* __MTB_ML_MODEL_DEFS_H__ */
