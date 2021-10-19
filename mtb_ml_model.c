/***************************************************************************//**
* \file mtb_ml_model.c
*
* \brief
* The file contains application programming interface to the ModusToolbox ML
* middleware model module
*
*******************************************************************************
* \copyright
* Copyright 2021, Cypress Semiconductor Corporation (an Infineon company).
* All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "mtb_ml_common.h"
#include "mtb_ml_model.h"
#include "cy_device_headers.h" /* for memory region definitions */

#define MEM_RANGE(x,y,z) y <= x && x < (y + z)

#ifndef  FLOAT_ABI
#define FLOAT_ABI Unknown
#endif //#ifndef  FLOAT_ABI

#ifndef  BUILD_HOST
#define BUILD_HOST Unknown
#endif //#ifndef  BUILD_HOST

#ifndef  CONFIG
#define CONFIG Unknown
#endif //#ifndef  CONFIG

/**
 * Get NN model memory type
 */
static char * memory_area(uint32_t addr)
{
    if (MEM_RANGE(addr, CY_SRAM_BASE, CY_SRAM_SIZE))
    {
        return "RAM";
    }
    else if (MEM_RANGE(addr, CY_FLASH_BASE, CY_FLASH_SIZE))
    {
        return "FLASH";
    }
    else if (MEM_RANGE(addr, CY_EM_EEPROM_BASE, CY_EM_EEPROM_SIZE))
    {
        return "EEPROM";
    }
    return "unknown";
}

/**
 * Print detailed model info
 */
cy_rslt_t mtb_ml_model_info(mtb_ml_model_t *obj, mtb_ml_model_bin_t *bin)
{
    cy_stc_ml_model_info_t *model_info;
    uint32_t model_address;
    char *memory_type;

    if ((obj == NULL) || (bin == NULL)) {
        return MTB_ML_RESULT_BAD_ARG;
    }

    model_info = &obj->model_info;
    model_address = (uint32_t)bin->wts_bin;
    memory_type = memory_area(model_address);

    printf("\r\n***************************************************\r\n");
    printf("MTB ML inference\r\n");
    printf("\tBuild host       \t:\t%s\r\n", EXPAND_AND_STRINGIFY(BUILD_HOST));
#if defined(__ARMCC_VERSION)
    printf("\tCompiler         \t:\t%s%u\r\n", "ARM CLANG ", __ARMCC_VERSION);
#elif defined(__ICCARM__)
    printf("\tCompiler         \t:\t%s%u\r\n", "IAR ", __VER__);
#elif defined(__GNUC__)
    printf("\tCompiler         \t:\t%s%u.%u\r\n", "GCC ", __GNUC__, __GNUC_MINOR__ );
#else
    printf("\tCompiler         \t:\t%s\r\n", "Unknown ");
#endif
    printf("\tConfig           \t:\t%s\r\n", EXPAND_AND_STRINGIFY(CONFIG));
    printf("\tFloat ABI        \t:\t%s\r\n", EXPAND_AND_STRINGIFY(FLOAT_ABI));
    printf("\tModel name       \t:\t%s\r\n", bin->name);
    printf("\tModel memory:    \t:\t%s (0x%.8"PRIx32")\r\n", memory_type, model_address);
    printf("\tLayers           \t:\t%d\r\n", model_info->num_of_layers);
    printf("\tInput_nodes      \t:\t%d\r\n", model_info->input_size);
    printf("\tOutput_nodes     \t:\t%d\r\n", model_info->output_size);
    printf("\tML Lib Version   \t:\t%d\r\n", model_info->libml_version);
    printf("\tML Lib Input Type\t:\t%s\r\n", model_info->libml_input_type == CY_ML_DATA_FLOAT ? "floating-point" :
                                             (model_info->libml_input_type == CY_ML_DATA_INT16 ? "int16" : "int8"));
    printf("\tML Lib Weight Type\t:\t%s\r\n", model_info->libml_weight_type == CY_ML_DATA_FLOAT ? "floating-point" :
                                             (model_info->libml_weight_type == CY_ML_DATA_INT16 ? "int16" : "int8"));
    printf("\tML Coretool Version\t:\t0x%"PRIx32"", model_info->ml_coretool_version);
    printf("\r\n\r\n");
    printf("Memory usage: \r\n");
    printf("\tScratch memory(%s)    \t: %8.2f kB\r\n", obj->flags & MTB_ML_MEM_DYNAMIC_PERSISTENT ?
                                                       "dynamic" : "static",
                                                       model_info->scratch_mem  / 1024.0);
    printf("\tPersistent memory(%s) \t: %8.2f kB\r\n", obj->flags & MTB_ML_MEM_DYNAMIC_SCRATCH ?
                                                       "dynamic" : "static",
                                                       model_info->persistent_mem  / 1024.0);
    printf("\tWeights & biases  \t: %8.2f kB", (bin->wts_size + bin->prms_size)  / 1024.0);
    printf("\r\n***************************************************\r\n");

    return MTB_ML_RESULT_SUCCESS;
}

/**
 *  Allocate and initialize NN model runtime object
 */
cy_rslt_t mtb_ml_model_init(mtb_ml_model_bin_t *bin, mtb_ml_model_buffer_t *buffer, mtb_ml_model_t **object)
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

    /* Get the NN model information by parsing the model parameters data */
    status = Cy_ML_Model_Parse((char *) bin->prms_bin, &model_object->model_info);
    if (status != CY_ML_SUCCESS)
    {
        ret = MTB_ML_RESULT_BAD_MODEL;
        goto RET_ERR;
    }

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
    free(object);
    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Perform NN model inference
 */
cy_rslt_t mtb_ml_model_run(mtb_ml_model_t *object, MTB_ML_DATA_T *input, MTB_ML_DATA_T *output)
{
    int ret;
    /* Sanity check of input parameters */
    if (object == NULL || input == NULL || output == NULL)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    /* Initialize output q-factor - acting as input and output q-factor */
    object->output_q_n = object->input_q_n;

    ret = Cy_ML_Model_Inference(object->inference_obj, input, output,
#if COMPONENT_ML_FLOAT32
                                NULL
#else
                                &object->output_q_n
#endif
                               );

    if (ret != CY_ML_SUCCESS)
    {
        object->ml_error = ret;
        return MTB_ML_RESULT_INFERENCE_ERROR;
    }

    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Get NN model input data size
 */
int mtb_ml_model_get_input_size(mtb_ml_model_t *object)
{
    if (object == NULL)
    {
        return 0;
    }
    else
    {
        return object->model_info.input_size;
    }
}

/**
 * Get NN model output data size
 */
int mtb_ml_model_get_output_size(mtb_ml_model_t *object)
{
    if (object == NULL)
    {
        return 0;
    }
    else
    {
        return object->model_info.output_size;
    }
}

/**
 * Get the number of frame in recurrent model time series
 */
int mtb_ml_model_get_recurrent_time_series_frames(mtb_ml_model_t *object)
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
        object->ml_error = result;
        return MTB_ML_RESULT_BAD_ARG;
    }
    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Get MTB ML inference runtime object
 */
void* mtb_ml_model_get_inference_object(mtb_ml_model_t *object)
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

    Cy_ML_Profile_Control(object->inference_obj, config);
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

#if !COMPONENT_ML_FLOAT32
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
uint8_t mtb_ml_model_get_output_q_fraction_bits(mtb_ml_model_t *object)
{
    /* Sanity check of input parameters */
    if (object == NULL)
    {
        return 0;
    }

    return object->output_q_n;
}

#endif /* !COMPONENT_ML_FLOAT32 */
