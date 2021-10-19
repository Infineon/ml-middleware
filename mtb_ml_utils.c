/***************************************************************************//**
* \file mtb_ml_utils.c
*
* \brief
* The file contains application programming interface to the ModusToolbox ML
* middleware utility module
*
*******************************************************************************
* \copyright
* Copyright 2021, Cypress Semiconductor Corporation (an Infineon company).
* All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
******************************************************************************/
#include "mtb_ml_common.h"
#include "mtb_ml_utils.h"

#include "arm_math.h"

#define INT_TO_FLT(in, out, size, q) \
do { \
    int loop_count; \
    float norm; \
    int max_q = ((sizeof(MTB_ML_DATA_T) * 8) - 1); \
\
    if (in == NULL || out == NULL || size <= 0 || q < 0 || q > max_q) \
    { \
        return MTB_ML_RESULT_BAD_ARG; \
    } \
\
    /* Calculate the floating Q normalization value */ \
    norm = (float) (1 << q); \
\
    /* Process 4 outputs at a time */ \
    loop_count = size >> 2; \
    while (loop_count > 0) \
    { \
        *out++ = ((float) *in++ / norm); \
        *out++ = ((float) *in++ / norm); \
        *out++ = ((float) *in++ / norm); \
        *out++ = ((float) *in++ / norm); \
\
        loop_count--; \
    } \
\
    /* Process remain output */ \
    loop_count = size % 4; \
    while (loop_count > 0) \
    { \
        *out++ = ((float) *in++ / norm); \
\
        loop_count--; \
    } \
\
    return MTB_ML_RESULT_SUCCESS; \
} while(0)

/**
 * \brief : This function finds the maximum value in an array and return its index.
 *
 * \param[in]   in          : Pointer of the array
 * \param[in]   size        : size of the array
 *
 * \return                  : The index of maximum value
 *                          : -1 if input paramter is invalid.
 */
int mtb_ml_utils_find_max(MTB_ML_DATA_T* in, int size)
{
    int loop_count, max_idx = -1;
    MTB_ML_DATA_T val, max_val;

    if (in != NULL && size > 0)
    {
        loop_count = size - 1;
        max_idx = 0;
        max_val = *in++;

        while(loop_count > 0)
        {
            val = *in++;
            if (val > max_val)
            {
                max_idx = size - loop_count;
                max_val = val;
            }
            loop_count--;
        }
    }
    return max_idx;
}

/**
 * \brief : This function finds the maximum value in an int32_t array and return its index.
 *
 * \param[in]   in          : Pointer of the array
 * \param[in]   size        : size of the array
 *
 * \return                  : The index of maximum value
 *                          : -1 if input paramter is invalid.
 */
int mtb_ml_utils_find_max_int32(int32_t* in, int size)
{
    int loop_count, max_idx = -1;
    int32_t val, max_val;

    if (in != NULL && size > 0)
    {
        loop_count = size - 1;
        max_idx = 0;
        max_val = *in++;

        while(loop_count > 0)
        {
            val = *in++;
            if (val > max_val)
            {
                max_idx = size - loop_count;
                max_val = val;
            }
            loop_count--;
        }
    }
    return max_idx;
}

/**
 * \brief : This function converts an array of 8-bits Q-format fixed-point integer to floating-point
 *
 * \param[in]   in          : pointer of input array in Q-format fixed-point
 * \param[out]  out         : pointer of output array in floating-point
 * \param[in]   size        : size of the array
 * \param[in]   q           : fixed-point Q factor
 *
 * \return                  : MTB_ML_RESULT_SUCCESS - success
 *                          : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
 */
cy_rslt_t mtb_ml_utils_convert_int8_to_flt(int8_t* in, float *out, int size, int q)
{
    INT_TO_FLT(in, out, size, q);
}

/**
 * \brief : This function converts an array of 16-bits Q-format fixed-point integer to floating-point
 *
 * \param[in]   in          : pointer of input array in Q-format fixed-point
 * \param[out]  out         : pointer of output array in floating-point
 * \param[in]   size        : size of the array
 * \param[in]   q           : fixed-point Q factor
 *
 * \return                  : MTB_ML_RESULT_SUCCESS - success
 *                          : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
 */
cy_rslt_t mtb_ml_utils_convert_int16_to_flt(int16_t* in, float *out, int size, int q)
{
    INT_TO_FLT(in, out, size, q);
}

/**
 * \brief : This function converts an array of MTB ML integer to floating-point
 *
 * \param[in]   in          : pointer of input array in Q-format fixed-point
 * \param[out]  out         : pointer of output array in floating-point
 * \param[in]   size        : size of the array
 * \param[in]   q           : fixed-point Q factor
 *
 * \return                  : MTB_ML_RESULT_SUCCESS - success
 *                          : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
 */
cy_rslt_t mtb_ml_utils_convert_int_to_flt(MTB_ML_DATA_T* in, float *out, int size, int q)
{
    INT_TO_FLT(in, out, size, q);
}

/**
 * \brief : This function converts an array of floating-point to a 8-bits Q-format fixed-point integer.
 *
 * \param[in]   in          : pointer of input array in floating-point
 * \param[out]  out         : pointer of output array in 8-bits Q-format fixed-point
 * \param[in]   size        : size of the array
 * \param[in]   q           : fixed-point Q factor (fraction bits)
 *
 * \return                  : MTB_ML_RESULT_SUCCESS - success
 *                          : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
*/
cy_rslt_t mtb_ml_utils_convert_flt_to_int8(float* in, int8_t *out, int size, int q)
{
    int loop_count;
    float norm, val;

    /* Sanity check of input parameters */
    if (in == NULL || out == NULL || size <= 0 || q < 0 || q > 7)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    /* Calculate the Q normalization value */
    norm = (float) (1 << q);

    /* Process 4 output at one timee */
    loop_count = size >> 2u;
    while (loop_count > 0)
    {
        val = *in++;
        val = (val * norm);
        val += val > 0.0f ? 0.5f : -0.5f;
        *out++ = (int8_t) (__SSAT((int32_t) (val), 8));

        val = *in++;
        val = (val * norm);
        val += val > 0.0f ? 0.5f : -0.5f;
        *out++ = (int8_t) (__SSAT((int32_t) (val), 8));

        val = *in++;
        val = (val * norm);
        val += val > 0.0f ? 0.5f : -0.5f;
        *out++ = (int8_t) (__SSAT((int32_t) (val), 8));

        val = *in++;
        val = (val * norm);
        val += val > 0.0f ? 0.5f : -0.5f;
        *out++ = (int8_t) (__SSAT((int32_t) (val), 8));

        loop_count--;
    }

    /* Process any remaining output */
    loop_count = size % 4;

    while (loop_count > 0)
    {
        val = *in++;
        val = (val * norm);
        val += val > 0.0f ? 0.5f : -0.5f;
        *out++ = (int8_t) (__SSAT((int32_t) (val), 8));

        loop_count--;
    }
    return MTB_ML_RESULT_SUCCESS;
}

/**
 * \brief : This function converts an array of floating-point to a 16-bits Q-format fixed-point integer.
 *
 * \param[in]   in          : pointer of input array in floating-point
 * \param[out]  out         : pointer of output array in 16-bits Q-format fixed-point
 * \param[in]   size        : size of the array
 * \param[in]   q           : fixed-point Q factor (fraction bits)
 *
 * \return                  : MTB_ML_RESULT_SUCCESS - success
 *                          : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
*/
cy_rslt_t mtb_ml_utils_convert_flt_to_int16(float* in, int16_t *out, int size, int q)
{
    int loop_count;
    float norm, val;

    /* Sanity check of input parameters */
    if (in == NULL || out == NULL || size <= 0 || q < 0 || q > 15)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    /* Calculate the Q normalization value */
    norm = (float) (1 << q);

    /* Process 4 output at a timee */
    loop_count = size >> 2u;

    while (loop_count > 0)
    {
        val = *in++;
        val = (val * norm);
        val += val > 0.0f ? 0.5f : -0.5f;
        *out++ = (int16_t) (__SSAT((int32_t) (val), 16));

        val = *in++;
        val = (val * norm);
        val += val > 0.0f ? 0.5f : -0.5f;
        *out++ = (int16_t) (__SSAT((int32_t) (val), 16));

        val = *in++;
        val = (val * norm);
        val += val > 0.0f ? 0.5f : -0.5f;
        *out++ = (int16_t) (__SSAT((int32_t) (val), 16));

        val = *in++;
        val = (val * norm);
        val += val > 0.0f ? 0.5f : -0.5f;
        *out++ = (int16_t) (__SSAT((int32_t) (val), 16));

        loop_count--;
    }

    /* Process any remaining output */
    loop_count = size % 4;

    while (loop_count > 0)
    {
        val = *in++;
        val = (val * norm);
        val += val > 0.0f ? 0.5f : -0.5f;
        *out++ = (int16_t) (__SSAT((int32_t) (val), 16));

        loop_count--;
    }
    return MTB_ML_RESULT_SUCCESS;
}

/**
 * \brief : This function converts an array of floating-point to MTB ML integer.
 *
 * \param[in]   in          : pointer of input array in floating-point
 * \param[out]  out         : pointer of output array in Q-format fixed-point
 * \param[in]   size        : size of the array
 * \param[in]   q           : fixed-point Q factor (fraction bits)
 *
 * \return                  : MTB_ML_RESULT_SUCCESS - success
 *                          : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
 */
cy_rslt_t mtb_ml_utils_convert_flt_to_int(float* in, MTB_ML_DATA_T *out, int size, int q)
{
#if defined(COMPONENT_ML_INT8x8)
    return mtb_ml_utils_convert_flt_to_int8(in, out, size, q);
#elif defined(COMPONENT_ML_INT16x16) || defined(COMPONENT_ML_INT16x8)
    return mtb_ml_utils_convert_flt_to_int16(in, out, size, q);
#elif defined(COMPONENT_ML_FLOAT32)
    /* Do nothing */
    (void) q;
    (void) in;
    (void) out;
    (void) size;
    return  MTB_ML_RESULT_SUCCESS;
#endif
}
