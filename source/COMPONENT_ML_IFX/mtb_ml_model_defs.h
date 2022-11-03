/***************************************************************************//**
* \file mtb_ml_model_defs.h
*
* \brief
* This file contains macro defines for IFX inference engine.
*
*******************************************************************************
* (c) 2019-2022, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnity Cypress against all liability.
*******************************************************************************/
#ifndef __MTB_ML_MODEL_DEFS_H__
#define __MTB_ML_MODEL_DEFS_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "cy_result.h"
#include "cy_ml_inference.h"

/******************************************************************************
 * Typedefs
 *****************************************************************************/
/**
 * A type definition for MTB ML Model's input/output data
 */
typedef CY_ML_DATA_TYPE_T MTB_ML_DATA_T;

/******************************************************************************
 * Macros
 *****************************************************************************/
#define EXPANDSTR(x) x
#define STRINGIFY(x) #x
#define EXPAND_AND_STRINGIFY(x) STRINGIFY(x)
#define CONCAT(X,Y) X##Y

#define INCLUDE_FILE(x,y)       EXPAND_AND_STRINGIFY(CONCAT(x,y))
#define MODEL_DATA_BIN(x,y)     EXPANDSTR(CONCAT(x,y))
#define MODEL_DATA_LEN(x,y)     EXPANDSTR(CONCAT(x,y))
#define MODEL_CONSTANT(x,y)     EXPANDSTR(CONCAT(x,y))

#define MODLE_STATE_OUTS(x)     MODEL_CONSTANT(x,_MODEL_SIZE_OF_STATE_OUT)
#define MODEL_LAYERS(x)         MODEL_CONSTANT(x,_MODEL_NUM_OF_LAYERS)
#define MODEL_RESIDUAL_CONNS(x) MODEL_CONSTANT(x,_MODEL_NUM_OF_RESIDUAL_CONN)

#define MTB_ML_MODEL_PRMS_BIN(x)               MODEL_DATA_BIN(x,_model_prms_bin)
#define MTB_ML_MODEL_PRMS_LEN(x)               MODEL_DATA_LEN(x,_MODEL_PRMS_BIN_LEN)

#define ML_MODEL_NAME_STR_IMPL(x)              EXPAND_AND_STRINGIFY(x)
#define ML_MODEL_BIN_IMPL(x)                   MODEL_DATA_BIN(x,_model_bin)
#define ML_MODEL_SIZE_IMPL(x)                  MODEL_DATA_LEN(x,_MODEL_BIN_LEN)
#define ML_MODEL_X_DATA_BIN_IMPL(x)            MODEL_DATA_BIN(x,_x_data_bin)
#define ML_MODEL_Y_DATA_BIN_IMPL(x)            MODEL_DATA_BIN(x,_y_data_bin)

#if defined(COMPONENT_ML_FLOAT32)
  #define ML_INCLUDE_MODEL_FILE_IMPL(x)        INCLUDE_FILE(x,_ifx_model_float.h)
  #define ML_INCLUDE_MODEL_X_DATA_FILE_IMPL(x) INCLUDE_FILE(x,_ifx_x_data_float.h)
  #define ML_INCLUDE_MODEL_Y_DATA_FILE_IMPL(x) INCLUDE_FILE(x,_ifx_y_data_float.h)
#elif defined(COMPONENT_ML_INT16x16)
  #define ML_INCLUDE_MODEL_FILE_IMPL(x)        INCLUDE_FILE(x,_ifx_model_int16x16.h)
  #define ML_INCLUDE_MODEL_X_DATA_FILE_IMPL(x) INCLUDE_FILE(x,_ifx_x_data_int16x16.h)
  #define ML_INCLUDE_MODEL_Y_DATA_FILE_IMPL(x) INCLUDE_FILE(x,_ifx_y_data_int16x16.h)
#elif defined(COMPONENT_ML_INT16x8)
  #define ML_INCLUDE_MODEL_FILE_IMPL(x)        INCLUDE_FILE(x,_ifx_model_int16x8.h)
  #define ML_INCLUDE_MODEL_X_DATA_FILE_IMPL(x) INCLUDE_FILE(x,_ifx_x_data_int16x8.h)
  #define ML_INCLUDE_MODEL_Y_DATA_FILE_IMPL(x) INCLUDE_FILE(x,_ifx_y_data_int16x8.h)
#elif defined(COMPONENT_ML_INT8x8)
  #define ML_INCLUDE_MODEL_FILE_IMPL(x)        INCLUDE_FILE(x,_ifx_model_int8x8.h)
  #define ML_INCLUDE_MODEL_X_DATA_FILE_IMPL(x) INCLUDE_FILE(x,_ifx_x_data_int8x8.h)
  #define ML_INCLUDE_MODEL_Y_DATA_FILE_IMPL(x) INCLUDE_FILE(x,_ifx_y_data_int8x8.h)
#else
  $(error Unsupported data type)
#endif

#define ML_MODEL_BIN_DATA_IMPL(x)              ML_MODEL_NAME_STR_IMPL(x), \
                                               MTB_ML_MODEL_PRMS_BIN(x), \
                                               MTB_ML_MODEL_PRMS_LEN(x), \
                                               ML_MODEL_BIN_IMPL(x),  \
                                               ML_MODEL_SIZE_IMPL(x)

/**
 * \addtogroup Common_Macro
 * @{
 */
/* Macors for static memory allocation */
/*!
 * \def MTB_ML_MODEL_INPUT_DATA_SIZE(x)
 *  A helper macro that returns the array size of a MTB ML Model's input data.
 *
 * \def MTB_ML_MODEL_OUTPUT_DATA_SIZE(x)
 *  A helper macro that returns the array size of a MTB ML Model's output data.
 *
 * \def MTB_ML_MODEL_INPUT_BUFFER_SIZE(x)
 * A helper macro that returns the buffer size of a MTB ML Model's input data.
 *
 * \def MTB_ML_MODEL_OUTPUT_BUFFER_SIZE(x)
 * A helper macro that returns the buffer size of a MTB ML Model's output data.
 *
 * \def MTB_ML_MODEL_SCRATCH_MEM_SIZE(x)
 * A helper macro that returns the size of a MTB ML Model's scratch memory.
 *
 * \def MTB_ML_MODEL_PERSISTENT_MEM_SIZE(x)
 * A helper macro that returns the size of a MTB ML Model's persistent memory.
 */
#define MTB_ML_MODEL_INPUT_DATA_SIZE(x)       MODEL_CONSTANT(x,_MODEL_INPUT_DATA_SIZE)
#define MTB_ML_MODEL_OUTPUT_DATA_SIZE(x)      MODEL_CONSTANT(x,_MODEL_OUTPUT_DATA_SIZE)
#define MTB_ML_MODEL_INPUT_BUFFER_SIZE(x)     ((MTB_ML_MODEL_INPUT_DATA_SIZE(x) + 2) * sizeof(CY_ML_DATA_TYPE_T))
#define MTB_ML_MODEL_OUTPUT_BUFFER_SIZE(x)    ((MTB_ML_MODEL_OUTPUT_DATA_SIZE(x)) * sizeof(CY_ML_DATA_TYPE_T))
#define MTB_ML_MODEL_SCRATCH_MEM_SIZE(x)      MODEL_CONSTANT(x,_MODEL_SCRATCH_MEM_SIZE)
#define MTB_ML_MODEL_PERSISTENT_MEM_SIZE(x)   ((CY_ML_MODEL_OBJECT_SIZE) + (MODLE_STATE_OUTS(x)) + \
                                              (CY_ML_LAYER_OBJECT_SIZE * MODEL_LAYERS(x)) + \
                                              (CY_ML_RES_CONN_OBJECT_SIZE * MODEL_RESIDUAL_CONNS(x)))

/**
 * @} end of Common_Macro group
 */
#define ML_MODEL_INFERENCE_ERROR_IMPL(err) \
    do { \
           printf("Inference error: Error code=%x, Layer index=%d, Line number=%d, aborting... \r\n", \
           CY_ML_ERR_CODE(err), CY_ML_ERR_LAYER_INDEX(err), CY_ML_ERR_LINE_NUMBER(err)); \
    } while(0)

#if defined(__cplusplus)
}
#endif

#endif /* __MTB_ML_MODEL_DEFS_H__ */
