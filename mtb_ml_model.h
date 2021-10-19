/***************************************************************************//**
* \file mtb_ml_model.h
*
* \brief
* This is the header file of ModusToolbox ML middleware NN model module.
*
*******************************************************************************
* \copyright
* Copyright 2021, Cypress Semiconductor Corporation (an Infineon company).
* All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
******************************************************************************/


#if !defined(__MTB_ML_MODEL_H__)
#define __MTB_ML_MODEL_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "cy_ml_inference.h"
#include "mtb_ml_common.h"


/******************************************************************************
 * Macros
 *****************************************************************************/
#define MEM_FLAG_SHIFT_PERSISTENT       (0)
#define MEM_FLAG_SHIFT_SCRATCH          (1)

#define MTB_ML_MEM_DYNAMIC_PERSISTENT   (1 << MEM_FLAG_SHIFT_PERSISTENT)
#define MTB_ML_MEM_DYNAMIC_SCRATCH      (1 << MEM_FLAG_SHIFT_SCRATCH)
/******************************************************************************
 * Typedefs
 *****************************************************************************/

/******************************************************************************
* Public definitions
******************************************************************************/

/******************************************************************************
* Structures
******************************************************************************/
/**
 * ML model working buffer structure
 */
typedef struct
{
    void *persistent;                   /**< the pointer of ML model persistent buffer */
    void *scratch;                      /**< the pointer of ML model scratch buffer */
} mtb_ml_model_buffer_t;

/**
 * ML model structure
 */
typedef struct
{
    char name[64];                      /**< the name of ML model */
    const unsigned char *prms_bin;      /**< the pointer of ML model parameters data */
    const unsigned int   prms_size;     /**< ML model parameters data size in bytes */
    const unsigned char *wts_bin;       /**< the pointer of ML model weight and bias data */
    const unsigned int   wts_size;      /**< ML model weights data size in bytes */
} mtb_ml_model_bin_t;

/**
 * ML model runtime object structure
 */
typedef struct
{
    void *p_mem;                        /**< pointer of persistent memory */
    void *s_mem;                        /**< pointer of scratch memory */
    void *inference_obj;                /**< pointer of ML inference runtime object */
    int ml_error;                       /**< error code from ML inference library */
    int input_q_n;                      /**< Q-format(Qm.n) fraction bit for input data */
    int output_q_n;                     /**< Q-format(Qm.n) fraction bit for output data */
    int flags;                          /**< flags for memory allocation */
    cy_stc_ml_model_info_t model_info;  /**< Interpreted model information */
} mtb_ml_model_t;

/******************************************************************************
* Function prototype
******************************************************************************/
/**
 * \addtogroup Model_API
 * @{
 */

/**
 * \brief : Allocate and initialize NN model runtime object based on model data
 *
 * \param[in]   bin      : Pointer of model binary data.
 * \param[in]   buffer   : Pointer of buffer data structure for statically allocated persistent and scratch buffer.
 *                         This is optional, if no passed-in buffer, the API will allocate memory as persistent and
 *                         scratch buffer.
 * \param[out] object    : Pointer of model object.
 *
 * \return               : MTB_ML_RESULT_SUCCESS - success
 *                       : MTB_ML_RESULT_BAD_ARG - if input parameter is invalid.
 *                       : MTB_ML_RESULT_ALLOC_ERR - if memory allocation failure.
 *                       : MTB_ML_RESULT_BAD_MODEL - if model parsing or initialization error.
 */
cy_rslt_t mtb_ml_model_init(mtb_ml_model_bin_t *bin, mtb_ml_model_buffer_t *buffer, mtb_ml_model_t **object);


/**
 * \brief : Delete NN model runtime object and free all dynamically allocated memory
 *
 * \param[in] object     : Pointer of model object.
 *
 * \return               : MTB_ML_RESULT_SUCCESS - success
 *                       : MTB_ML_RESULT_BAD_ARG - if input parameter is invalid.
 */
cy_rslt_t mtb_ml_model_deinit(mtb_ml_model_t *object);

/**
 * \brief : Perform NN model inference
 *
 * \param[in] object     : Pointer of model object.
 * \param[in] input      : Pointer of input data buffer
 * \param[in] output     : Pointer of output data array
 *
 * \return               : MTB_ML_RESULT_SUCCESS - success
 *                       : MTB_ML_RESULT_BAD_ARG - if input parameter is invalid.
 *                       : MTB_ML_RESULT_INFERENCE_ERROR - if inference failure
 */
cy_rslt_t mtb_ml_model_run(mtb_ml_model_t *object, MTB_ML_DATA_T *input, MTB_ML_DATA_T *output);


/**
 * \brief : Get NN model input data size
 *
 * \param[in] object     : Pointer of model object.
 *
 * \return               : Input data size
 *                       : 0 - if input parameter is invalid.
 */
int mtb_ml_model_get_input_size(mtb_ml_model_t *object);

/**
 * \brief : Get NN model output data size
 *
 * \param[in] object     : Pointer of model object.
 *
 * \return               : Output data size
 *                       : 0 - if input parameter is invalid.
 */
int mtb_ml_model_get_output_size(mtb_ml_model_t *object);

/**
 * \brief : Get the number of frame in recurrent model time series
 *
 * \param[in] object     : Pointer of model object.
 *
 * \return               : >0 - GNU model feature window size
 *                       : 0 - otherwise
 */
int mtb_ml_model_get_recurrent_time_series_frames(mtb_ml_model_t *object);

/**
 * \brief : Update recurrent NN reset state
 *
 * \param[in] object     : Pointer of model object.
 * \param[in] status     : Enable (1) or disable (0) Recurrent NN reset
 * \param[in] win_size   : Recurrent NN reset window size
 *
 * \return               : MTB_ML_RESULT_SUCCESS - success
 *                       : MTB_ML_RESULT_BAD_ARG - if input parameter is invalid.
 */
cy_rslt_t mtb_ml_model_rnn_state_control(mtb_ml_model_t *object, int status, int win_size);

/**
 * \brief : Get MTB ML inference runtime object
 *
 * \param[in] object     : Pointer of model object.
 *
 * \return               : Pointer of inference object
 *                       : NULL - if input parameter is invalid.
 */
void* mtb_ml_model_get_inference_object(mtb_ml_model_t *object);

/**
 * \brief : Update MTB ML inference profiling setting
 *
 * \param[in] object    : Pointer of model object's pointer.
 * \param[in] config     : Profiling setting
 *
 * \return               : MTB_ML_RESULT_SUCCESS - success
 *                       : MTB_ML_RESULT_BAD_ARG - if input parameter is invalid.
 */
cy_rslt_t mtb_ml_model_profile_config(mtb_ml_model_t *object, mtb_ml_profile_config_t config);

/**
 * \brief : Generate MTB ML profiling log
 *
 * \param[in] object     : Pointer of model object.
 *
 * \return               : MTB_ML_RESULT_SUCCESS - success
 *                       : MTB_ML_RESULT_BAD_ARG - if input parameter is invalid.
 */
cy_rslt_t mtb_ml_model_profile_log(mtb_ml_model_t *object);

#if !COMPONENT_ML_FLOAT32
/**
 * \brief : Set NN model input data Q-format fraction bits
 *
 * \param[in] object     : Pointer of model object.
 * \param[in] bits       : Q-format fraction bits
 *
 * \return               : MTB_ML_RESULT_SUCCESS - success
 *                       : MTB_ML_RESULT_BAD_ARG - if input parameter is invalid.
 */
cy_rslt_t mtb_ml_model_set_input_q_fraction_bits(mtb_ml_model_t *object, uint8_t bits);

/**
 * \brief : Get NN model output data Q-format fraction bits
 *
 * \param[in] object     : Pointer of model object.
 *
 * \return               : Q-format fraction bit
 *                       : 0 - if input parameter is invalid.
 */
uint8_t mtb_ml_model_get_output_q_fraction_bits(mtb_ml_model_t *object);
#endif /* !COMPONENT_ML_FLOAT32 */

/**
 * \brief : Print detailed model info
 *
 * \param[in] obj        : Pointer of model object.
 * \param[in] bin        : Pointer of model binary data.
 *
 * \return               : MTB_ML_RESULT_SUCCESS - success
 *                       : MTB_ML_RESULT_BAD_ARG - if input paramter is invalid.
 */
cy_rslt_t mtb_ml_model_info(mtb_ml_model_t *obj, mtb_ml_model_bin_t *bin);

/**
 * @} end of Model_API group
 */

#if defined(__cplusplus)
}
#endif

#endif /* __MTB_ML_MODEL_H__ */
