/***************************************************************************//**
* \file mtb_ml_common.h
*
* \brief
* This is the common header file of ModusToolbox ML middleware library.
*
*******************************************************************************
* (c) 2019-2026, Infineon Technologies AG, or
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
#if !defined(__MTB_ML_COMMON_H__)
#define __MTB_ML_COMMON_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "cy_result.h"

#if defined(__cplusplus)
extern "C" {
#endif

/******************************************************************************
 * Version
 *****************************************************************************/
#define MTB_ML_MIDDLEWARE_VERSION_MAJOR     3
#define MTB_ML_MIDDLEWARE_VERSION_MINOR     1
#define MTB_ML_MIDDLEWARE_VERSION_PATCH     0

/******************************************************************************
 * Macros
 *****************************************************************************/

/**
 * \addtogroup Common_Macro
 * @{
 */
/*!
 * \def MTB_ML_MODEL_NAME_STR(m)
 * A helper macro that returns the string constant of a MTB ML Model's name.
 *
 * \def MTB_ML_INCLUDE_MODEL_FILE(m)
 * A helper macro that returns the header filename of a MTB ML Model.
 *
 * \def MTB_ML_MODEL_BIN(m)
 * A helper macro that returns the address of a MTB ML Model's data.
 *
 * \def MTB_ML_MODEL_SIZE(m)
 * A helper macro that returns the size of a MTB Model's data.
 *
 * \def MTB_ML_INCLUDE_MODEL_X_DATA_FILE(m)
 * A helper macro that returns the data filename of a MTB ML Model's sample data.
 *
 * \def MTB_ML_MODEL_X_DATA_BIN(m)
 * A helper macro that returns the address of a MTB ML Model's sample data.
 *
 * \def MTB_ML_INCLUDE_MODEL_Y_DATA_FILE(m)
 * A helper macro that returns the data filename of a MTB ML Model's reference result.
 *
 * \def MTB_ML_MODEL_Y_DATA_BIN(m)
 * A helper macro that returns the address of a MTB ML Model's reference result.
 */
#define MTB_ML_MODEL_NAME_STR(m)             ML_MODEL_NAME_STR_IMPL(m)
#define MTB_ML_INCLUDE_MODEL_FILE(m)         ML_INCLUDE_MODEL_FILE_IMPL(m)
#define MTB_ML_MODEL_BIN(m)                  ML_MODEL_BIN_IMPL(m)
#define MTB_ML_MODEL_SIZE(m)                 ML_MODEL_SIZE_IMPL(m)
#define MTB_ML_INCLUDE_MODEL_X_DATA_FILE(m)  ML_INCLUDE_MODEL_X_DATA_FILE_IMPL(m)
#define MTB_ML_MODEL_X_DATA_BIN(m)           ML_MODEL_X_DATA_BIN_IMPL(m)
#define MTB_ML_INCLUDE_MODEL_Y_DATA_FILE(m)  ML_INCLUDE_MODEL_Y_DATA_FILE_IMPL(m)
#define MTB_ML_MODEL_Y_DATA_BIN(m)           ML_MODEL_Y_DATA_BIN_IMPL(m)

/*!
 * \def MTB_ML_MODEL_BIN_DATA(x)
 * A helper macro that populates mtb_ml_model_bin_t structure with MTB ML Model's data.
 */
#define MTB_ML_MODEL_BIN_DATA(x)             ML_MODEL_BIN_DATA_IMPL(x)

/*!
 * \def MTB_ML_MODEL_INFERENCE_ERROR(err)
 * A helper macro that prints out the error information.
 */
#define MTB_ML_MODEL_INFERENCE_ERROR(err)    ML_MODEL_INFERENCE_ERROR_IMPL(err)

/**
 * @} end of Common_Macro group
 */

/******************************************************************************
 * Typedefs
 *****************************************************************************/
/**
 * A enum type for MTB ML profiling configuration
 */
typedef enum
{
    MTB_ML_PROFILE_DISABLE                     = 0,
    MTB_ML_PROFILE_ENABLE_MODEL                = 1,
    MTB_ML_PROFILE_ENABLE_LAYER                = 2,
    MTB_ML_PROFILE_ENABLE_MODEL_PER_FRAME      = 4,
    MTB_ML_PROFILE_ENABLE_LAYER_PER_FRAME      = 8,
    MTB_ML_LOG_ENABLE_MODEL_LOG                = 16
} mtb_ml_profile_config_t;

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
#define MTB_ML_RESULT_NPU_INIT_ERROR     CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_ML, 9)
#define MTB_ML_RESULT_CYCLE_COUNT_ERROR  CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_ML, 10)

/******************************************************************************
* Structures
******************************************************************************/

#if defined(__cplusplus)
}
#endif

#endif /* __MTB_ML_COMMON_H__ */
