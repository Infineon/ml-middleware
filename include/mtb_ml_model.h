/***************************************************************************//**
* \file mtb_ml_model.h
*
* \brief
* This is the header file of ModusToolbox ML middleware NN model module.
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
#if !defined(__MTB_ML_MODEL_H__)
#define __MTB_ML_MODEL_H__

#if defined(__cplusplus)
extern "C" {
#endif
#include "mtb_ml_common.h"
#include "mtb_ml_model_defs.h"

/******************************************************************************
 * Macros
 *****************************************************************************/
#define MEM_FLAG_SHIFT_PERSISTENT       (0)
#define MEM_FLAG_SHIFT_SCRATCH          (1)

#define MTB_ML_MEM_DYNAMIC_PERSISTENT   (1 << MEM_FLAG_SHIFT_PERSISTENT)
#define MTB_ML_MEM_DYNAMIC_SCRATCH      (1 << MEM_FLAG_SHIFT_SCRATCH)

#define MTB_ML_MODEL_NAME_LEN           64
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
#if defined(COMPONENT_ML_IFX)
/** @name COMPONENT_ML_IFX
 *  Buffer parameters for IFX inference engine.
 */
///@{
/** Model Buffer parameters for IFX inference engine:  */
    void *persistent;                   /**< the pointer of ML model persistent buffer */
    void *scratch;                      /**< the pointer of ML model scratch buffer */
///@}
#endif
#if defined(COMPONENT_ML_TFLM_INTERPRETER) || defined(COMPONENT_ML_TFLM_INTERPRETER_LESS)
/** @name COMPONENT_ML_TFLM
 *  Buffer pareamters for TFLM.
 */
///@{
/** Model buffer parameters for TfLite-Micro inference engine */
    uint8_t* tensor_arena;              /**< the pointer of tensor arena buffer provided by application */
    size_t tensor_arena_size;           /**< the size of ML tensor arena buffer provided by application */
///@}
#endif
} mtb_ml_model_buffer_t;

/**
 * ML model structure
 */
typedef struct
{
/** @name
 *  Model name
 */
/**@{*/
    char name[MTB_ML_MODEL_NAME_LEN];   /**< the name of ML model */
/**@}*/
#if defined(COMPONENT_ML_IFX)
/** @name COMPONENT_ML_IFX
 *  Model pareamters for IFX inference engine.
 */
///@{
    const unsigned char *prms_bin;      /**< the pointer of ML model parameters data */
    const unsigned int   prms_size;     /**< ML model parameters data size in bytes */
    const unsigned char *wts_bin;       /**< the pointer of ML model weight and bias data */
    const unsigned int   wts_size;      /**< ML model weights data size in bytes */
///@}
#endif
#if defined(COMPONENT_ML_TFLM_INTERPRETER)
/** @name COMPONENT_ML_TFLM_INTERPRETER
 *  Model pareamters for TFLM with interpreter
 */
///@{
    const uint8_t *      model_bin;     /**< the pointer of Tflite model */
    const unsigned int   model_size;    /**< the size of Tflite model */
    const int            arena_size;    /**< the size of arena buffer for Tflite model */
///@}
#endif
#if defined(COMPONENT_ML_TFLM_INTERPRETER_LESS)
/** @name COMPONENT_ML_TFLM_INTERPRETER_LESS
 *  Model pareamters for TFLM without interpreter
 */
///@{
    tflm_rmf_apis_t      rmf_bin;     /**< the data structure of Tflite APIs */
///@}
#endif
} mtb_ml_model_bin_t;

/**
 * ML model runtime object structure
 */
typedef struct
{
/** @name
 *  Model runtime object common fields
 */
/**@{*/
    char name[MTB_ML_MODEL_NAME_LEN];   /**< the name of ML model */
    int model_size;                     /**< the size of ML model */
    int buffer_size;                    /**<the size of ML model working buffer */
    int input_size;                     /**< array size of input data */
    int output_size;                    /**< array size of output data */
    int lib_error;                      /**< error code from ML inference library */
    MTB_ML_DATA_T *output;              /**< pointer of ML inference output buffer */
/**@}*/
#if defined(COMPONENT_ML_IFX)
/** @name COMPONENT_ML_IFX
 *  Model runtime object fields for IFX inference engine
 */
/**@{*/
    void *p_mem;                        /**< pointer of persistent memory */
    void *s_mem;                        /**< pointer of scratch memory */
    void *inference_obj;                /**< pointer of ML inference runtime object */
    int input_q_n;                      /**< Q-format(Qm.n) fraction bit for input data */
    int output_q_n;                     /**< Q-format(Qm.n) fraction bit for output data */
    int flags;                          /**< flags for memory allocation */
    cy_stc_ml_model_info_t model_info;  /**< Interpreted model information */
/**@}*/
#endif
#if defined(COMPONENT_ML_TFLM_INTERPRETER) || defined(COMPONENT_ML_TFLM_INTERPRETER_LESS)
/** @name COMPONENT_ML_TFLM
 *  Model runtime object fields for TFLM
 */
/**@{*/
    void *tflm_obj;                     /**< pointer of Tflite-micro runtime object */
    int output_zero_point;              /**< zero point of output data */
    float output_scale;                 /**< scale of output data */
    mtb_ml_profile_config_t profiling;  /**< flags of profiling */
    uint32_t m_cycles;                  /**< profiling cycles */
    uint32_t m_sum_frames;              /**< profiling frames */
    uint64_t m_sum_cycles;              /**< profiling total cycles */
    uint32_t m_peak_frame;              /**< profiling peak frame */
    uint32_t m_peak_cycles;             /**< profiling peak cycles */
/**@}*/
#endif
#if defined(COMPONENT_ML_TFLM_INTERPRETER)
/** @name COMPONENT_ML_TFLM_INTERPRETER
 *  Model runtime object fields for TFLM with interpreter
 */
/**@{*/
    uint8_t *arena_buffer;              /**< pointer of allocated tensor arena buffer */
#endif
#if defined(COMPONENT_ML_TFLM_INTERPRETER_LESS)
/** @name COMPONENT_ML_TFLM_INTERPRETER_LESS
 *  Model runtime object fields for TFLM without interpreter
 */
/**@{*/
    tflm_rmf_apis_t rmf_apis;           /**< data structure of Tflite-micro APIs */
/**@}*/
#endif
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
cy_rslt_t mtb_ml_model_init(const mtb_ml_model_bin_t *bin, const mtb_ml_model_buffer_t *buffer, mtb_ml_model_t **object);


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
 *
 * \return               : MTB_ML_RESULT_SUCCESS - success
 *                       : MTB_ML_RESULT_BAD_ARG - if input parameter is invalid.
 *                       : MTB_ML_RESULT_INFERENCE_ERROR - if inference failure
 */
cy_rslt_t mtb_ml_model_run(mtb_ml_model_t *object, MTB_ML_DATA_T *input);


/**
 * \brief : Get NN model input data size
 *
 * \param[in] object     : Pointer of model object.
 *
 * \return               : Input data size
 *                       : 0 - if input parameter is invalid.
 */
int mtb_ml_model_get_input_size(const mtb_ml_model_t *object);

/**
 * \brief : Get NN model output buffer and size
 *
 * \param[in] object     : Pointer of model object.
 * \param[out] out_pptr  : Pointer of output buffer pointer
 * \param[out] size_ptr  : Pointer of output size
 *
 * \return               : Output data size
 *                       : 0 - if input parameter is invalid.
 */
cy_rslt_t mtb_ml_model_get_output(const mtb_ml_model_t *object, MTB_ML_DATA_T **out_pptr, int* size_ptr);

/**
 * \brief : Get the number of frame in recurrent model time series
 *
 * \param[in] object     : Pointer of model object.
 *
 * \return               : >0 - GNU model feature window size
 *                       : 0 - otherwise
 */
int mtb_ml_model_get_recurrent_time_series_frames(const mtb_ml_model_t *object);

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
void* mtb_ml_model_get_inference_object(const mtb_ml_model_t *object);

/**
 * \brief : Read time stamp counter (TSC) .
 *
 * Platform specific function to read HW time stamp counter or OS tick timer counter for profiling.
 * The application program developer should provide this function if profiling is enabled.
 *
 * \param[out]   val        : Pointer to time stamp counter return value
 *
 * \return                  : Return 0 when success, otherwise return error code
 */
int mtb_ml_model_profile_get_tsc(uint32_t *val);

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

/**
 * \brief : Set NN model input data Q-format fraction bits
 *          Only apply to IFX inference engine in integer mode
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
 *          Only apply to IFX inference engine in integer mode
 *
 * \param[in] object     : Pointer of model object.
 *
 * \return               : Q-format fraction bit
 *                       : 0 - if input parameter is invalid.
 */
uint8_t mtb_ml_model_get_output_q_fraction_bits(const mtb_ml_model_t *object);

/**
 * @} end of Model_API group
 */

#if defined(__cplusplus)
}
#endif

#endif /* __MTB_ML_MODEL_H__ */
