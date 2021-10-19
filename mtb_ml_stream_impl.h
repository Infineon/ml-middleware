/***************************************************************************//**
* \file mtb_ml_stream_impl.h
*
* \brief
* This file contains common definitions for MTB ML streaming feature
*
*******************************************************************************
* \copyright
* Copyright 2021, Cypress Semiconductor Corporation (an Infineon company).
* All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
******************************************************************************/

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
    int n_out_classes;      /**< NN model inference classification output size */
    int scratch_mem;        /**< Scratch memory size required for inference */
    int persistent_mem;     /**< Persistent memory size required for inference */
    int recurrent_ts_size;  /**< Recurrent time series sample size or zero if not recurrent network */
    unsigned int model_sz;  /**< Model's weights & biases */
} cy_ml_regression_info_t;

typedef struct
{
    int n_ex;               /**< Number of frames in the data set */
    int in_sz;              /**< Frame size in data units*/
    int q_fixed;            /**< fixed-point Q factor;*/
    int input_size;         /**< Input data unit size */
    int output_size;        /**< Output data unit size */
    int baud_rate;          /**< Communication port baudrate */
} cy_ml_dataset_header_t;

/*******************************************************************************
* Function prototypes
******************************************************************************/

#if defined(__cplusplus)
}
#endif

#endif /* __MTB_ML_STREAM_IMPL_H__ */
