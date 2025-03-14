/******************************************************************************
* File Name: mtb_ml_model.cpp
*
* Description: This file contains API calls to initialize and invoke Tflite-Micro
*              inference without interpreter for model generated from
*              MTB ML software.
*
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "mtb_ml.h"
#include <tensorflow/lite/kernels/kernel_util.h>

extern "C" {

/* LCOV_EXCL_START (Excluded from the code coverage, until the STOP marker) */
int __attribute__((weak)) mtb_ml_model_profile_get_tsc(uint64_t *val)
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

/*******************************************************************************
 * Public Functions
*******************************************************************************/
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
    /* Input parameters */
    model_object->input = input_ptr(rmf_api);
    model_object->input_size = input_elements(rmf_api);
    model_object->input_zero_point = rmf_api->model_input(0)->params.zero_point;
    model_object->input_scale = rmf_api->model_input(0)->params.scale;
    model_object->model_time_steps = rmf_api->model_input(0)->dims->data[1];
    /* Output parameters*/
    model_object->output_size = output_elements(rmf_api);
    model_object->output = output_ptr(rmf_api);
    model_object->output_zero_point = rmf_api->model_output(0)->params.zero_point;
    model_object->output_scale = rmf_api->model_output(0)->params.scale;

    *object = model_object;
    return MTB_ML_RESULT_SUCCESS;
}

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
        mtb_ml_model_profile_get_tsc(&object->m_cpu_cycles);
#if (!defined(COMPONENT_RTOS) && \
      defined(COMPONENT_NNLITE2))
        mtb_ml_npu_cycles = 0;
#endif
    }
    ret = rmf_api->model_invoke();
    if ( ret != kTfLiteOk )
    {
        object->lib_error = ret;
        return MTB_ML_RESULT_INFERENCE_ERROR;
    }

    if (object->profiling & MTB_ML_PROFILE_ENABLE_MODEL)
    {
        uint64_t cycles = 0U;
        mtb_ml_model_profile_get_tsc(&cycles);
        uint64_t cpu_cycles_only = cycles - object->m_cpu_cycles;
#if (!defined(COMPONENT_RTOS) && \
      defined(COMPONENT_NNLITE2))
        /* mtb_ml_init() : mtb_ml_norm_clk_freq = npu_freq/cpu_freq */
        uint64_t norm_npu_cycles = (uint64_t)(((float)mtb_ml_npu_cycles) / mtb_ml_norm_clk_freq);
        /* Check for bad cpu/npu count values so we don't overflow */
        if (norm_npu_cycles > cpu_cycles_only)
        {
            return MTB_ML_RESULT_CYCLE_COUNT_ERROR;
        }

        object->m_npu_cycles = mtb_ml_npu_cycles;
        if (object->m_npu_cycles > object->m_npu_peak_cycles)
        {
            object->m_npu_peak_cycles = object->m_npu_cycles;
            object->m_npu_peak_frame = object->m_sum_frames;
        }
        object->m_npu_sum_cycles += object->m_npu_cycles;
        /* Subtracting NPU fraction */
        cpu_cycles_only -= norm_npu_cycles;
#endif
        if (cpu_cycles_only > object->m_cpu_peak_cycles)
        {
            object->m_cpu_peak_cycles = cpu_cycles_only;
            object->m_cpu_peak_frame = object->m_sum_frames;
        }
        object->m_cpu_sum_cycles += cpu_cycles_only;
        object->m_sum_frames++;
    }
    else if (object->profiling & MTB_ML_LOG_ENABLE_MODEL_LOG)
    {
       MTB_ML_DATA_T * output_ptr = object->output;
       /**
       * This string must track ML_PROFILE_OUTPUT_STRING in mtb_ml_stream.c,
       * as the header file is currently unable to be included due to conflicts.
       */
       printf(" output:");
       for (int j = 0; j < object->output_size; j++)
       {
           printf("%6.3f ", (float) (output_ptr[j]));
       }
       printf("\r\n");
    }

    return MTB_ML_RESULT_SUCCESS;
}

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

int mtb_ml_model_get_input_size(const mtb_ml_model_t *object)
{
    return object->input_size;
}

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
        object->m_cpu_sum_cycles = 0;
        object->m_cpu_peak_frame = 0;
        object->m_cpu_peak_cycles = 0;
    }
#if defined(COMPONENT_U55) || \
    defined(COMPONENT_NNLITE2)
        object->m_npu_sum_cycles = 0;
        object->m_npu_peak_frame = 0;
        object->m_npu_peak_cycles = 0;
#endif
    return MTB_ML_RESULT_SUCCESS;
}

cy_rslt_t mtb_ml_model_profile_log(mtb_ml_model_t *object)
{
    /* Sanity check of input parameters */
    if (object == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    if (object->profiling & MTB_ML_PROFILE_ENABLE_MODEL)
    {
        printf("PROFILE_INFO, MTB ML model profile, avg_cpu_cyc=%-10.2f, peak_cpu_cyc=%.0f, peak_cpu_frame=%-" PRIu32 ", cpu_freq_Mhz=%-" PRIu32 "\r\n",
                (float)object->m_cpu_sum_cycles / object->m_sum_frames,
                (float)object->m_cpu_peak_cycles,
                object->m_cpu_peak_frame,
                mtb_ml_cpu_clk_freq / 1000000);
#if defined(COMPONENT_U55) || \
    defined(COMPONENT_NNLITE2)
        printf("PROFILE_INFO, MTB ML model profile, avg_npu_cyc=%-10.2f, peak_npu_cyc=%.0f, peak_npu_frame=%-" PRIu32 ", npu_freq_Mhz=%-" PRIu32 "\r\n",
                (float)object->m_npu_sum_cycles / object->m_sum_frames,
                (float)object->m_npu_peak_cycles,
                object->m_npu_peak_frame,
                mtb_ml_npu_clk_freq / 1000000);
#endif
    }

    return MTB_ML_RESULT_SUCCESS;
}

cy_rslt_t mtb_ml_model_rnn_reset_all_parameters(mtb_ml_model_t *object)
{
    /* Sanity check of model object parameter */
    if (object == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }
    tflm_rmf_apis_t * rmf_api = (tflm_rmf_apis_t *) object->tflm_obj;
    if ( rmf_api->model_reset() != kTfLiteOk )
    {
        return MTB_ML_RESULT_BAD_ARG;
    }
    return MTB_ML_RESULT_SUCCESS;
}

/* LCOV_EXCL_STOP */

#ifdef __cplusplus
}
#endif  // __cplusplus
