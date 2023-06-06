/***************************************************************************//**
* \file mtb_ml_utils.c
*
* \brief
* The file contains application programming interface to the ModusToolbox ML
* middleware utility module
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
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <limits.h>
#include "mtb_ml_common.h"
#include "mtb_ml_utils.h"

#if defined(MTB_ML_HAVING_CMSIS_DSP)
#include "arm_math.h"
#endif

#define INT_TO_FLT(in, out, size, q, max_q) \
do { \
    int loop_count; \
    float norm; \
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
int mtb_ml_utils_find_max(const MTB_ML_DATA_T* in, int size)
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
int mtb_ml_utils_find_max_int32(const int32_t* in, int size)
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
cy_rslt_t mtb_ml_utils_convert_int_to_flt(const MTB_ML_DATA_T* in, float *out, int size, int q)
{
    int max_q = ((sizeof(MTB_ML_DATA_T) * 8) - 1);

    INT_TO_FLT(in, out, size, q, max_q);
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
cy_rslt_t mtb_ml_utils_convert_flt_to_int8(const float* in, int8_t *out, int size, int q)
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
#if defined(MTB_ML_HAVING_CMSIS_DSP)
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
#else
    loop_count = size;
    while (loop_count > 0)
    {
        val = (*in++) * norm;
        val += val > 0.0f ? 0.5f : -0.5f;
        if ((int32_t) val > SCHAR_MAX)
            *out++ = SCHAR_MAX;
        else if ((int32_t) val < SCHAR_MIN)
            *out++ = SCHAR_MIN;
        else
            *out++ = (int8_t) (val);

        loop_count--;
    }
#endif /* MTB_ML_HAVING_CMSIS_DSP */

    return MTB_ML_RESULT_SUCCESS;
}

/**
 * \brief : This function converts an array of floating-point to a 8-bits fixed-point integer for TFLiteU.
 *
 * \param[in]   in          : pointer of input array in floating-point
 * \param[out]  out         : pointer of output array in 8-bits
 * \param[in]   size        : size of the array
 * \param[in]   scale       : scale of output data
 * \param[in]   zero_point  : zero point of the output data
 *
 * \return                  : MTB_ML_RESULT_SUCCESS - success
 *                          : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
*/
static cy_rslt_t mtb_ml_utils_convert_tflm_flt_to_int8(const float* in, int8_t *out, int size, float scale, int zero_point)
{
    int loop_count;
    float val;

    /* Sanity check of input parameters */
    if (in == NULL || out == NULL || size <= 0 || scale <= 0)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

#if defined(MTB_ML_HAVING_CMSIS_DSP)
    /* Process 4 output at one time */
    loop_count = size >> 2u;
    while (loop_count > 0)
    {
        val = *in++;
        val = (val / scale) + zero_point;
        val += val > 0.0f ? 0.5f : -0.5f;
        *out++ = (int8_t) (__SSAT((int32_t) (val), 8));

        val = *in++;
        val = (val / scale) + zero_point;
        val += val > 0.0f ? 0.5f : -0.5f;
        *out++ = (int8_t) (__SSAT((int32_t) (val), 8));

        val = *in++;
        val = (val / scale) + zero_point;
        val += val > 0.0f ? 0.5f : -0.5f;
        *out++ = (int8_t) (__SSAT((int32_t) (val), 8));

        val = *in++;
        val = (val / scale) + zero_point;
        val += val > 0.0f ? 0.5f : -0.5f;
        *out++ = (int8_t) (__SSAT((int32_t) (val), 8));

        loop_count--;
    }

    /* Process any remaining output */
    loop_count = size % 4;

    while (loop_count > 0)
    {
        val = *in++;
        val = (val / scale) + zero_point;
        val += val > 0.0f ? 0.5f : -0.5f;
        *out++ = (int8_t) (__SSAT((int32_t) (val), 8));

        loop_count--;
    }
#else
    loop_count = size;
    while (loop_count > 0)
    {
        val = (*in++ / scale) + zero_point;
        val += val > 0.0f ? 0.5f : -0.5f;
        if ((int32_t) val > SCHAR_MAX)
            *out++ = SCHAR_MAX;
        else if ((int32_t) val < SCHAR_MIN)
            *out++ = SCHAR_MIN;
        else
            *out++ = (int8_t) (val);

        loop_count--;
    }
#endif /* MTB_ML_HAVING_CMSIS_DSP */

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
cy_rslt_t mtb_ml_utils_convert_flt_to_int16(const float* in, int16_t *out, int size, int q)
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

#if defined(MTB_ML_HAVING_CMSIS_DSP)
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
#else
    loop_count = size;
    while (loop_count > 0)
    {
        val = (*in++) * norm;
        val += val > 0.0f ? 0.5f : -0.5f;
        if ((int32_t) val > SHRT_MAX)
            *out++ = SHRT_MAX;
        else if ((int32_t) val < SHRT_MIN)
            *out++ = SHRT_MIN;
        else
            *out++ = (int16_t) (val);

        loop_count--;
    }
#endif /* MTB_ML_HAVING_CMSIS_DSP */
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
cy_rslt_t mtb_ml_utils_convert_flt_to_int(const float* in, MTB_ML_DATA_T *out, int size, int q)
{
#if defined(COMPONENT_ML_INT8x8)
    return mtb_ml_utils_convert_flt_to_int8(in, out, size, q);
#elif defined(COMPONENT_ML_INT16x16) || defined(COMPONENT_ML_INT16x8)
    return mtb_ml_utils_convert_flt_to_int16(in, out, size, q);
#elif defined(COMPONENT_ML_FLOAT32) || defined (COMPONENT_ML_TFLM_INTERPRETER) || defined (COMPONENT_ML_TFLM_INTERPRETER_LESS)
    /* Do nothing */
    (void) q;
    (void) in;
    (void) out;
    (void) size;
    return  MTB_ML_RESULT_SUCCESS;
#endif
}

/**
 * \brief : Print detailed model info
 *
 * \param[in] obj        : Pointer of model object.
 *
 * \return               : MTB_ML_RESULT_SUCCESS - success
 *                       : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
 */
cy_rslt_t mtb_ml_utils_print_model_info(const mtb_ml_model_t *obj)
{
    if (obj == NULL) {
        return MTB_ML_RESULT_BAD_ARG;
    }

    printf("\r\n***************************************************\r\n");

#if defined(COMPONENT_ML_IFX)
    const cy_stc_ml_model_info_t *model_info = &obj->model_info;
    printf("MTB IFX inference\r\n");
#elif defined(COMPONENT_ML_TFLM_INTERPRETER) || defined(COMPONENT_ML_TFLM_INTERPRETER_LESS)
    printf("MTB Tflite-Micro inference\r\n");
#endif
    printf("Model name       \t:\t%s\r\n", obj->name);
    printf("Model size       \t:\t%-7.2f kB\r\n", obj->model_size / 1024.0);
    printf("Buffer size      \t:\t%-7.2f kB\r\n", obj->buffer_size / 1024.0);
    printf("\r\n");
#if defined(COMPONENT_ML_IFX)
    printf("Layers           \t:\t%d\r\n", model_info->num_of_layers);
    printf("Input_nodes      \t:\t%d\r\n", model_info->input_size);
    printf("Output_nodes     \t:\t%d\r\n", model_info->output_size);
    printf("ML Lib Version   \t:\t%d\r\n", model_info->libml_version);
    printf("ML Lib Input Type\t:\t%s\r\n", model_info->libml_input_type == CY_ML_DATA_FLOAT ? "floating-point" :
                                             (model_info->libml_input_type == CY_ML_DATA_INT16 ? "int16" : "int8"));
    printf("ML Lib Weight Type\t:\t%s\r\n", model_info->libml_weight_type == CY_ML_DATA_FLOAT ? "floating-point" :
                                             (model_info->libml_weight_type == CY_ML_DATA_INT16 ? "int16" : "int8"));
    printf("ML Coretool Version\t:\t0x%"PRIx32"\r\n", model_info->ml_coretool_version);
#endif
    return MTB_ML_RESULT_SUCCESS;
}

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
cy_rslt_t mtb_ml_utils_model_quantize(const mtb_ml_model_t *obj, const float* input_data, MTB_ML_DATA_T* quantized_values)
{
    if (obj == NULL || input_data == NULL || quantized_values == NULL) {
        return MTB_ML_RESULT_BAD_ARG;
    }

#if !defined(COMPONENT_ML_FLOAT32)
    int32_t size = obj->input_size;
    const float *value = input_data;
#if defined(COMPONENT_ML_IFX)
    return mtb_ml_utils_convert_flt_to_int(value, quantized_values, size, obj->input_q_n);
#else
    return mtb_ml_utils_convert_tflm_flt_to_int8(value, quantized_values, size, obj->input_scale, obj->input_zero_point);
#endif
#else
    return MTB_ML_RESULT_SUCCESS;
#endif
}

/**
 * \brief : Dequantize model's output if it is quantized
 *
 * \param[in] obj        : Pointer of model object.
 * \param[out] dequantized_values : pointer of dequantized_values
 *
 * \return               : MTB_ML_RESULT_SUCCESS - success
 *                       : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
 */
cy_rslt_t mtb_ml_utils_model_dequantize(const mtb_ml_model_t *obj, float* dequantized_values)
{
    if (obj == NULL || dequantized_values == NULL) {
        return MTB_ML_RESULT_BAD_ARG;
    }
    int size = obj->output_size;
    MTB_ML_DATA_T *value = obj->output;

#if !defined(COMPONENT_ML_FLOAT32)
#if defined(COMPONENT_ML_IFX)

    return mtb_ml_utils_convert_int_to_flt(value, dequantized_values, size, obj->output_q_n);
#else
    int zero_point = obj->output_zero_point;
    float scale = obj->output_scale;
    while ( size > 0 )
    {
        *dequantized_values++ = ((int) (*value++) - zero_point) * scale;
        size--;
    }
    return MTB_ML_RESULT_SUCCESS;
#endif /* defined(COMPONENT_ML_IFX) */
#else  /* !defined(COMPONENT_ML_FLOAT32) */
    /* Copy the value over */
    while ( size > 0 )
    {
        *dequantized_values++ = *value++;
        size--;
    }
    return MTB_ML_RESULT_SUCCESS;
#endif /* !defined(COMPONENT_ML_FLOAT32) */
}
