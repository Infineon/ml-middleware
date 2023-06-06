/******************************************************************************
* File Name: mtb_ml_model.cpp
*
* Description: This file contains API calls to initialize and invoke Tflite-Micro
*              inference without interpreter for model generated from
*              MTB ML software.
*
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "mtb_ml_model.h"
#include <tensorflow/lite/kernels/kernel_util.h>

extern "C" {

/* LCOV_EXCL_START (Excluded from the code coverage, until the STOP marker) */
int __attribute__((weak)) mtb_ml_model_profile_get_tsc(uint32_t *val)
{
    return 0;
}
/* LCOV_EXCL_STOP */

}

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

static int input_elements(tflm_rmf_apis_t *rmf_api)
{
    int count = 1;
    for (int i = 0; i < rmf_api->model_input_dims_len(0); ++i)
    {
        count *= rmf_api->model_input_dims(0)[i];
    }
    return count;
}

static int output_elements(tflm_rmf_apis_t *rmf_api)
{
    int count = 1;
    for (int i = 0; i < rmf_api->model_output_dims_len(0); ++i)
    {
        count *= rmf_api->model_output_dims(0)[i];
    }
    return count;
}

static MTB_ML_DATA_T * input_ptr(tflm_rmf_apis_t *rmf_api)
{
    return (MTB_ML_DATA_T *) rmf_api->model_input_ptr(0);
}

static MTB_ML_DATA_T * output_ptr(tflm_rmf_apis_t *rmf_api)
{
    return (MTB_ML_DATA_T *) rmf_api->model_output_ptr(0);
}

cy_rslt_t mtb_ml_model_init(const mtb_ml_model_bin_t *bin, const mtb_ml_model_buffer_t *buffer, mtb_ml_model_t **object)
{
    mtb_ml_model_t *model_object = NULL;
    (void) buffer;

    /* Sanity check of input parameters */
    if (bin == NULL || object == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    /* Allocate runtime object */
    model_object = (mtb_ml_model_t *)calloc(1, sizeof(mtb_ml_model_t));

    if (model_object == NULL)
    {
        return MTB_ML_RESULT_ALLOC_ERR;
    }

    /* Copy the model name */
    memcpy(model_object->name, bin->name, MTB_ML_MODEL_NAME_LEN);

    /* Get model API structure */
    model_object->rmf_apis = bin->rmf_bin;
    model_object->tflm_obj = (void *)&model_object->rmf_apis;

    /* Get model and buffer size */
    model_object->model_size = bin->rmf_bin.model_data_size;
    model_object->buffer_size = bin->rmf_bin.model_buffer_size;

    /* Init the model */
    tflm_rmf_apis_t * rmf_api = (tflm_rmf_apis_t *) model_object->tflm_obj;
    if ( rmf_api->model_init() != kTfLiteOk )
    {
        free(model_object);
        return MTB_ML_RESULT_BAD_MODEL;
    }

    /* Get model parameters */
    model_object->input = input_ptr(rmf_api);
    model_object->input_size = input_elements(rmf_api);
    model_object->input_zero_point = rmf_api->model_input(0)->params.zero_point;
    model_object->input_scale = rmf_api->model_input(0)->params.scale;
    model_object->output_size = output_elements(rmf_api);
    model_object->output = output_ptr(rmf_api);
    model_object->output_zero_point = rmf_api->model_output(0)->params.zero_point;
    model_object->output_scale = rmf_api->model_output(0)->params.scale;

    *object = model_object;
    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Delete TfLite model runtime object and free all dynamically allocated memory
 */
cy_rslt_t mtb_ml_model_deinit(mtb_ml_model_t *object)
{
    /* Sanity check of input parameters */
    if (object == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }
    free(object);

    return MTB_ML_RESULT_SUCCESS;
}


/**
 * Perform NN model inference
 */
cy_rslt_t mtb_ml_model_run(mtb_ml_model_t *object, MTB_ML_DATA_T *input)
{
    TfLiteStatus ret;
    /* Sanity check of input parameters */
    if (object == NULL || input == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    tflm_rmf_apis_t *rmf_api = (tflm_rmf_apis_t *) object->tflm_obj;

    /* Set input data */
    for (size_t i = 0; i < rmf_api->model_inputs(); ++i)
    {
        memcpy(rmf_api->model_input_ptr(i), input, rmf_api->model_input_size(i));
        input += rmf_api->model_input_size(i) / sizeof(MTB_ML_DATA_T);
    }

    /* Model profiling */
    if (object->profiling & MTB_ML_PROFILE_ENABLE_MODEL)
    {
        mtb_ml_model_profile_get_tsc(&object->m_cycles);
    }
    ret = rmf_api->model_invoke();
    if ( ret != kTfLiteOk )
    {
        object->lib_error = ret;
        return MTB_ML_RESULT_INFERENCE_ERROR;
    }

    if (object->profiling & MTB_ML_PROFILE_ENABLE_MODEL)
    {
        uint32_t cycles;
        mtb_ml_model_profile_get_tsc(&cycles);
        object->m_cycles = cycles - object->m_cycles;
        if (object->m_cycles > object->m_peak_cycles)
        {
            object->m_peak_cycles = object->m_cycles;
            object->m_peak_frame = object->m_sum_frames;
        }
        object->m_sum_frames++;
        object->m_sum_cycles += object->m_cycles;
    }
    else if (object->profiling & MTB_ML_LOG_ENABLE_MODEL_LOG)
    {
       MTB_ML_DATA_T * output_ptr = object->output;
       for (int j = 0; j < object->output_size; j++)
       {
           printf("%6.3f ", (float) (output_ptr[j]));
       }
       printf("\r\n");
    }

    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Get NN model output buffer pointer and its size
 */
cy_rslt_t mtb_ml_model_get_output(const mtb_ml_model_t *object, MTB_ML_DATA_T **output_pptr, int *size_ptr)
{
    /* Sanity check of input parameters */
    if (object == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    if (output_pptr != NULL)
    {
        *output_pptr = object->output;
    }

    if (size_ptr != NULL)
    {
        *size_ptr = object->output_size;
    }

    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Get NN model input buffer size
 */
int mtb_ml_model_get_input_size(const mtb_ml_model_t *object)
{
    return object->input_size;
}

/**
 * Update MTB ML inference profiling setting
 */
cy_rslt_t mtb_ml_model_profile_config(mtb_ml_model_t *object, mtb_ml_profile_config_t config)
{
    /* Sanity check of input parameters */
    if (object == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    object->profiling = config;
    if (object->profiling != MTB_ML_PROFILE_DISABLE)
    {
        object->m_sum_frames = 0;
        object->m_sum_cycles = 0;
        object->m_peak_frame = 0;
        object->m_peak_cycles = 0;
    }
    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Generate MTB ML profiling log
 */
cy_rslt_t mtb_ml_model_profile_log(mtb_ml_model_t *object)
{
    /* Sanity check of input parameters */
    if (object == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    if (object->profiling & MTB_ML_PROFILE_ENABLE_MODEL)
    {
        printf("PROFILE_INFO, MTB ML model profile, avg_cyc=%-10.2f, peak_cyc=%-" PRIu32 ", peak_frame=%-" PRIu32 "\r\n",
                (float)object->m_sum_cycles / object->m_sum_frames,
                object->m_peak_cycles,
                object->m_peak_frame);
    }

    return MTB_ML_RESULT_SUCCESS;
}

/* LCOV_EXCL_START (Excluded from the code coverage, until the STOP marker) */
/**
 * Get the number of frame in recurrent model time series
 */
int mtb_ml_model_get_recurrent_time_series_frames(const mtb_ml_model_t *object)
{
    (void) object;
    return 0;
}

/**
 * Update recurrent NN reset state
 */
cy_rslt_t mtb_ml_model_rnn_state_control(mtb_ml_model_t *object, int status, int win_size)
{
    (void) object;
    (void) status;
    (void) win_size;
    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Set NN model input data Q-format fraction bits
 */
cy_rslt_t mtb_ml_model_set_input_q_fraction_bits(mtb_ml_model_t *object, uint8_t bits)
{
    (void) object;
    (void) bits;
    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Get NN model output data Q-format fraction bits
 */
uint8_t mtb_ml_model_get_output_q_fraction_bits(const mtb_ml_model_t *object)
{
    (void) object;
    return 0;
}
/* LCOV_EXCL_STOP */

#ifdef __cplusplus
}
#endif  // __cplusplus
