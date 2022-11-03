/***************************************************************************//**
* \file mtb_ml_model.c
*
* \brief
* The file contains application programming interface to the ModusToolbox ML
* middleware model module
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
#include <string.h>

#include "mtb_ml_common.h"
#include "mtb_ml_model.h"

/* LCOV_EXCL_START (Excluded from the code coverage, until the STOP marker) */
int __attribute__((weak)) mtb_ml_model_profile_get_tsc(uint32_t *val)
{
    return 0;
}
/* LCOV_EXCL_STOP */

static cy_en_ml_profile_config_t convert_profile_config(mtb_ml_profile_config_t config)
{
    switch (config)
    {
        case MTB_ML_PROFILE_DISABLE:
            return CY_ML_PROFILE_DISABLE;
        case MTB_ML_PROFILE_ENABLE_MODEL:
            return CY_ML_PROFILE_ENABLE_MODEL;
        case MTB_ML_PROFILE_ENABLE_LAYER:
            return CY_ML_PROFILE_ENABLE_LAYER;
        case MTB_ML_PROFILE_ENABLE_MODEL_PER_FRAME:
            return CY_ML_PROFILE_ENABLE_MODEL_PER_FRAME;
        case MTB_ML_PROFILE_ENABLE_LAYER_PER_FRAME:
            return CY_ML_PROFILE_ENABLE_LAYER_PER_FRAME;
        case MTB_ML_LOG_ENABLE_MODEL_LOG:
            return CY_ML_LOG_ENABLE_MODEL_LOG;
        default:
            /* disable profiling if the config is not supported */
            return CY_ML_PROFILE_DISABLE;
    }
}

/**
 *  Allocate and initialize NN model runtime object
 */
cy_rslt_t mtb_ml_model_init(const mtb_ml_model_bin_t *bin, const mtb_ml_model_buffer_t *buffer, mtb_ml_model_t **object)
{
    mtb_ml_model_t *model_object = NULL;
    int status;
    cy_rslt_t ret;

    /* Sanity check of input parameters */
    if (bin == NULL || bin->prms_bin == NULL || bin->wts_bin == NULL || object == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    /* Allocate runtime object */
    model_object = calloc(1, sizeof(mtb_ml_model_t));

    if (model_object == NULL)
    {
        return MTB_ML_RESULT_ALLOC_ERR;
    }

    /* Copy the model name */
    memcpy(model_object->name, bin->name, MTB_ML_MODEL_NAME_LEN);

    /* Get model buffer size */
    model_object->model_size = bin->wts_size + bin->prms_size;

    /* Get the NN model information by parsing the model parameters data */
    status = Cy_ML_Model_Parse((char *) bin->prms_bin, &model_object->model_info);
    if (status != CY_ML_SUCCESS)
    {
        ret = MTB_ML_RESULT_BAD_MODEL;
        goto RET_ERR;
    }

    /* Get workign buffer size */
    model_object->buffer_size = model_object->model_info.scratch_mem + model_object->model_info.persistent_mem;

    model_object->input_size = model_object->model_info.input_size;
    model_object->output_size = model_object->model_info.output_size;

    /* Allocate persistent buffer */
    if(buffer == NULL || buffer->persistent == NULL)
    {
        model_object->p_mem = malloc(model_object->model_info.persistent_mem);
        if (model_object->p_mem == NULL)
        {
            ret = MTB_ML_RESULT_ALLOC_ERR;
            goto RET_ERR;
        }
        model_object->flags |= MTB_ML_MEM_DYNAMIC_PERSISTENT;
    }
    else
    {
        model_object->p_mem = buffer->persistent;
    }

    /* Allocate scratch buffer */
    if(buffer == NULL || buffer->scratch == NULL)
    {
        model_object->s_mem = malloc(model_object->model_info.scratch_mem);
        if (model_object->s_mem == NULL)
        {
            ret = MTB_ML_RESULT_ALLOC_ERR;
            goto RET_ERR;
        }
        model_object->flags |= MTB_ML_MEM_DYNAMIC_SCRATCH;
    }
    else
    {
        model_object->s_mem = buffer->scratch;
    }

    /* Allocate output buffer */
    model_object->output = malloc(model_object->model_info.output_size * sizeof(MTB_ML_DATA_T));
    if (model_object->output == NULL)
    {
        ret = MTB_ML_RESULT_ALLOC_ERR;
        goto RET_ERR;
    }

    /* Initialize model and get model object */
    status = Cy_ML_Model_Init(&model_object->inference_obj,
                              (char *)bin->prms_bin, (char *)bin->wts_bin,
                              model_object->p_mem, model_object->s_mem,
                              &model_object->model_info);

    if (status == CY_ML_SUCCESS)
    {
        *object = model_object;
        return  MTB_ML_RESULT_SUCCESS;
    }

    ret = MTB_ML_RESULT_BAD_MODEL;

RET_ERR:

    if (model_object->p_mem && model_object->flags & MTB_ML_MEM_DYNAMIC_PERSISTENT)
    {
        free(model_object->p_mem);
        model_object->p_mem = NULL;
    }
    if (model_object->s_mem && model_object->flags & MTB_ML_MEM_DYNAMIC_SCRATCH)
    {
        free(model_object->s_mem);
        model_object->s_mem = NULL;
    }
    if (model_object->output)
    {
        free(model_object->output);
    }
    free(model_object);
    return ret;
}

/**
 * Delete NN model runtime object and free all dynamically allocated memory
 */
cy_rslt_t mtb_ml_model_deinit(mtb_ml_model_t *object)
{
    /* Sanity check of input parameters */
    if (object == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    if (object->p_mem && object->flags & MTB_ML_MEM_DYNAMIC_PERSISTENT)
    {
        free(object->p_mem);
        object->p_mem = NULL;
    }
    if (object->s_mem && object->flags & MTB_ML_MEM_DYNAMIC_SCRATCH)
    {
        free(object->s_mem);
        object->s_mem = NULL;
    }
    if (object->output)
    {
        free(object->output);
    }
    free(object);
    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Perform NN model inference
 */
cy_rslt_t mtb_ml_model_run(mtb_ml_model_t *object, MTB_ML_DATA_T *input)
{
    int ret;
    /* Sanity check of input parameters */
    if (object == NULL || input == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    /* Initialize output q-factor - acting as input and output q-factor */
    object->output_q_n = object->input_q_n;

    ret = Cy_ML_Model_Inference(object->inference_obj, input, object->output,
#if COMPONENT_ML_FLOAT32
                                NULL
#else
                                &object->output_q_n
#endif
                               );

    if (ret != CY_ML_SUCCESS)
    {
        object->lib_error = ret;
        return MTB_ML_RESULT_INFERENCE_ERROR;
    }

    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Get NN model input data size
 */
int mtb_ml_model_get_input_size(const mtb_ml_model_t *object)
{
    if (object == NULL)
    {
        return 0;
    }
    else
    {
        return object->input_size;
    }
}

/**
 * Get NN model output data size
 */
cy_rslt_t mtb_ml_model_get_output(const mtb_ml_model_t *object, MTB_ML_DATA_T **output_pptr, int* size_ptr)
{
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
 * Get the number of frame in recurrent model time series
 */
int mtb_ml_model_get_recurrent_time_series_frames(const mtb_ml_model_t *object)
{
    if (object == NULL)
    {
        return 0;
    }
    else
    {
        return object->model_info.recurrent_ts_size;
    }
}

/**
 * Rest recurrent NN state
 */
cy_rslt_t mtb_ml_model_rnn_state_control(mtb_ml_model_t *object, int status, int win_size)
{
    int result;
    if (object == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    result = Cy_ML_Rnn_State_Control(object->inference_obj, status, win_size);
    if (result != CY_ML_SUCCESS)
    {
        object->lib_error = result;
        return MTB_ML_RESULT_BAD_ARG;
    }
    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Get MTB ML inference runtime object
 */
void* mtb_ml_model_get_inference_object(const mtb_ml_model_t *object)
{
    if (object == NULL)
    {
        return NULL;
    }
    else
    {
        return object->inference_obj;
    }
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

    Cy_ML_Profile_Control(object->inference_obj, convert_profile_config(config));
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

    Cy_ML_Profile_Print(object->inference_obj);
    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Set NN model input data Q-format fraction bits
 */
cy_rslt_t mtb_ml_model_set_input_q_fraction_bits(mtb_ml_model_t *object, uint8_t bits)
{
    /* Sanity check of input parameters */
    if (object == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    object->input_q_n = bits;
    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Get NN model output data Q-format fraction bits
 */
uint8_t mtb_ml_model_get_output_q_fraction_bits(const mtb_ml_model_t *object)
{
    /* Sanity check of input parameters */
    if (object == NULL)
    {
        return 0;
    }
#if COMPONENT_ML_FLOAT32
    return 0;
#else
    return object->output_q_n;
#endif
}

inline int Cy_ML_Profile_Get_Tsc( uint32_t *val )
{
    return mtb_ml_model_profile_get_tsc(val);
}

