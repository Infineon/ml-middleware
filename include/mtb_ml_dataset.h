/***************************************************************************//**
* \file mtb_ml_dataset.h
*
* \brief
* This is the test dataset header file of ModusToolbox ML middleware library.
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
#if !defined(__MTB_ML_DATASET_H__)
#define __MTB_ML_DATASET_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Typedefs
 *****************************************************************************/

typedef enum MTB_ML_X_DATA_TYPE {
    MTB_ML_X_DATA_UNKNOWN = 0,
    MTB_ML_X_DATA_FLOAT32 = 1,
    MTB_ML_X_DATA_INT8 =    2,
    MTB_ML_X_DATA_INT16 =   3,
} mtb_ml_x_data_type_t;

/******************************************************************************
* Public definitions
******************************************************************************/

/******************************************************************************
* Structures
******************************************************************************/
/**
 * Sample data file header data structure
 */
typedef struct mtb_ml_x_file_header
{
    mtb_ml_x_data_type_t data_type; /**< data type of sample dataset */
    int32_t num_of_samples;         /**< number of samples (or frames) in dataset */
    int32_t input_size;             /**< Input length (or frame size) of dataset */
    int32_t recurrent_ts_size;      /**< -1 - non-RNN model, 1 - non-streaming RNN, other - number of time steps */
} mtb_ml_x_file_header_t;

#if defined(__cplusplus)
}
#endif

#endif /* __MTB_ML_DATASET_H__ */
