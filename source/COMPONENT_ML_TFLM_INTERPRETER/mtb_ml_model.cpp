/******************************************************************************
* File Name: mtb_ml_model_tflm.cpp
*
* Description: This file contains API calls to initialize and invoke Tflite-Micro
*              inference for model generated from CY ML software.
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
#include <stdlib.h>
#include <inttypes.h>
#include "mtb_ml_model.h"

#include <climits>

#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_op_resolver.h"
#include "tensorflow/lite/micro/recording_micro_interpreter.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_utils.h"

extern "C" {

/* LCOV_EXCL_START (Excluded from the code coverage, until the STOP marker) */
int __attribute__((weak)) mtb_ml_model_profile_get_tsc(uint32_t *val)
{
    return 0;
}
/* LCOV_EXCL_STOP */

}

namespace tflite {

static tflite::AllOpsResolver resolver;

template <typename inputT>
class MTBTFLiteMicro {
 public:
  // The lifetimes of model, op_resolver, tensor_arena must exceed
  // that of the created MicroBenchmarkRunner object.
  MTBTFLiteMicro(const uint8_t* model,
                       uint8_t* tensor_arena, int tensor_arena_size,
                       const tflite::MicroOpResolver& op_resolver)
      : interpreter_(GetModel(model), op_resolver, tensor_arena,
                     tensor_arena_size, GetMicroErrorReporter(), nullptr, nullptr) {
      allocate_status_ = interpreter_.AllocateTensors();
  }

  TfLiteStatus RunSingleIteration() {
    // Run the model on this input and return the status.
    return interpreter_.Invoke();
  }

  TfLiteTensor* Input(int index = 0)  { return interpreter_.input(index); }
  TfLiteTensor* Output(int index = 0) { return interpreter_.output(index); }

  TfLiteStatus AllocationStatus() { return allocate_status_; }

  inputT* input_ptr(int index = 0) { return GetTensorData<inputT>(Input(index)); }
  size_t input_size(int index = 0) { return interpreter_.input(index)->bytes; }
  size_t input_elements(int index = 0) { return tflite::ElementCount(*(interpreter_.input(index)->dims)); }
  int    input_dims_len(int index=0) {return interpreter_.input(index)->dims->size; }
  int *  input_dims( int index=0) { return &interpreter_.input(index)->dims->data[0]; }
  int    output_zero_point( int index=0) { return interpreter_.output(index)->params.zero_point; }
  float  output_scale( int index=0) { return interpreter_.output(index)->params.scale; }

  inputT* output_ptr(int index = 0) { return GetTensorData<inputT>(Output(index)); }
  size_t output_size(int index = 0) { return interpreter_.output(index)->bytes; }
  size_t output_elements(int index = 0) { return  tflite::ElementCount(*(interpreter_.output(index)->dims));}
  int    output_dims_len(int index=0) {return interpreter_.output(index)->dims->size; }
  int *  output_dims( int index=0) { return &interpreter_.output(index)->dims->data[0]; }
  size_t get_used_arena_size() { return (interpreter_.arena_used_bytes() + 1023); }

  void SetInput(const inputT* custom_input, int input_index = 0) {
    TfLiteTensor* input = interpreter_.input(input_index);
    inputT* input_buffer = tflite::GetTensorData<inputT>(input);
    int input_length = input->bytes / sizeof(inputT);
    for (int i = 0; i < input_length; i++) {
      input_buffer[i] = custom_input[i];
    }
  }

  void PrintAllocations() const {
    interpreter_.GetMicroAllocator().PrintAllocations();
  }

 private:
  tflite::RecordingMicroInterpreter interpreter_;
  TfLiteStatus allocate_status_;
};

using MTB_TFLM_flt = MTBTFLiteMicro<float>;
using MTB_TFLM_int8 = MTBTFLiteMicro<int8_t>;

} // namespace tflite

#ifdef COMPONENT_ML_INT8x8
#define MTB_TFLM_Class MTB_TFLM_int8
#else
#define MTB_TFLM_Class MTB_TFLM_flt
#endif

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


cy_rslt_t mtb_ml_model_init(const mtb_ml_model_bin_t *bin, const mtb_ml_model_buffer_t *buffer, mtb_ml_model_t **object)
{
    mtb_ml_model_t *model_object = NULL;
    uint8_t * arena_buffer = NULL;
    int arena_size;
    tflite::MTB_TFLM_Class * TFLMClass;
    int ret = MTB_ML_RESULT_SUCCESS;

    /* Sanity check of input parameters */
    if (bin == NULL || bin->model_bin == NULL || object == NULL)
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

    /* Get the arena size specified in model data file */
    arena_size = bin->arena_size;
    if (buffer != NULL)
    {
        if (buffer->tensor_arena_size != 0)
        {
            /* over-write with application provided value */
            arena_size = buffer->tensor_arena_size;
        }
        arena_buffer = buffer->tensor_arena;
    }

    /* Get model and buffer size */
    model_object->model_size = bin->model_size;
    model_object->buffer_size = arena_size;

    /* Allocate tensor arena if it is not specified */
    if (arena_buffer == NULL)
    {
        model_object->arena_buffer = (uint8_t *) malloc(arena_size);
        if (model_object->arena_buffer == NULL)
        {
            ret = MTB_ML_RESULT_ALLOC_ERR;
            goto ret_err;
        }
        arena_buffer = model_object->arena_buffer;
    }

    TFLMClass = new tflite::MTB_TFLM_Class(bin->model_bin, arena_buffer, arena_size, tflite::resolver);
    model_object->tflm_obj = reinterpret_cast<void *>(TFLMClass);
    if( model_object->tflm_obj == NULL)
    {
        ret = MTB_ML_RESULT_BAD_MODEL;
        goto ret_err;
    }

    /* Check Tensor allocation failure */
    if (TFLMClass->AllocationStatus() != kTfLiteOk)
    {
        ret = MTB_ML_RESULT_ALLOC_ERR;
        goto ret_err;
    }

    model_object->input_size = TFLMClass->input_elements();
    model_object->output_size = TFLMClass->output_elements();
    model_object->output = TFLMClass->output_ptr();
    model_object->buffer_size = TFLMClass->get_used_arena_size();
    model_object->output_zero_point = TFLMClass->output_zero_point();
    model_object->output_scale = TFLMClass->output_scale();

    *object = model_object;
    return ret;
ret_err:
    free(model_object->arena_buffer);
    free(model_object);
    return ret;
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
    delete reinterpret_cast<tflite::MTB_TFLM_Class *>(object->tflm_obj);
    free(object->arena_buffer);
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

    tflite::MTB_TFLM_Class *Tflm = reinterpret_cast<tflite::MTB_TFLM_Class *>(object->tflm_obj);

    /* Set input data */
    Tflm->SetInput(input);

    /* Model profiling */
    if (object->profiling & MTB_ML_PROFILE_ENABLE_MODEL)
    {
        mtb_ml_model_profile_get_tsc(&object->m_cycles);
    }
    ret = Tflm->RunSingleIteration();
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
