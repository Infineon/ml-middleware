/***************************************************************************//**
* \file mtb_ml_stream.c
*
* \brief
* This file contains interface for ML validation data streaming feature
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
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#include "mtb_ml_common.h"
#include "mtb_ml_utils.h"
#include "mtb_ml_stream.h"
#include "mtb_ml_stream_impl.h"

#include "mtb_ml_model.h"

typedef struct
{
    mtb_ml_model_t *model_object;           /* Pointer of model object */
    void *interface;                        /* Communication interface object */
    MTB_ML_DATA_T* out;                     /* Model output buffer */
    uint8_t *rx_buff;                       /* Receive buffer for frame data */
    const mtb_ml_model_bin_t *model_bin;    /* Model binary structure */
} mtb_ml_stream_t;

static cy_ml_dataset_header_t dataset_info = {
    .data_type = MTB_ML_X_DATA_UNKNOWN,
    .n_ex = 0,
    .in_sz = 0,
    .q_fixed = 0,
    .input_size = 0,
    .output_size = 0,
    .baud_rate = 0
};
static cyhal_uart_t *uartobj;

mtb_ml_stream_t stream_obj;

#define DEFAULT_TX_TIMEOUT 1000
#define DEFAULT_RX_TIMEOUT 5000
#define ML_START_TIMEOUT   10000

/**
 * Send binary data via UART interface
 */
static cy_rslt_t uart_send_data(char* tx_buff, size_t tx_length)
{
    size_t requested_tx = tx_length;
    cy_rslt_t uart_ret;
    size_t bytes_sent = 0;
    char *buff = tx_buff;
    uint32_t timeout = DEFAULT_TX_TIMEOUT;

    while(bytes_sent < requested_tx)
    {
        /* Wait until UART TX buffer has some space */
        while(cyhal_uart_writable(uartobj) < 32)
        {
            Cy_SysLib_Delay(1);
            if(--timeout == 0)
            {
                printf("ERROR: timeout waiting for TX buffer\r\n");
                return MTB_ML_RESULT_TIMEOUT;
            }
        }

        uart_ret = cyhal_uart_write(uartobj, buff, &tx_length);
        if (uart_ret != CY_RSLT_SUCCESS)
        {
            printf("ERROR: UART write error\n");
            return MTB_ML_RESULT_COMM_ERROR;
        }
        bytes_sent += tx_length;
        buff = buff + (char)tx_length;
        tx_length = requested_tx - bytes_sent;
    }

    if(bytes_sent != requested_tx)
    {
        printf("ERROR: string length mismatch tx_length: %d, expected: %d\n",
                bytes_sent, requested_tx);
        return MTB_ML_RESULT_COMM_ERROR;
    }
    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Receive binary data via UART interface
 */
static cy_rslt_t uart_get_data(void *rx_buff, size_t length, int timeout)
{
    size_t rx_counter = 0;
    cy_rslt_t uart_ret;
    size_t rx_length = length;
    uint8_t *rx_pointer = (uint8_t *)rx_buff;

    while(rx_counter < length)
    {
        rx_length = length - rx_counter;
        uart_ret = cyhal_uart_read(uartobj, rx_pointer, &rx_length);
        if (uart_ret != CY_RSLT_SUCCESS)
        {
            printf("ERROR: UART read error\n");
            return MTB_ML_RESULT_COMM_ERROR;
        }
        if(rx_length == 0)
        {
            if(--timeout == 0)
            {
                printf("ERROR: timeout receiving data, received %d of %d bytes\r\n",
                       rx_counter, length);
                return MTB_ML_RESULT_TIMEOUT;
            }
            Cy_SysLib_Delay(1);
        }
        rx_counter += rx_length;
        rx_pointer += rx_length;
    }

    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Wait for a string in UART data
 */
static cy_rslt_t uart_wait_for_string(const char *string, size_t size, uint32_t timeout)
{
    cy_rslt_t result = MTB_ML_RESULT_SUCCESS;
    uint8_t uart_rx_data;
    uint32_t position = 0;
    while(true)
    {
        result = cyhal_uart_getc(uartobj, &uart_rx_data, timeout);
        if (result == CY_RSLT_SUCCESS)
        {
            if(uart_rx_data == string[position])
            {
                if((position + 1) == size)
                {
                    /* found the string */
                    result = MTB_ML_RESULT_SUCCESS;
                    break;
                }
                position++;
            }
            else
            {
                /* this is not the sequence, start over */
                position = 0;
            }
        }
        else
        {
            result = MTB_ML_RESULT_COMM_ERROR;
            break;
        }
    }
    return result;
}

/**
 * Establish connection with the host for data streaming
 */
static cy_rslt_t mtb_ml_stream_handshake(void)
{
    cy_rslt_t result;
    cy_ml_regression_info_t model_regr_info;

    /* Wait for the start string from the host (ML_START) */
    printf("Waiting for the data stream to begin...\n");
    result = uart_wait_for_string(ML_TC_START_STRING,
                                  sizeof(ML_TC_START_STRING), ML_START_TIMEOUT);
    if(result != MTB_ML_RESULT_SUCCESS)
    {
        printf("Didn't receive %s from host within %d seconds\n", ML_TC_START_STRING,
                ML_START_TIMEOUT/1000);
        return MTB_ML_RESULT_TIMEOUT;
    }

    /* Print the model info */
    mtb_ml_utils_print_model_info(stream_obj.model_object);

    /* Indicate the host that we are ready (ML_READY) */
    result = uart_send_data(ML_CT_READY_STRING, sizeof(ML_CT_READY_STRING));
    if (result != MTB_ML_RESULT_SUCCESS)
    {
        printf("ERROR: failed to send %s to host\n", ML_CT_READY_STRING);
        return MTB_ML_RESULT_COMM_ERROR;
    }

    /* Wait for the model info from the host */
    result = uart_wait_for_string(ML_TC_MODEL_DATA_REQ_STRING,
                                  sizeof(ML_TC_MODEL_DATA_REQ_STRING), DEFAULT_RX_TIMEOUT);
    if(result != MTB_ML_RESULT_SUCCESS)
    {
        printf("ERROR: failed to receive %s from host\n", ML_TC_MODEL_DATA_REQ_STRING);
        return MTB_ML_RESULT_COMM_ERROR;
    }

    /* Send model info header */
    result = uart_send_data(ML_CT_MODEL_DATA_STRING, sizeof(ML_CT_MODEL_DATA_STRING));
    if (result != MTB_ML_RESULT_SUCCESS)
    {
        printf("ERROR: failed to send %s to host\n", ML_CT_MODEL_DATA_STRING);
        return MTB_ML_RESULT_COMM_ERROR;
    }

    /* send model info */
    model_regr_info.model_size = stream_obj.model_object->model_size;
    model_regr_info.output_size = stream_obj.model_object->output_size;
    model_regr_info.buffer_size = stream_obj.model_object->buffer_size;
    model_regr_info.engine_type = ENG_TFLM;
    model_regr_info.recurrent_ts_size = 0;
#if defined(COMPONENT_ML_IFX)
    model_regr_info.engine_type = ENG_IFX;
    cy_stc_ml_model_info_t *model_info = &stream_obj.model_object->model_info;
    model_regr_info.recurrent_ts_size = model_info->recurrent_ts_size;
#else
    // output_scale and output_zero_point are only available for TFLM
    model_regr_info.output_zero_point = stream_obj.model_object->output_zero_point;
    model_regr_info.output_scale = stream_obj.model_object->output_scale;
#endif
    result = uart_send_data((char *)&model_regr_info, sizeof(model_regr_info));
    if (result != MTB_ML_RESULT_SUCCESS)
    {
        printf("ERROR: failed to send model info to host\n");
        return MTB_ML_RESULT_COMM_ERROR;
    }

    /* receive datasets info */
    result = uart_wait_for_string(ML_TC_DATASET_REQ_SEND_STRING,
                               sizeof(ML_TC_DATASET_REQ_SEND_STRING), DEFAULT_RX_TIMEOUT);
    if(result != MTB_ML_RESULT_SUCCESS)
    {
        printf("ERROR: failed to receive %s from the host\n", ML_TC_DATASET_REQ_SEND_STRING);
        return MTB_ML_RESULT_COMM_ERROR;
    }

    result = uart_send_data(ML_CT_READY_STRING, sizeof(ML_CT_READY_STRING));
    if (result != MTB_ML_RESULT_SUCCESS)
    {
        printf("ERROR: failed to send %s to the host\n", ML_CT_READY_STRING);
        return MTB_ML_RESULT_COMM_ERROR;
    }

    result = uart_get_data(&dataset_info, sizeof(dataset_info), DEFAULT_RX_TIMEOUT);
    if (result != MTB_ML_RESULT_SUCCESS)
    {
        printf("ERROR: failure receiving dataset header\n");
        return MTB_ML_RESULT_COMM_ERROR;
    }

    return MTB_ML_RESULT_SUCCESS;
}

/**
 * Perform data streaming
 */
cy_rslt_t mtb_ml_stream_task()
{
    cy_rslt_t result = MTB_ML_RESULT_SUCCESS;

    mtb_ml_model_t *model_object = stream_obj.model_object;

    void *in_buffer = NULL;

    int n_ex, in_sz, ts_cnt=0;
    size_t data_size = 0;

    result = mtb_ml_stream_handshake();
    if(result != MTB_ML_RESULT_SUCCESS)
    {
        return result;
    }

    n_ex = dataset_info.n_ex;
    in_sz = dataset_info.in_sz;
    data_size = dataset_info.input_size;
    printf("\r\nReceived: num frames:%d frame size:%d in_size: %d out_size: %d ",
           n_ex, in_sz, dataset_info.input_size, dataset_info.output_size);
#if !COMPONENT_ML_FLOAT32
    printf("in_q_fixed:%d\r\n",dataset_info.q_fixed);
#else
    printf("\r\n");
#endif

    if (in_sz != model_object->input_size)
    {
        printf("ERROR: input buffer size error, input size=%d, model input size=%d, aborting... \r\n",
                in_sz, model_object->input_size);
        result = MTB_ML_RESULT_BAD_ARG;
        return result;
    }

    if(dataset_info.output_size != sizeof(MTB_ML_DATA_T))
    {
        printf("ERROR: Streaming data size (%d) doesn't match NN data size (%d), aborting\r\n",
                dataset_info.output_size, sizeof(MTB_ML_DATA_T));
        result = MTB_ML_RESULT_BAD_ARG;
        return result;
    }

    if(dataset_info.input_size != sizeof(MTB_ML_DATA_T))
    {
        printf("ERROR: Streaming input unit size (%d) doesn't match NN (%d), aborting\r\n",
                dataset_info.input_size, sizeof(MTB_ML_DATA_T));
        result = MTB_ML_RESULT_BAD_ARG;
        return result;
    }

    int recurrent_ts_size = mtb_ml_model_get_recurrent_time_series_frames(model_object);
    if(recurrent_ts_size > 0)
    {
        /* Perform recurrent NN reset (rnn_status = 1) */
        mtb_ml_model_rnn_state_control(model_object, 1, recurrent_ts_size);
    }

    /* Setup q-factor if input data is in Q-format */
    mtb_ml_model_set_input_q_fraction_bits(model_object, dataset_info.q_fixed);
    /* Do frame-by-frame inference */
    for (int j = 0; j < n_ex; j++)
    {
        ts_cnt++;

        result = uart_send_data(ML_CT_FRAME_REQ_STRING, sizeof(ML_CT_FRAME_REQ_STRING));
        if(result != MTB_ML_RESULT_SUCCESS)
        {
            printf("ERROR: failed to send frame request\n");
            return result;
        }

        /* Calculate timeout as ten times of 1 second per 10KB at 115200*/
        /* For a small (a few bytes) transfers calculated timeout with the
           asatiated overhead may be not long enough */
        int rx_timeout = (10 * 1000 * in_sz * data_size)/(10*1024);
        if(rx_timeout < DEFAULT_RX_TIMEOUT)
        {
            rx_timeout = DEFAULT_RX_TIMEOUT;
        }
        result = uart_get_data(stream_obj.rx_buff, in_sz * data_size, rx_timeout);
        if(result != MTB_ML_RESULT_SUCCESS)
        {
            printf("ERROR: failed to receive frame data\n");
            return result;
        }

        in_buffer = stream_obj.rx_buff;

        /* Run inference */
        result = mtb_ml_model_run(model_object, in_buffer);
        if (result != MTB_ML_RESULT_SUCCESS)
        {
            int err = model_object->lib_error;
#if defined(COMPONENT_ML_IFX)
            printf("ERROR: Inference error code=%x, Layer index=%d, Line number=%d, Frame number=%d, aborting\r\n",
            CY_ML_ERR_CODE(err), CY_ML_ERR_LAYER_INDEX(err), CY_ML_ERR_LINE_NUMBER(err), j);
#elif defined(COMPONENT_ML_TFLM_INTERPRETER) || defined(COMPONENT_ML_TFLM_INTERPRETER_LESS)
            printf("Inference eror: Tflite-Micro status: %d\r\n", err);
#endif
            return result;
        }

        /* Recurrent GRU network only check accuracy at its end of time series */
        if ( recurrent_ts_size == 0 ||
            (recurrent_ts_size != 0 && ts_cnt == recurrent_ts_size))
        {
            ts_cnt = 0;
            result = uart_send_data(ML_CT_RESULT_STRING, sizeof(ML_CT_RESULT_STRING));
            if(result != MTB_ML_RESULT_SUCCESS)
            {
                printf("ERROR: failed to send %s to host\n", ML_CT_RESULT_STRING);
                return result;
            }
            result = uart_send_data((char *)stream_obj.out, dataset_info.output_size * model_object->output_size);
            if(result != MTB_ML_RESULT_SUCCESS)
            {
                printf("ERROR: failed to send output data to host\n");
                return result;
            }

#if !COMPONENT_ML_FLOAT32
            int32_t out_q = mtb_ml_model_get_output_q_fraction_bits(model_object);
            result = uart_send_data((char *)&out_q, sizeof(out_q));
            if(result != MTB_ML_RESULT_SUCCESS)
            {
                printf("ERROR: failed to send q-fraction bits data to host\n");
                return result;
            }
#endif
        }
    }

    /* Generate profiling log if it is enabled */
    mtb_ml_model_profile_log(model_object);

    /* In the stream mode, all calculations are done on the host.*/
    result = uart_wait_for_string(ML_TC_DONE_STRING,
                                  sizeof(ML_TC_DONE_STRING), DEFAULT_RX_TIMEOUT);
    if(result != MTB_ML_RESULT_SUCCESS)
    {
        printf("ERROR: failure receiving %s from the host\n", ML_TC_DONE_STRING);
        return result;
    }

    result = uart_send_data(ML_CT_DONE_STRING, sizeof(ML_CT_DONE_STRING));
    if (result != MTB_ML_RESULT_SUCCESS)
    {
        printf("ERROR: failed to send %s to the host\n", ML_CT_DONE_STRING);
        return result;
    }

    return result;
}

/**
 * Initialize stream
 */
cy_rslt_t mtb_ml_stream_init(const mtb_ml_stream_interface_t *interface, mtb_ml_profile_config_t profile_cfg,
                             const mtb_ml_model_bin_t *model_bin)
{
    if(!interface || !model_bin)
    {
        printf("ERROR: mtb_ml_stream_init invalid parameters\r\n");
        return MTB_ML_RESULT_BAD_ARG;
    }

    switch (interface->bus) {
        case CY_ML_INTERFACE_UART:
            uartobj = interface->interface_obj;
            break;
        default:
            printf("ERROR: mtb_ml_stream_init invalid parameters\r\n");
            return MTB_ML_RESULT_BAD_ARG;
    }

    /* Initialize the model runtime context and allocate resources */
    cy_rslt_t result = mtb_ml_model_init(model_bin, NULL,
                                         &stream_obj.model_object);
    if (result != MTB_ML_RESULT_SUCCESS)
    {
        printf("MTB ML initialization failure:%"PRIu32"\r\n", result);
        return result;
    }

    stream_obj.model_bin = model_bin;

    /* Allocate buffers */

    /* Get the model output size */
    int output_size;
    mtb_ml_model_get_output(stream_obj.model_object, &stream_obj.out, &output_size);

    /* allocate rx buffer (frame data) */
    int input_size = mtb_ml_model_get_input_size(stream_obj.model_object);
    stream_obj.rx_buff = (uint8_t *)malloc(input_size * sizeof(MTB_ML_DATA_T));
    if (stream_obj.rx_buff == NULL)
    {
        printf("ERROR: Allocate memory for RX buffer failed\r\n");
        mtb_ml_model_deinit(stream_obj.model_object);
        return MTB_ML_RESULT_ALLOC_ERR;
    }

    /* Setup profile configuration */
    mtb_ml_model_profile_config(stream_obj.model_object, profile_cfg);

    return MTB_ML_RESULT_SUCCESS;
}

/**
 *  De-init stream
 */
void mtb_ml_stream_deinit(void)
{
    /* Free resources and delete model runtime contex */
    mtb_ml_model_deinit(stream_obj.model_object);
    free(stream_obj.rx_buff);
}
