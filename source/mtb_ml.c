/***************************************************************************//**
* \file mtb_ml.c
*
* \brief
* The file contains application programming interface to the ModusToolbox ML
* middleware module
*
*******************************************************************************
* (c) 2025-2026, Infineon Technologies AG, or
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
#include <inttypes.h>
#include "cy_pdl.h"
#include "mtb_ml.h"

/******************************************************************************
 * Public variables
******************************************************************************/
uint32_t mtb_ml_cpu_clk_freq;
#if defined(COMPONENT_U55) || \
    defined(COMPONENT_NNLITE2)
uint64_t mtb_ml_npu_cycles;
uint32_t mtb_ml_npu_clk_freq;
float mtb_ml_norm_clk_freq;
#endif

#if defined(COMPONENT_U55)
extern struct ethosu_driver ethosu_drv;
#endif

/******************************************************************************
 * Static variables
******************************************************************************/
static uint32_t mtb_ml_init_state = 0;

/*******************************************************************************
 * Public Functions
*******************************************************************************/
#if defined(COMPONENT_U55)
extern cy_rslt_t mtb_ml_ethosu_init(struct ethosu_driver *drv, uint32_t priority);
extern cy_rslt_t mtb_ml_ethosu_deinit(void);
#elif defined(COMPONENT_NNLITE2)
extern cy_rslt_t mtb_ml_nnlite_init(uint32_t priority);
extern cy_rslt_t mtb_ml_nnlite_deinit(void);
#endif

cy_rslt_t mtb_ml_init(uint32_t priority)
{
    cy_rslt_t result = MTB_ML_RESULT_SUCCESS;
#if !defined(COMPONENT_U55) && !defined(COMPONENT_NNLITE2)
    (void)priority;
#endif

    if (mtb_ml_init_state == 0)
    {
        mtb_ml_cpu_clk_freq = (uint32_t)SystemCoreClock;

#if defined(COMPONENT_U55)
        result = mtb_ml_ethosu_init(&ethosu_drv, priority);
#elif defined(COMPONENT_NNLITE2)
        result = mtb_ml_nnlite_init(priority);
#endif

#if defined(COMPONENT_U55) || \
        defined(COMPONENT_NNLITE2)
        mtb_ml_norm_clk_freq = ((float)mtb_ml_npu_clk_freq) / ((float)mtb_ml_cpu_clk_freq);
#endif
    }
    mtb_ml_init_state++;
    return result;
}

cy_rslt_t mtb_ml_deinit(void)
{
    cy_rslt_t result = MTB_ML_RESULT_SUCCESS;
    if (mtb_ml_init_state == 1)
    {
#if defined(COMPONENT_U55)
        result = mtb_ml_ethosu_deinit();
#elif defined(COMPONENT_NNLITE2)
        result = mtb_ml_nnlite_deinit();
#endif
    }
    if (mtb_ml_init_state > 0)
    {
        mtb_ml_init_state--;
    }
    return result;
}

uint32_t mtb_ml_get_init_state(void)
{
    return mtb_ml_init_state;
}
