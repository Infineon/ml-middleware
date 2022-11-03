/***************************************************************************//**
* \file mtb_ml_stream.h
*
* \brief
* This file contains interface for MTB ML streaming feature
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
#if !defined(__MTB_ML_STREAM_H__)
#define __MTB_ML_STREAM_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "mtb_ml_model.h"
#include "mtb_ml_dataset.h"
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
cy_rslt_t mtb_ml_stream_init(const mtb_ml_stream_interface_t *interface,
                       mtb_ml_profile_config_t profile_cfg,
                       const mtb_ml_model_bin_t *model_bin);

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
