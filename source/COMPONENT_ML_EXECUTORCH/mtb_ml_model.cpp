/******************************************************************************
* File Name: mtb_ml_model.cpp
*
* Description: This file contains API calls to initialize and invoke ExecutorTorch
*              inference for model generated from CY ML software.
*
*
*******************************************************************************
* (c) 2026, Infineon Technologies AG, or
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

#include "mtb_ml_model.h"
#include "mtb_ml_common.h"
#include "mtb_ml_model_defs.h"
#include "mtb_executorch_wrapper.h"
#include "mtb_ml.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

extern "C" {

#if defined(COMPONENT_U55)
extern ethosu_driver *mtb_ml_ethosu_driver_handle;
#endif

/* Helper macro for safe type casting of inference object */
#define GET_ET_WRAPPER(obj) \
    (reinterpret_cast<executorch_mtb::MTBExecuTorch *>((obj)->tflm_obj))

/* Validate that the object has a valid ExecutorTorch wrapper */
#define VALIDATE_ET_WRAPPER(obj) \
    do { \
        if ((obj) == NULL || (obj)->tflm_obj == NULL) { \
            return MTB_ML_RESULT_BAD_ARG; \
        } \
    } while(0)

/* Validate that a tensor index is within bounds */
#define VALIDATE_TENSOR_INDEX(idx, count) \
    do { \
        if ((idx) < 0 || (idx) >= (count)) { \
            return MTB_ML_RESULT_BAD_ARG; \
        } \
    } while(0)

static void mtb_ml_model_free_object(mtb_ml_model_t *object)
{
    if (object == NULL)
    {
        return;
    }
    free(object->inputs);
    object->inputs = NULL;
    free(object->outputs);
    object->outputs = NULL;
    free(object->temp_allocation_pool);
    object->temp_allocation_pool = NULL;
    free(object->method_allocation_pool);
    object->method_allocation_pool = NULL;
    free(object);
}

/* Weak function for platform-specific TSC (Time Stamp Counter) */
int __attribute__((weak)) mtb_ml_model_profile_get_tsc(uint64_t *val)
{
    return 0;
}

/**
 * Helper: Resolve one ExecuTorch allocation pool from user input or heap fallback.
 */
static cy_rslt_t resolve_allocation_pool(
    uint8_t *user_pool,
    size_t user_pool_size,
    size_t default_pool_size,
    uint8_t **owned_pool,
    uint8_t **resolved_pool,
    size_t *resolved_pool_size)
{
    /* Check if user provided a pool - if so, use it */
    if (user_pool != NULL)
    {
        if (user_pool_size == 0U)
        {
            return MTB_ML_RESULT_BAD_ARG;
        }

        *resolved_pool = user_pool;
        *resolved_pool_size = user_pool_size;
        return MTB_ML_RESULT_SUCCESS;
    }

    if (default_pool_size == 0U)
    {
        *resolved_pool = NULL;
        *resolved_pool_size = 0U;
        return MTB_ML_RESULT_SUCCESS;
    }

    /* Fallback: Allocate pool from heap if user did not provide one */
#if defined (__ARMCC_VERSION)
    int ret_val = posix_memalign((void **)owned_pool, (size_t)16, default_pool_size);
    if (ret_val != 0)
    {
        return MTB_ML_RESULT_ALLOC_ERR;
    }
#else
    const size_t pool_alignment = 16U;
    size_t aligned_pool_size = ((default_pool_size + pool_alignment - 1U) / pool_alignment) * pool_alignment;
    *owned_pool = (uint8_t *)aligned_alloc(16, aligned_pool_size);
    if (*owned_pool == NULL)
    {
        return MTB_ML_RESULT_ALLOC_ERR;
    }
#endif

    *resolved_pool = *owned_pool;
    *resolved_pool_size = default_pool_size;
    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Helper: Create and validate ExecutorTorch wrapper
 */
static cy_rslt_t create_executorch_wrapper(
    const uint8_t *model_bin,
    size_t model_size,
    uint8_t *method_allocation_pool,
    size_t method_allocation_pool_size,
    uint8_t *temp_allocation_pool,
    size_t temp_allocation_pool_size,
    executorch_mtb::MTBExecuTorch **wrapper_out)
{
    executorch_mtb::MTBExecuTorch *wrapper = new executorch_mtb::MTBExecuTorch(
        model_bin,
        model_size,
        method_allocation_pool,
        method_allocation_pool_size,
        temp_allocation_pool,
        temp_allocation_pool_size);

    if (wrapper == NULL) {
        return MTB_ML_RESULT_ALLOC_ERR;
    }

    if (wrapper->AllocationStatus() != executorch::runtime::Error::Ok) {
        delete wrapper;
        return MTB_ML_RESULT_ALLOC_ERR;
    }

    *wrapper_out = wrapper;
    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Helper: Copy a single TensorMeta entry into a mtb_ml_tensor_desc_t
 */
static void fill_tensor_desc(mtb_ml_tensor_desc_t *desc, const executorch_mtb::MTBExecuTorch::TensorMeta &meta)
{
    desc->ptr                 = meta.ptr;
    desc->bytes               = meta.bytes;
    desc->elements            = meta.elements;
    desc->dim_len             = meta.dim_len;
    desc->dims                = meta.dims;
    desc->zero_point          = meta.zero_point.value_or(0);
    desc->scale               = meta.scale.value_or(1.0f);
    desc->concat_offset_bytes = static_cast<int>(meta.concat_offset_bytes);
    desc->type_size           = meta.type_size;
}

/**
 * Helper: Populate model input/output metadata
 */
static cy_rslt_t populate_io_metadata(
    mtb_ml_model_t *model_object,
    executorch_mtb::MTBExecuTorch *wrapper)
{
    model_object->input_count         = wrapper->input_count;
    model_object->output_count        = wrapper->output_count;
    model_object->input_concat_bytes  = wrapper->input_concat_bytes;
    model_object->output_concat_bytes = wrapper->output_concat_bytes;
    model_object->inputs              = NULL;
    model_object->outputs             = NULL;

    if (model_object->input_count > 0)
    {
        const auto &input_meta = wrapper->input_meta;

        /* For single-input models, populate legacy fields for backwards compatibility. */
        model_object->input           = input_meta[0].ptr;
        model_object->input_zero_point = input_meta[0].zero_point.value_or(0);
        model_object->input_scale     = input_meta[0].scale.value_or(1.0f);
        model_object->input_type_size = static_cast<int>(input_meta[0].type_size);

        model_object->inputs = (mtb_ml_tensor_desc_t *)calloc(
            model_object->input_count, sizeof(mtb_ml_tensor_desc_t));
        if (model_object->inputs == NULL)
        {
            return MTB_ML_RESULT_ALLOC_ERR;
        }

        size_t total_input_elems = 0;
        for (int i = 0; i < model_object->input_count; ++i)
        {
            fill_tensor_desc(&model_object->inputs[i], input_meta[i]);
            total_input_elems += input_meta[i].elements;
        }

        /* Input size must span all inputs so that the streaming test framework
         * knows how many elements to fill before invoking the model. */
        model_object->input_size = static_cast<int>(total_input_elems);
    }
    else
    {
        model_object->input           = NULL;
        model_object->input_size      = 0;
        model_object->input_zero_point = 0;
        model_object->input_scale     = 1.0f;
        model_object->input_type_size = 0;
    }

    if (model_object->output_count > 0)
    {
        const auto &output_meta = wrapper->output_meta;

        model_object->output            = output_meta[0].ptr;
        model_object->output_zero_point = output_meta[0].zero_point.value_or(0);
        model_object->output_scale      = output_meta[0].scale.value_or(1.0f);
        model_object->output_type_size  = static_cast<int>(output_meta[0].type_size);

        model_object->outputs = (mtb_ml_tensor_desc_t *)calloc(
            model_object->output_count, sizeof(mtb_ml_tensor_desc_t));
        if (model_object->outputs == NULL)
        {
            free(model_object->inputs);
            model_object->inputs = NULL;
            return MTB_ML_RESULT_ALLOC_ERR;
        }

        size_t total_output_elems = 0;
        for (int i = 0; i < model_object->output_count; ++i)
        {
            fill_tensor_desc(&model_object->outputs[i], output_meta[i]);
            total_output_elems += output_meta[i].elements;
        }
        model_object->output_size = static_cast<int>(total_output_elems);
    }
    else
    {
        model_object->output            = NULL;
        model_object->output_size       = 0;
        model_object->output_zero_point = 0;
        model_object->output_scale      = 1.0f;
        model_object->output_type_size  = 0;
    }

    return MTB_ML_RESULT_SUCCESS;
}

/******************************************************************************
* Function Name: mtb_ml_model_init
*******************************************************************************
* Summary:
*   Initialize NN model runtime object
*
* Parameters:
*   bin    : Pointer to model binary data structure
*   buffer : Pointer to model buffer configuration
*   object : Pointer to store created model object
*
* Return:
*   cy_rslt_t: MTB_ML_RESULT_SUCCESS on success, error code otherwise
*
******************************************************************************/
cy_rslt_t mtb_ml_model_init(
    const mtb_ml_model_bin_t *bin,
    const mtb_ml_model_buffer_t *buffer,
    mtb_ml_model_t **object)
{
    cy_rslt_t result;
    uint8_t *resolved_temp_allocation_pool = NULL;
    uint8_t *resolved_method_allocation_pool = NULL;
    size_t resolved_temp_allocation_pool_size = 0;
    size_t resolved_method_allocation_pool_size = 0;
    executorch_mtb::MTBExecuTorch *wrapper = NULL;

    /* Validate input parameters */
    if (bin == NULL || bin->model_bin == NULL || object == NULL) {
        return MTB_ML_RESULT_BAD_ARG;
    }

    /* Allocate model object */
    mtb_ml_model_t *model_object = (mtb_ml_model_t *)calloc(1, sizeof(mtb_ml_model_t));
    if (model_object == NULL) {
        return MTB_ML_RESULT_ALLOC_ERR;
    }

    /* Initialize model name */
    strncpy(model_object->name, bin->name, MTB_ML_MODEL_NAME_LEN - 1);
    model_object->name[MTB_ML_MODEL_NAME_LEN - 1] = '\0';

    model_object->model_size = bin->model_size;

    result = resolve_allocation_pool(
        (buffer != NULL) ? buffer->temp_allocation_pool : NULL,
        (buffer != NULL) ? buffer->temp_allocation_pool_size : 0U,
        (bin->temp_allocator_size > 0) ? (size_t)bin->temp_allocator_size : 0U,
        &model_object->temp_allocation_pool,
        &resolved_temp_allocation_pool,
        &resolved_temp_allocation_pool_size);
    if (result != MTB_ML_RESULT_SUCCESS)
    {
        mtb_ml_model_free_object(model_object);
        return result;
    }

    result = resolve_allocation_pool(
        (buffer != NULL) ? buffer->method_allocation_pool : NULL,
        (buffer != NULL) ? buffer->method_allocation_pool_size : 0U,
        (bin->method_allocator_size > 0) ? (size_t)bin->method_allocator_size : 0U,
        &model_object->method_allocation_pool,
        &resolved_method_allocation_pool,
        &resolved_method_allocation_pool_size);
    if (result != MTB_ML_RESULT_SUCCESS)
    {
        mtb_ml_model_free_object(model_object);
        return result;
    }

    /* Create ExecutorTorch wrapper */
    result = create_executorch_wrapper(
        bin->model_bin,
        bin->model_size,
        resolved_method_allocation_pool,
        resolved_method_allocation_pool_size,
        resolved_temp_allocation_pool,
        resolved_temp_allocation_pool_size,
        &wrapper);
    if (result != MTB_ML_RESULT_SUCCESS) {
        mtb_ml_model_free_object(model_object);
        return result;
    }

    model_object->tflm_obj = reinterpret_cast<void *>(wrapper);

    /* Populate input/output metadata */
    result = populate_io_metadata(model_object, wrapper);
    if (result != MTB_ML_RESULT_SUCCESS) {
        delete wrapper;
        mtb_ml_model_free_object(model_object);
        return result;
    }

    /* Add up the used arena size for the method allocator (static after initialization) and temp allocator size
       to get the total memory consumption. We use the resolved pool size in case the user has configured their own temp memory pool. */
    model_object->buffer_size = wrapper->get_used_arena_size() + resolved_temp_allocation_pool_size;
    model_object->model_time_steps = 0;
    model_object->recurrent_ts_size = 0;

    *object = model_object;
    return MTB_ML_RESULT_SUCCESS;
}

/******************************************************************************
* Function Name: mtb_ml_model_deinit
*******************************************************************************
* Summary:
*   Delete NN model runtime object and free all dynamically allocated memory
*
* Parameters:
*   object : Pointer to model object
*
* Return:
*   cy_rslt_t: MTB_ML_RESULT_SUCCESS on success, error code otherwise
*
******************************************************************************/
cy_rslt_t mtb_ml_model_deinit(mtb_ml_model_t *object)
{
    /* Sanity check of input parameters */
    if (object == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    /* Delete wrapper (destructor handles ExecutorTorch cleanup) */
    delete GET_ET_WRAPPER(object);

    mtb_ml_model_free_object(object);

    return MTB_ML_RESULT_SUCCESS;
}

/******************************************************************************
* Function Name: mtb_ml_model_inputs
*******************************************************************************
* Summary:
*   Set input data for a specific input tensor of the NN model
*
* Parameters:
*   object      : Pointer to model object
*   input       : Pointer to input data buffer for the specified tensor
*   input_index : Index of the input tensor (0 to input_count-1)
*
* Return:
*   cy_rslt_t: MTB_ML_RESULT_SUCCESS on success, error code otherwise
*
******************************************************************************/
cy_rslt_t mtb_ml_model_inputs(const mtb_ml_model_t *object, MTB_ML_DATA_T *input, int input_index)
{
    /* Sanity check of input parameters */
    VALIDATE_ET_WRAPPER(object);
    if (input == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    VALIDATE_TENSOR_INDEX(input_index, object->input_count);

    executorch_mtb::MTBExecuTorch *ET = GET_ET_WRAPPER(object);

    /* Set input data (internal copy) */
    ET->SetInput(input, input_index);

    return MTB_ML_RESULT_SUCCESS;
}

/******************************************************************************
* Function Name: mtb_ml_model_invoke
*******************************************************************************
* Summary:
*   Perform NN model inference (low-level function)
*
* Parameters:
*   object : Pointer to model object
*
* Return:
*   cy_rslt_t: MTB_ML_RESULT_SUCCESS on success, error code otherwise
*
******************************************************************************/
cy_rslt_t mtb_ml_model_invoke(mtb_ml_model_t *object)
{
    /* Sanity check of input parameters */
    VALIDATE_ET_WRAPPER(object);

    executorch_mtb::MTBExecuTorch *ET = GET_ET_WRAPPER(object);

    /* Model profiling - start */
    if (object->profiling & MTB_ML_PROFILE_ENABLE_MODEL)
    {
        mtb_ml_model_profile_get_tsc(&object->m_cpu_cycles);
#if (!defined(COMPONENT_RTOS) && defined(COMPONENT_U55))
        mtb_ml_npu_cycles = 0;
#endif
    }

#if defined(COMPONENT_U55)
    if(mtb_ml_get_cache_mgmt_type() == MTB_ML_ETHOSU_CACHE_MGMT_OUTER_LAYERS)
    {
        for (int i = 0; i < object->input_count; ++i)
        {
            SCB_InvalidateDCache_by_Addr((uint32_t *)object->inputs[i].ptr, object->inputs[i].bytes);
        }
    }
#endif

    /* Execute inference */
    executorch::runtime::Error ret = ET->RunSingleIteration();
#if defined(COMPONENT_U55)
    if(mtb_ml_get_cache_mgmt_type() == MTB_ML_ETHOSU_CACHE_MGMT_OUTER_LAYERS)
    {
        for (int i = 0; i < object->output_count; ++i)
        {
            SCB_InvalidateDCache_by_Addr((uint32_t *)object->outputs[i].ptr, object->outputs[i].bytes);
        }
    }
#endif
    /* Reset temp allocator (ExecutorTorch-specific requirement) */
    ET->reset_temp_allocator();

    /* Check inference result */
    if (ret != executorch::runtime::Error::Ok)
    {
        object->lib_error = static_cast<int>(ret);
        return MTB_ML_RESULT_INFERENCE_ERROR;
    }

    /* Model profiling - stop and update statistics */
    if (object->profiling & MTB_ML_PROFILE_ENABLE_MODEL)
    {
        uint64_t cycles = 0U;
        mtb_ml_model_profile_get_tsc(&cycles);
        uint64_t cpu_cycles_only = cycles - object->m_cpu_cycles;
#if (!defined(COMPONENT_RTOS) && defined(COMPONENT_U55))
        uint64_t norm_npu_cycles = (uint64_t)(((float)mtb_ml_npu_cycles) / mtb_ml_norm_clk_freq);
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
        cpu_cycles_only -= norm_npu_cycles;
#endif

        /* Update peak tracking */
        if (cpu_cycles_only > object->m_cpu_peak_cycles)
        {
            object->m_cpu_peak_cycles = cpu_cycles_only;
            object->m_cpu_peak_frame = object->m_sum_frames;
        }

        object->m_cpu_cycles = cpu_cycles_only;
        object->m_cpu_sum_cycles += cpu_cycles_only;
        object->m_sum_frames++;
    }
    if (object->profiling & MTB_ML_LOG_ENABLE_MODEL_LOG)
    {
        /*
         * Output logging.
         * This string must match ML_PROFILE_OUTPUT_STRING used by the streaming
         * host/device log parser (see mtb_ml_stream.c and coretools parser).
         */
        if (object->outputs != NULL && object->output_count > 0)
        {
            for (int out_i = 0; out_i < object->output_count; ++out_i)
            {
                const mtb_ml_tensor_desc_t* out = &object->outputs[out_i];
                MTB_ML_DATA_T* output_ptr = out->ptr;

                /* Prefix line with the magic substring so existing tooling can find it. */
                printf(" output:");
                if (object->output_count > 1)
                {
                    /* Optional index marker for multi-output models (keeps prefix intact). */
                    printf("[%d] ", out_i);
                }

                switch (out->type_size)
                {
                    case sizeof(float):
                        for (size_t j = 0; j < out->elements; ++j)
                        {
                            printf("%6.3f ", (float)(((float*)output_ptr)[j]));
                        }
                        break;
                    case sizeof(int16_t):
                        for (size_t j = 0; j < out->elements; ++j)
                        {
                            printf("%6.3f ", (float)(((int16_t*)output_ptr)[j]));
                        }
                        break;
                    case sizeof(int8_t):
                        for (size_t j = 0; j < out->elements; ++j)
                        {
                            printf("%6.3f ", (float)(((int8_t*)output_ptr)[j]));
                        }
                        break;
                    default:
                        return MTB_ML_RESULT_MISMATCH_DATA_TYPE;
                }

                printf("\r\n");
            }
        }
        else
        {
            /* Keep behavior predictable for models without tensor outputs. */
            printf(" output:\r\n");
        }
    }

    return MTB_ML_RESULT_SUCCESS;
}

/******************************************************************************
* Function Name: mtb_ml_model_run
*******************************************************************************
* Summary:
*   Perform NN model inference (convenience wrapper for single-input models)
*
* Parameters:
*   object : Pointer to model object
*   input  : Pointer to input data buffer
*
* Return:
*   cy_rslt_t: MTB_ML_RESULT_SUCCESS on success, error code otherwise
*
******************************************************************************/
cy_rslt_t mtb_ml_model_run(mtb_ml_model_t *object, MTB_ML_DATA_T *input)
{
    cy_rslt_t result;

    result = mtb_ml_model_inputs(object, input, 0);
    if (result != MTB_ML_RESULT_SUCCESS)
    {
        return result;
    }
    return mtb_ml_model_invoke(object);
}

/******************************************************************************
* Function Name: mtb_ml_model_get_input_size
*******************************************************************************
* Summary:
*   Get NN model input data size
*
* Parameters:
*   object : Pointer to model object
*
* Return:
*   int: Input data size, or 0 if invalid parameter
*
******************************************************************************/
int mtb_ml_model_get_input_size(const mtb_ml_model_t *object)
{
    return (object != NULL) ? object->input_size : 0;
}

/******************************************************************************
* Function Name: mtb_ml_model_get_output
*******************************************************************************
* Summary:
*   Get NN model output buffer and size
*
* Parameters:
*   object   : Pointer to model object
*   out_pptr : Pointer to store output buffer pointer
*   size_ptr : Pointer to store output size
*
* Return:
*   cy_rslt_t: MTB_ML_RESULT_SUCCESS on success, error code otherwise
*
******************************************************************************/
cy_rslt_t mtb_ml_model_get_output(const mtb_ml_model_t *object, MTB_ML_DATA_T **out_pptr, int* size_ptr)
{
    /* Call mtb_ml_model_get_output_tensor with output_index=0 for backward compatibility */
    return mtb_ml_model_get_output_tensor(object, out_pptr, size_ptr, 0);
}

/******************************************************************************
* Function Name: mtb_ml_model_get_output_tensor
*******************************************************************************
* Summary:
*   Get NN model output buffer and size for a specific output tensor
*
* Parameters:
*   object       : Pointer to model object
*   out_pptr     : Pointer to store output buffer pointer
*   size_ptr     : Pointer to store output size (number of elements)
*   output_index : Index of the output tensor (0 to output_count-1)
*
* Return:
*   cy_rslt_t: MTB_ML_RESULT_SUCCESS on success, error code otherwise
*
******************************************************************************/
cy_rslt_t mtb_ml_model_get_output_tensor(const mtb_ml_model_t *object, MTB_ML_DATA_T **out_pptr, int* size_ptr, int output_index)
{
    /* Sanity check of input parameters */
    if (object == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    VALIDATE_TENSOR_INDEX(output_index, object->output_count);

    if (out_pptr != NULL)
    {
        *out_pptr = object->outputs[output_index].ptr;
    }

    if (size_ptr != NULL)
    {
        *size_ptr = static_cast<int>(object->outputs[output_index].elements);
    }

    return MTB_ML_RESULT_SUCCESS;
}

/******************************************************************************
* Function Name: mtb_ml_model_load_output
*******************************************************************************
* Summary:
*   Load and concatenate all output tensors into a single buffer
*
* Parameters:
*   object      : Pointer to model object
*   output_pptr : Pointer to pre-allocated output buffer (must be at least
*                 object->output_concat_bytes in size)
*
* Return:
*   cy_rslt_t: MTB_ML_RESULT_SUCCESS on success, error code otherwise
*
******************************************************************************/
cy_rslt_t mtb_ml_model_load_output(const mtb_ml_model_t *object, MTB_ML_DATA_T **output_pptr)
{
    /* Sanity check of input parameters */
    if (object == NULL || output_pptr == NULL || *output_pptr == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }
    /* Iterate through all output tensors and concatenate them into the provided buffer */
    for (int i = 0; i < object->output_count; ++i)
    {
        const mtb_ml_tensor_desc_t *tensor = &object->outputs[i];
        uint8_t *dest = (uint8_t*)(*output_pptr) + tensor->concat_offset_bytes;
        memcpy(dest, tensor->ptr, tensor->bytes);
    }
    return MTB_ML_RESULT_SUCCESS;
}

/******************************************************************************
* Function Name: mtb_ml_model_get_input_detail
*******************************************************************************
* Summary:
*   Get NN model input details
*
* Parameters:
*   object      : Pointer to model object
*   index       : Input tensor index
*   in_pptr     : Pointer to store model's input data pointer
*   size_ptr    : Pointer to store size of model input data
*   dim_ptr     : Pointer to store model dimensions
*   dim_len_ptr : Pointer to store model dimension length
*   zero_ptr    : Pointer to store model input zero point
*   scale_ptr   : Pointer to store model input scale
*
* Return:
*   cy_rslt_t: MTB_ML_RESULT_SUCCESS on success, error code otherwise
*
******************************************************************************/
cy_rslt_t mtb_ml_model_get_input_detail(const mtb_ml_model_t *object, int index, MTB_ML_DATA_T **in_pptr, size_t* size_ptr,
                                        int** dim_ptr, int* dim_len_ptr, int* zero_ptr, float* scale_ptr)
{
    /* Sanity check of input parameters */
    if (object == NULL || size_ptr == NULL || in_pptr == NULL ||
        zero_ptr == NULL || scale_ptr == NULL ||
        dim_len_ptr == NULL || dim_ptr == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    VALIDATE_TENSOR_INDEX(index, object->input_count);

    const mtb_ml_tensor_desc_t *meta = &object->inputs[index];

    *size_ptr = meta->elements;
    *in_pptr = meta->ptr;
    *dim_len_ptr = meta->dim_len;
    *dim_ptr = meta->dims;
    *zero_ptr = meta->zero_point;
    *scale_ptr = meta->scale;

    return MTB_ML_RESULT_SUCCESS;
}

/******************************************************************************
* Function Name: mtb_ml_model_get_output_detail
*******************************************************************************
* Summary:
*   Get NN model output details
*
* Parameters:
*   object      : Pointer to model object
*   index       : Output tensor index
*   out_pptr    : Pointer to store model's output data pointer
*   size_ptr    : Pointer to store size of model output data
*   dim_ptr     : Pointer to store model dimensions
*   dim_len_ptr : Pointer to store model dimension length
*   zero_ptr    : Pointer to store model output zero point
*   scale_ptr   : Pointer to store model output scale
*
* Return:
*   cy_rslt_t: MTB_ML_RESULT_SUCCESS on success, error code otherwise
*
******************************************************************************/
cy_rslt_t mtb_ml_model_get_output_detail(const mtb_ml_model_t *object, int index, MTB_ML_DATA_T **out_pptr, size_t* size_ptr,
                                         int** dim_ptr, int* dim_len_ptr, int* zero_ptr, float* scale_ptr)
{
    /* Sanity check of input parameters */
    if (object == NULL || size_ptr == NULL || out_pptr == NULL ||
        zero_ptr == NULL || scale_ptr == NULL ||
        dim_len_ptr == NULL || dim_ptr == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    VALIDATE_TENSOR_INDEX(index, object->output_count);

    const mtb_ml_tensor_desc_t *meta = &object->outputs[index];

    *size_ptr = meta->elements;
    *out_pptr = meta->ptr;
    *dim_len_ptr = meta->dim_len;
    *dim_ptr = meta->dims;
    *zero_ptr = meta->zero_point;
    *scale_ptr = meta->scale;

    return MTB_ML_RESULT_SUCCESS;
}

/******************************************************************************
* Function Name: mtb_ml_model_rnn_reset_all_parameters
*******************************************************************************
* Summary:
*   Reset model parameters (for RNN models)
*
* Parameters:
*   object : Pointer to model object
*
* Return:
*   cy_rslt_t: MTB_ML_RESULT_SUCCESS on success, error code otherwise
*
******************************************************************************/
cy_rslt_t mtb_ml_model_rnn_reset_all_parameters(mtb_ml_model_t *object)
{
    /* Sanity check of model object parameter */
    VALIDATE_ET_WRAPPER(object);

    executorch_mtb::MTBExecuTorch *ET = GET_ET_WRAPPER(object);
    executorch::runtime::Error ret = ET->reset_all_variables();

    if (ret != executorch::runtime::Error::Ok)
    {
        return MTB_ML_RESULT_INFERENCE_ERROR;
    }

    return MTB_ML_RESULT_SUCCESS;
}

/******************************************************************************
* Function Name: mtb_ml_model_profile_config
*******************************************************************************
* Summary:
*   Update MTB ML inference profiling setting
*
* Parameters:
*   object : Pointer to model object
*   config : Profiling configuration
*
* Return:
*   cy_rslt_t: MTB_ML_RESULT_SUCCESS on success, error code otherwise
*
******************************************************************************/
cy_rslt_t mtb_ml_model_profile_config(mtb_ml_model_t *object, mtb_ml_profile_config_t config)
{
    /* Sanity check of input parameters */
    if (object == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

#if defined(COMPONENT_U55)
    ETHOSU_PMU_Enable(mtb_ml_ethosu_driver_handle);
#endif
    object->profiling = config;
    if (object->profiling != MTB_ML_PROFILE_DISABLE)
    {
        object->m_sum_frames = 0;
        object->m_cpu_sum_cycles = 0;
        object->m_cpu_peak_frame = 0;
        object->m_cpu_peak_cycles = 0;
    }
#if defined(COMPONENT_U55)
    object->m_npu_sum_cycles = 0;
    object->m_npu_peak_frame = 0;
    object->m_npu_peak_cycles = 0;
#endif

    return MTB_ML_RESULT_SUCCESS;
}

/******************************************************************************
* Function Name: mtb_ml_model_profile_log
*******************************************************************************
* Summary:
*   Generate MTB ML profiling log
*
* Parameters:
*   object : Pointer to model object
*
* Return:
*   cy_rslt_t: MTB_ML_RESULT_SUCCESS on success, error code otherwise
*
******************************************************************************/
cy_rslt_t mtb_ml_model_profile_log(mtb_ml_model_t *object)
{
    /* Sanity check of input parameters */
    if (object == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    if (object->profiling & MTB_ML_PROFILE_ENABLE_MODEL)
    {
        printf("PROFILE_INFO, MTB ML model profile, avg_cpu_cyc=%-10.2f, peak_cpu_cyc=%.0f, peak_cpu_frame=%" PRIu32 ", cpu_freq_Mhz=%" PRIu32 "\r\n",
               (float)object->m_cpu_sum_cycles / object->m_sum_frames,
               (float)object->m_cpu_peak_cycles,
               object->m_cpu_peak_frame,
               mtb_ml_cpu_clk_freq / 1000000);
#if defined(COMPONENT_U55)
        printf("PROFILE_INFO, MTB ML model profile, avg_npu_cyc=%-10.2f, peak_npu_cyc=%.0f, peak_npu_frame=%" PRIu32 ", npu_freq_Mhz=%" PRIu32 "\r\n",
               (float)object->m_npu_sum_cycles / object->m_sum_frames,
               (float)object->m_npu_peak_cycles,
               object->m_npu_peak_frame,
               mtb_ml_npu_clk_freq / 1000000);
#endif
    }

    return MTB_ML_RESULT_SUCCESS;
}

/******************************************************************************
* Function Name: mtb_ml_model_get_inference_object
*******************************************************************************
* Summary:
*   Get MTB ML inference runtime object
*
* Parameters:
*   object : Pointer to model object
*
* Return:
*   void*: Pointer to inference object, or NULL if invalid parameter
*
******************************************************************************/
void* mtb_ml_model_get_inference_object(const mtb_ml_model_t *object)
{
    return (object != NULL) ? object->tflm_obj : NULL;
}

}  /* extern "C" */
