/***************************************************************************//**
* \file mtb_ml_stream.h
*
* \brief
* This file contains interface for MTB ML streaming feature
*
*******************************************************************************
* (c) 2019-2026, Infineon Technologies AG, or
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
#if !defined(__MTB_ML_STREAM_H__)
#define __MTB_ML_STREAM_H__

#ifdef COMPONENT_ML_MW_STREAM

#include "mtb_ml_model.h"
#include "mtb_ml_dataset.h"


#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(COMPONENT_MTB_HAL)
#include "cyhal.h"
#else
#include "mtb_hal.h"
#endif

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
 * Streaming vcontext
*/
typedef struct
{
    void* placeholder[3]; /**< Implementation relies on 96 bytes (3 32 bits) of data */
} mtb_data_streaming_vcontext_t;

/**
 * Streaming send
*/
typedef cy_rslt_t (* mtb_data_streaming_send_t)(mtb_data_streaming_vcontext_t* context,
                                                void *data, size_t count, void* tag);

/**
 * Streaming receive
*/
typedef cy_rslt_t (* mtb_data_streaming_receive_t)(mtb_data_streaming_vcontext_t* context,
                                                   uint8_t* data, size_t count, void* tag);

/**
 * Streaming xfer done
*/
typedef void (* mtb_data_streaming_xfer_done_t)(void* tag, cy_rslt_t rslt);

/**
 * Streaming obj
*/
typedef struct
{
#if !defined(COMPONENT_MTB_HAL)
    cyhal_uart_t*                       uart; /**< uart handle */
#else
    mtb_hal_uart_t*                     uart; /**< uart handle */
#endif
} mtb_data_streaming_obj_t;

/**
 * Streaming context
*/
typedef struct
{
    mtb_data_streaming_obj_t        obj_inst; /**< object instance */
    mtb_data_streaming_xfer_done_t  callback; /**< callback function */
    void*                           call_tag; /**< pointer to call tag */
} mtb_data_streaming_context_t;


/**
 * Streaming interface
*/
typedef struct
{
    mtb_data_streaming_send_t send;         /**< Function to send data to a host device. */
    mtb_data_streaming_receive_t receive;   /**< Function to receive data from a host device. */
    mtb_data_streaming_vcontext_t context;  /**< Context data for performing operations. */
} mtb_data_streaming_interface_t;


/**
 * Stream tag
*/
typedef struct
{
    bool stream_done;           /**< Flag set/reset to track status of stream */
    cy_rslt_t stream_status;    /**< Status of stream event */
} mtb_ml_stream_tag_t;


/**
 * Stream interface
 */
typedef struct
{
    mtb_data_streaming_interface_t *interface_obj;  /**< Stream interface object */
    mtb_ml_stream_tag_t *stream_tag;                /**< Tag that tracks status of stream */
    mtb_ml_x_file_header_t x_data_info;             /**< x data info retrieved from host (input_size in elements) */
    size_t input_size;                              /**< Size of input data buffer, set to model object input size */
    size_t output_size;                             /**< Size of output data buffer, set to model object output size */
    size_t input_data_size;                         /**< Size of input data buffer, set to model object input size */
    size_t output_data_size;                        /**< Size of output data buffer, set to model object output size */
} mtb_ml_stream_interface_t;

/*******************************************************************************
* Function prototypes
******************************************************************************/
/**
 * \addtogroup Stream_API
 * @{
 */

/**
 * \brief : Prepare for data streaming: initialize members of interface, validate input test data
 *          matches dimensions of model object.
 *
 * \param[in]   interface       :   Stream interface provided by user. interface_obj, rx_tag,
 *                                  and tx_tag must be allocated by user. input_size and output_size
 *                                  will be set by call.
 * \param[in]   model_object    :   Model object associated with interface. Parsed to verify input
 *                                  test data and initialize interface.
 * \return                      :   MTB_ML_RESULT_SUCCESS - success
 *                              :   otherwise - check the return value for detail.
 */
cy_rslt_t mtb_ml_stream_init(mtb_ml_stream_interface_t *interface,
                            const mtb_ml_model_t *model_object);
/**
 * \brief : Streams output test data via interface.
 *
 * \param[in]   interface   :   Stream interface provided by user. Used to stream output data.
 * \param[in]   tx_buf      :   Transmit buffer. Assumed to be of size interface->output_size.
 * \param[in]   timeout_ms  :   Timeout in milliseconds. Value of 0 means attempt to transmit data
 *                              forever.
 * \return                  :   MTB_ML_RESULT_SUCCESS - success
 *                          :   otherwise - check the return value for detail.
 */
cy_rslt_t mtb_ml_stream_output_data(mtb_ml_stream_interface_t *interface,
                                    void *tx_buf,
                                    uint32_t timeout_ms);
/**
 * \brief : Streams input test data via interface.
 *
 * \param[in]   interface   :   Stream interface provided by user. Used to stream input data.
 * \param[in]   rx_buf      :   Receive buffer. Assumed to be of size interface->input_size.
 * \param[in]   timeout_ms  :   Timeout in milliseconds. Value of 0 means attempt to receive data
 *                              forever.
 * \return                  :   MTB_ML_RESULT_SUCCESS - success
 *                          :   otherwise - check the return value for detail.
 */
cy_rslt_t mtb_ml_stream_input_data( mtb_ml_stream_interface_t *interface,
                                    void *rx_buf,
                                    uint32_t timeout_ms);
/**
 * \brief : API to inform host application when the device task is complete
 *
 * \param[in]   interface   :   Stream interface provided by user. Used to stream input data.
 * \param[in]   timeout_ms  :   Timeout in milliseconds. Value of 0 means attempt to receive data
 *                              forever.
 * \return                  :   MTB_ML_RESULT_SUCCESS - success
 *                          :   otherwise - check the return value for detail.
 */
cy_rslt_t mtb_ml_inform_host_done( mtb_ml_stream_interface_t *interface, uint32_t timeout_ms);
/**
 * \brief : TX/RX callback function.
 *
 * \param[in]   tag     :   Used to store arbitrary caller information.
 * \param[in]   rslt    :   Result of TX/RX parent callback.
 * \return              :   None
 */
void mtb_ml_stream_cb(void* tag, cy_rslt_t rslt);

/**
 * @} end of Stream_API group
 */

#if defined(__cplusplus)
}
#endif

#endif /* COMPONENT_ML_MW_STREAM */

#endif /* __MTB_ML_STREAM_H__ */
