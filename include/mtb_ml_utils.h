/***************************************************************************//**
* \file mtb_ml_utils.h
*
* \brief
* This the header file of ModusToolbox ML middleware utility module
*
*******************************************************************************
* (c) 2019-2024, Cypress Semiconductor Corporation (an Infineon company) or
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
#if !defined(__MTB_ML_UTILS_H__)
#define __MTB_ML_UTILS_H__

#include "mtb_ml_common.h"
#include "mtb_ml_model.h"

#if defined(__cplusplus)
extern "C" {
#endif

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Typedefs
 *****************************************************************************/

/******************************************************************************
 * Public definitions
******************************************************************************/


/******************************************************************************
 * Structures
******************************************************************************/

/*******************************************************************************
 * Function Prototypes
*******************************************************************************/
/**
 * \addtogroup Utils_API
 * @{
 */

/**
 * \brief : This function finds the maximum value in an array and return its index.
 *
 * \param[in]   in          : Pointer of the array
 * \param[in]   size        : size of the array
 *
 * \return                  : The index of maximum value
 *                          : -1 if input paramter is invalid.
 */
int mtb_ml_utils_find_max(const MTB_ML_DATA_T* in, int size);

/**
 * \brief : This function finds the maximum value in an int32_t array and return its index.
 *
 * \param[in]   in          : Pointer of the array
 * \param[in]   size        : size of the array
 *
 * \return                  : The index of maximum value
 *                          : -1 if input paramter is invalid.
 */
int mtb_ml_utils_find_max_int32(const int32_t* in, int size);

/**
 * \brief : Print detailed model info
 *
 * \param[in] obj        : Pointer of model object.
 *
 * \return               : MTB_ML_RESULT_SUCCESS - success
 *                       : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
 */
cy_rslt_t mtb_ml_utils_print_model_info(const mtb_ml_model_t *obj);

/**
 * \brief : Quantizes float data for model input. This function will always attempt to quantize the provided input data.
 * In order to properly use this function the end user must give the mtb ml model object, pointer to the input data of
 * floating point values and a pointer to a input buffer to contain all quantized values.
 *
 * \param[in] obj        : Pointer of model object.
 * \param[in] input_data : Pointer to input floating point data
 * \param[out] quantized_values : Input buffer to contain all quantized values. Its size should be allocated to match the number of input nodes within the model.
 *
 * \return               : MTB_ML_RESULT_SUCCESS - success
 *                       : MTB_ML_RESULT_BAD_ARG - if an input parameter is invalid.
 */
cy_rslt_t mtb_ml_utils_model_quantize(const mtb_ml_model_t *obj, const float* input_data, MTB_ML_DATA_T* quantized_values);

/**
 * \brief : Dequantize model's output if it is quantized
 *
 * \param[in] obj        : Pointer of model object.
 * \param[out] dequantized_values : pointer of dequantized_values
 *
 * \return               : MTB_ML_RESULT_SUCCESS - success
 *                       : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
 */
cy_rslt_t mtb_ml_utils_model_dequantize(const mtb_ml_model_t *obj, float* dequantized_values);

/**
 * @} end of Utils_API group
 */

#if defined(__cplusplus)
}
#endif

#endif /* __MTB_ML_UTILS_H__ */
