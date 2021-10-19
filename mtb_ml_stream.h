/***************************************************************************//**
* \file mtb_ml_stream.h
*
* \brief
* This file contains interface for MTB ML streaming feature
*
*******************************************************************************
* \copyright
* Copyright 2021, Cypress Semiconductor Corporation (an Infineon company).
* All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
******************************************************************************/

#if !defined(__MTB_ML_STREAM_H__)
#define __MTB_ML_STREAM_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "mtb_ml_model.h"
#include "mtb_ml_stream_impl.h"

/******************************************************************************
* Compile-time flags
*****************************************************************************/

/******************************************************************************
* Enums
*****************************************************************************/

/******************************************************************************
* Macros
*****************************************************************************/

/*******************************************************************************
* extern variables
******************************************************************************/

/*******************************************************************************
* Structures and enumerations
******************************************************************************/

/**
 * Communication ports
 */
typedef enum
{
    CY_ML_INTERFACE_UART = 1,
    CY_ML_INTERFACE_SPI,
    CY_ML_INTERFACE_I2C,
    CY_ML_INTERFACE_USB
} mtb_ml_stream_bus_t;

/**
 * Stream interface metadata
 */
typedef struct
{
    mtb_ml_stream_bus_t bus;   /**< Communication port for streaming */
    void *interface_obj;       /**< Interface HAL object, i.e. from
                                    cyhal_uart_init or cy_retarget_io_init*/
} mtb_ml_stream_interface_t;

/*******************************************************************************
* Function prototypes
******************************************************************************/
/**
 * \addtogroup Stream_API
 * @{
 */

/**
 * \brief : Prepare for data streaming: initialize selected NN model, initialize
 *          module's variables, allocate communication and ML buffers, setup
 *          profiling.
 *
 * \param[in]   interface   : Stream interface metadata
 * \param[in]   profile_cfg : Profile configuration
 * \param[in]   model_bin   : Model binary data
 *
 * \return                  : MTB_ML_RESULT_SUCCESS - success
 *                          : otherwise - check the return value for detail.
 */
cy_rslt_t mtb_ml_stream_init(mtb_ml_stream_interface_t *interface,
                       mtb_ml_profile_config_t profile_cfg,
                       mtb_ml_model_bin_t *model_bin);

/**
 * \brief : Model validation using data streaming. This function receives data
 *          and performs frame-by-frame inference with the NN model, then
 *          uploads inference output back to the host for validation.
 *
 * \return                  : MTB_ML_RESULT_SUCCESS - success
 *                          : otherwise - check the return value for detail.
 */
cy_rslt_t mtb_ml_stream_task(void);

/**
 * \brief : De-initialize the stream module: deinit the model, release buffers.
 *
 * \return                  : None
 */
void mtb_ml_stream_deinit(void);

/**
 * @} end of Stream_API group
 */

#if defined(__cplusplus)
}
#endif

#endif /* __MTB_ML_STREAM_H__ */
