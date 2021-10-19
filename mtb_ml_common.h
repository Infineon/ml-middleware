/***************************************************************************//**
* \file mtb_ml_common.h
*
* \brief
* This is the common header file of ModusToolbox ML middleware library.
*
*******************************************************************************
* \copyright
* Copyright 2021, Cypress Semiconductor Corporation (an Infineon company).
* All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
******************************************************************************/


#if !defined(__MTB_ML_COMMON_H__)
#define __MTB_ML_COMMON_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "cy_result.h"
#include "cy_ml_inference.h"

/******************************************************************************
 * Version
 *****************************************************************************/
#define MTB_ML_MIDDLEWARE_VERSION_MAJOR     1
#define MTB_ML_MIDDLEWARE_VERSION_MINOR     0
#define MTB_ML_MIDDLEWARE_VERSION_PATCH     0

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

/**
 * \addtogroup Common_Macro
 * @{
 */
/*!
 * \def MTB_ML_MODEL_NAME_STR(x)
 * A helper macro that returns the string constant of a MTB ML Model's name.
 *
 * \def MTB_ML_INCLUDE_MODEL_FILE(x)
 * A helper macro that returns the string constant of a MTB ML Model's header file.
 *
 * \def MTB_ML_MODEL_PRMS_BIN(x)
 * A helper macro that returns the address of a MTB ML Model's parameter.
 *
 * \def MTB_ML_MODEL_PRMS_LEN(x)
 * A helper macro that returns the size of a MTB Model's parameter.
 */
#define MTB_ML_MODEL_NAME_STR(x)             EXPAND_AND_STRINGIFY(x)
#define MTB_ML_INCLUDE_MODEL_FILE(x)         INCLUDE_FILE(x,_model_all.h)
#define MTB_ML_MODEL_PRMS_BIN(x)             MODEL_DATA_BIN(x,_model_prms_bin)
#define MTB_ML_MODEL_PRMS_LEN(x)             MODEL_DATA_LEN(x,_model_prms_bin_len)

/*!
 * \def MTB_ML_INCLUDE_MODEL_X_DATA_FILE(x)
 * A helper macro that returns the string constant of a MTB ML Model's sample data.
 *
 * \def MTB_ML_INCLUDE_MODEL_Y_DATA_FILE(x)
 * A helper macro that returns the string constant of a MTB ML Model's reference result.
 *
 * \def MTB_ML_MODEL_WTS_BIN(x)
 * A helper macro that returns the address of a MTB ML Model's weights and bias data.
 *
 * \def MTB_ML_MODEL_WTS_LEN(x)
 * A helper macro that returns the size of a MTB ML Model's weights and bias data.
 *
 * \def MTB_ML_MODEL_X_DATA_BIN(x)
 * A helper macro that returns the address of a MTB ML Model's sample data.
 *
 * \def MTB_ML_MODEL_X_DATA_LEN(x)
 * A helper macro that returns the size of a MTB ML Model's sample data.
 *
 * \def MTB_ML_MODEL_Y_DATA_BIN(x)
 * A helper macro that returns the address of a MTB ML Model's reference result.
 *
 * \def MTB_ML_MODEL_Y_DATA_LEN(x)
 *  A helper macro that returns the size of a MTB ML Model's reference result.
 */
#if defined(COMPONENT_ML_FLOAT32)
  #define MTB_ML_INCLUDE_MODEL_X_DATA_FILE(x)  INCLUDE_FILE(x,_x_data_flt.h)
  #define MTB_ML_INCLUDE_MODEL_Y_DATA_FILE(x)  INCLUDE_FILE(x,_y_data_flt.h)
  #define MTB_ML_MODEL_WTS_BIN(x)              MODEL_DATA_BIN(x,_model_flt_bin)
  #define MTB_ML_MODEL_WTS_LEN(x)              MODEL_DATA_LEN(x,_model_flt_bin_len)
  #define MTB_ML_MODEL_X_DATA_BIN(x)           MODEL_DATA_BIN(x,_x_data_flt_bin)
  #define MTB_ML_MODEL_X_DATA_LEN(x)           MODEL_DATA_LEN(x,_x_data_flt_bin_len)
  #define MTB_ML_MODEL_Y_DATA_BIN(x)           MODEL_DATA_BIN(x,_y_data_flt_bin)
  #define MTB_ML_MODEL_Y_DATA_LEN(x)           MODEL_DATA_LEN(x,_y_data_flt_bin_len)
#elif defined(COMPONENT_ML_INT16x16)
  #define MTB_ML_INCLUDE_MODEL_X_DATA_FILE(x)  INCLUDE_FILE(x,_x_data_fixed.h)
  #define MTB_ML_INCLUDE_MODEL_Y_DATA_FILE(x)  INCLUDE_FILE(x,_y_data_fixed.h)
  #define MTB_ML_MODEL_WTS_BIN(x)              MODEL_DATA_BIN(x,_model_fixed16_bin)
  #define MTB_ML_MODEL_WTS_LEN(x)              MODEL_DATA_LEN(x,_model_fixed16_bin_len)
  #define MTB_ML_MODEL_X_DATA_BIN(x)           MODEL_DATA_BIN(x,_x_data_fixed16_bin)
  #define MTB_ML_MODEL_X_DATA_LEN(x)           MODEL_DATA_LEN(x,_x_data_fixed16_bin_len)
  #define MTB_ML_MODEL_Y_DATA_BIN(x)           MODEL_DATA_BIN(x,_y_data_fixed_bin)
  #define MTB_ML_MODEL_Y_DATA_LEN(x)           MODEL_DATA_LEN(x,_y_data_fixed_bin_len)
#elif defined(COMPONENT_ML_INT16x8)
  #define MTB_ML_INCLUDE_MODEL_X_DATA_FILE(x)  INCLUDE_FILE(x,_x_data_fixed.h)
  #define MTB_ML_INCLUDE_MODEL_Y_DATA_FILE(x)  INCLUDE_FILE(x,_y_data_fixed.h)
  #define MTB_ML_MODEL_WTS_BIN(x)              MODEL_DATA_BIN(x,_model_fixed8_bin)
  #define MTB_ML_MODEL_WTS_LEN(x)              MODEL_DATA_LEN(x,_model_fixed8_bin_len)
  #define MTB_ML_MODEL_X_DATA_BIN(x)           MODEL_DATA_BIN(x,_x_data_fixed16_bin)
  #define MTB_ML_MODEL_X_DATA_LEN(x)           MODEL_DATA_LEN(x,_x_data_fixed16_bin_len)
  #define MTB_ML_MODEL_Y_DATA_BIN(x)           MODEL_DATA_BIN(x,_y_data_fixed_bin)
  #define MTB_ML_MODEL_Y_DATA_LEN(x)           MODEL_DATA_LEN(x,_y_data_fixed_bin_len)
#elif defined(COMPONENT_ML_INT8x8)
  #define MTB_ML_INCLUDE_MODEL_X_DATA_FILE(x)  INCLUDE_FILE(x,_x_data_fixed.h)
  #define MTB_ML_INCLUDE_MODEL_Y_DATA_FILE(x)  INCLUDE_FILE(x,_y_data_fixed.h)
  #define MTB_ML_MODEL_WTS_BIN(x)              MODEL_DATA_BIN(x,_model_fixed8_bin)
  #define MTB_ML_MODEL_WTS_LEN(x)              MODEL_DATA_LEN(x,_model_fixed8_bin_len)
  #define MTB_ML_MODEL_X_DATA_BIN(x)           MODEL_DATA_BIN(x,_x_data_fixed8_bin)
  #define MTB_ML_MODEL_X_DATA_LEN(x)           MODEL_DATA_LEN(x,_x_data_fixed8_bin_len)
  #define MTB_ML_MODEL_Y_DATA_BIN(x)           MODEL_DATA_BIN(x,_y_data_fixed_bin)
  #define MTB_ML_MODEL_Y_DATA_LEN(x)           MODEL_DATA_LEN(x,_y_data_fixed_bin_len)
#else
  $(error Unsupported data type)
#endif

/*!
 * \def MTB_ML_MODEL_BIN_DATA(x)
 * A helper macro that populates mtb_ml_model_bin_t structure with MTB ML Model's data.
 */
#define MTB_ML_MODEL_BIN_DATA(x)               MTB_ML_MODEL_NAME_STR(x), \
                                               MTB_ML_MODEL_PRMS_BIN(x), \
                                               MTB_ML_MODEL_PRMS_LEN(x), \
                                               MTB_ML_MODEL_WTS_BIN(x),  \
                                               MTB_ML_MODEL_WTS_LEN(x)

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

/******************************************************************************
 * Typedefs
 *****************************************************************************/
/**
 * A type definition for MTB ML Model's input/output data
 */
typedef CY_ML_DATA_TYPE_T MTB_ML_DATA_T;

/**
 * A enum type for MTB ML profiling configuration
 */
typedef cy_en_ml_profile_config_t mtb_ml_profile_config_t;

/**
 * @} end of Common_Macro group
 */

/******************************************************************************
* Public definitions
******************************************************************************/

#define MTB_ML_RESULT_SUCCESS            CY_RSLT_SUCCESS
#define MTB_ML_RESULT_BAD_ARG            CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_ML, 1)
#define MTB_ML_RESULT_ALLOC_ERR          CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_ML, 2)
#define MTB_ML_RESULT_BAD_MODEL          CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_ML, 3)
#define MTB_ML_RESULT_MISMATCH_DATA_TYPE CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_ML, 4)
#define MTB_ML_RESULT_INPUT_ERROR        CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_ML, 5)
#define MTB_ML_RESULT_INFERENCE_ERROR    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_ML, 6)
#define MTB_ML_RESULT_COMM_ERROR         CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_ML, 7)
#define MTB_ML_RESULT_TIMEOUT            CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_ML, 8)

/******************************************************************************
* Structures
******************************************************************************/



#if defined(__cplusplus)
}
#endif

#endif /* __MTB_ML_COMMON_H__ */

