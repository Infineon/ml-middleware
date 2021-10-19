/***************************************************************************//**
* \file mtb_ml_utils.h
*
* \brief
* This the header file of ModusToolbox ML middleware utility module
*
*******************************************************************************
* \copyright
* Copyright 2021, Cypress Semiconductor Corporation (an Infineon company).
* All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
******************************************************************************/


#if !defined(__MTB_ML_UTILS_H__)
#define __MTB_ML_UTILS_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "mtb_ml_common.h"

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
int mtb_ml_utils_find_max(MTB_ML_DATA_T* in, int size);

/**
 * \brief : This function finds the maximum value in an int32_t array and return its index.
 *
 * \param[in]   in          : Pointer of the array
 * \param[in]   size        : size of the array
 *
 * \return                  : The index of maximum value
 *                          : -1 if input paramter is invalid.
 */
int mtb_ml_utils_find_max_int32(int32_t* in, int size);

/**
 * \brief : This function converts an array of 8-bits Q-format fixed-point integer to floating-point
 *
 * \param[in]   in          : pointer of input array in Q-format fixed-point
 * \param[out]  out         : pointer of output array in floating-point
 * \param[in]   size        : size of the array
 * \param[in]   q           : fractional bits of Q-format fixed-point
 *
 * \return                  : MTB_ML_RESULT_SUCCESS - success
 *                          : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
 */
cy_rslt_t mtb_ml_utils_convert_int8_to_flt(int8_t* in, float *out, int size, int q);

/**
 * \brief : This function converts an array of 16-bits Q-format fixed-point integer to floating-point
 *
 * \param[in]   in          : pointer of input array in Q-format fixed-point
 * \param[out]  out         : pointer of output array in floating-point
 * \param[in]   size        : size of the array
 * \param[in]   q           : fractional bits of Q-format fixed-point
 *
 * \return                  : MTB_ML_RESULT_SUCCESS - success
 *                          : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
 */
cy_rslt_t mtb_ml_utils_convert_int16_to_flt(int16_t* in, float *out, int size, int q);

/**
 * \brief : This function converts an array of MTB ML integer to floating-point
 *
 * \param[in]   in          : pointer of input array in Q-format fixed-point
 * \param[out]  out         : pointer of output array in floating-point
 * \param[in]   size        : size of the array
 * \param[in]   q           : fractional bits of Q-format fixed-point
 *
 * \return                  : MTB_ML_RESULT_SUCCESS - success
 *                          : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
 */
cy_rslt_t mtb_ml_utils_convert_int_to_flt(MTB_ML_DATA_T* in, float *out, int size, int q);

/**
 * \brief : This function converts an array of floating-point to a Q-format fixed-point integer.
 *
 * \param[in]   in          : pointer of input array in floating-point
 * \param[out]  out         : pointer of output array in Q-format fixed-point
 * \param[in]   size        : size of the array
 * \param[in]   q           : fractional bits of Q-format fixed-point
 *
 * \return                  : MTB_ML_RESULT_SUCCESS - success
 *                          : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
 */
cy_rslt_t mtb_ml_utils_convert_flt_to_int(float* in, MTB_ML_DATA_T *out, int size, int q);


/**
 * \brief : This function converts an array of floating-point to a 16-bits Q-format fixed-point integer.
 *
 * \param[in]   in          : pointer of input array in floating-point
 * \param[out]  out         : pointer of output array in 16-bits Q-format fixed-point
 * \param[in]   size        : size of the array
 * \param[in]   q           : fractional bits of Q-format fixed-point
 *
 * \return                  : MTB_ML_RESULT_SUCCESS - success
 *                          : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
 */
cy_rslt_t mtb_ml_utils_convert_flt_to_int16(float* in, int16_t *out, int size, int q);

/**
 * \brief : This function converts an array of floating-point to a 8-bits Q-format fixed-point integer.
 *
 * \param[in]   in          : pointer of input array in floating-point
 * \param[out]  out         : pointer of output array in 8-bits Q-format fixed-point
 * \param[in]   size        : size of the array
 * \param[in]   q           : fractional bits of Q-format fixed-point
 *
 * \return                  : MTB_ML_RESULT_SUCCESS - success
 *                          : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
 */
cy_rslt_t mtb_ml_utils_convert_flt_to_int8(float* in, int8_t *out, int size, int q);

/**
 * @} end of Utils_API group
 */

#if defined(__cplusplus)
}
#endif

#endif /* __MTB_ML_UTILS_H__ */

