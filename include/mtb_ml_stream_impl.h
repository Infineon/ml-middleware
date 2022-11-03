/***************************************************************************//**
* \file mtb_ml_stream_impl.h
*
* \brief
* This file contains common definitions for MTB ML streaming feature
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
#if !defined(__MTB_ML_STREAM_IMPL_H__)
#define __MTB_ML_STREAM_IMPL_H__

#if defined(__cplusplus)
extern "C" {
#endif

/******************************************************************************
* Compile-time flags
*****************************************************************************/

/******************************************************************************
* Enums
*****************************************************************************/

typedef enum {FLOAT32, INT32, INT16, DOUBLE, INT8} var_type_t;
typedef enum {ENG_UNKNOWN, ENG_IFX, ENG_TFLM} engine_type_t;

/******************************************************************************
* Macros
*****************************************************************************/

#define ML_PROFILE_FRAME_STRING         "Frame "
#define ML_PROFILE_OUTPUT_STRING        " output:"
#define ML_PROFILE_INFO_STRING          "PROFILE_INFO,"

#define ML_TC_START_STRING              "ML_START"
#define ML_CT_READY_STRING              "ML_READY"
#define ML_TC_MODEL_DATA_REQ_STRING     "ML_MODEL_DATA_REQ"
#define ML_CT_MODEL_DATA_STRING         "ML_MODEL_DATA"
#define ML_TC_DATASET_REQ_SEND_STRING   "ML_DATASET_SENDREQ"
#define ML_CT_FRAME_REQ_STRING          "ML_FRAME"
#define ML_CT_RESULT_STRING             "ML_RESULT"
#define ML_TC_DONE_STRING               "ML_COMPLETED"
#define ML_CT_DONE_STRING               "ML_DONE"
#define ML_ERROR_STRING                 "ERROR"

#if __APPLE__
/* MACOS implementation doesn't support baud rates higher than 115200 */
#define UART_DEFAULT_STREAM_BAUD_RATE       115200
#else
#define UART_DEFAULT_STREAM_BAUD_RATE       1000000
#endif

/*******************************************************************************
* extern variables
******************************************************************************/

/*******************************************************************************
* Structures and enumerations
******************************************************************************/
typedef struct
  {
    int output_size;                /**< NN model inference classification output size */
    int buffer_size;                /**< Runtime buffer size required for inference */
    int model_size;                 /**< Model's weights & biases */
    int engine_type;      /**< Inference engine type */
    int recurrent_ts_size;          /**< Recurrent time series sample size or zero if not recurrent network */
    int output_zero_point;              /**< zero point of output data */
    float output_scale;                 /**< scale of output data */
  } cy_ml_regression_info_t;

typedef struct
{
    mtb_ml_x_data_type_t data_type; /**< Dataset data type */
    int n_ex;                       /**< Number of frames in the data set */
    int in_sz;                      /**< Frame size in data units*/
    int q_fixed;                    /**< fixed-point Q factor;*/
    int input_size;                 /**< Input data unit size */
    int output_size;                /**< Output data unit size */
    int baud_rate;                  /**< Communication port baudrate */
} cy_ml_dataset_header_t;

/*******************************************************************************
* Function prototypes
******************************************************************************/

#if defined(__cplusplus)
}
#endif

#endif /* __MTB_ML_STREAM_IMPL_H__ */
