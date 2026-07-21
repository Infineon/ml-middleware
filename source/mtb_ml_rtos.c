/******************************************************************************
* File Name: mtb_ml_rtos.c
*
* Description: This file contains implementations of
*              the mutex and semaphore functions.
*
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
#ifdef CY_RTOS_AWARE

#include "mtb_ml.h"

#if defined(COMPONENT_U55) || \
    defined(COMPONENT_NNLITE2)

#include "cyabs_rtos.h"
#include "cy_pdl.h"
#include <stdlib.h>

/*******************************************************************************
 * Private Functions
*******************************************************************************/
/**
 * \brief   : Create mutex and return handle using API of RTOS abstraction.
 *
 * \return  : Mutex handle
 */
void *mtb_ml_mutex_create(void)
{
    cy_mutex_t *mutex;
    mutex = (cy_mutex_t *)malloc(sizeof(cy_mutex_t));
    if(mutex == NULL)
    {
        return NULL;
    }

    cy_rslt_t res = cy_rtos_init_mutex2(mutex, false);
    if (res != CY_RSLT_SUCCESS) {
        free(mutex);
        return NULL;
    }

    return (void *)mutex;
}

/**
 * \brief : Lock mutex using API of RTOS abstraction.
 *
 * \param[in]   mutex  : mutex handle to be locked
 */
int mtb_ml_mutex_lock(void *mutex)
{
    cy_mutex_t *handle = (cy_mutex_t *)mutex;
    cy_rslt_t res = cy_rtos_get_mutex(handle, ML_NPU_MUTEX_TIMEOUT);

    return res;
}

/**
 * \brief : Unlock mutex using API of RTOS abstraction.
 *
 * \param[in]   mutex  : mutex handle to be unlocked
 */
int mtb_ml_mutex_unlock(void *mutex)
{
    cy_mutex_t *handle = (cy_mutex_t *)mutex;
    cy_rslt_t res = cy_rtos_set_mutex(handle);

    return res;
}

/**
 * \brief : Destroy mutex and free allocated memory.
 *
 * \param[in]   mutex  : mutex handle to be destroyed
 */
void mtb_ml_mutex_destroy(void *mutex)
{
    if (mutex != NULL)
    {
        if (cy_rtos_deinit_mutex(mutex) == CY_RSLT_SUCCESS)
        {
            free(mutex);
        }
    }
}

/**
 * \brief   : Create semaphore and return handle using API of RTOS abstraction.
 *
 * \return  : Semaphore handle
 */
void *mtb_ml_sem_create(void)
{
    cy_semaphore_t *sem;
    sem = (cy_semaphore_t *)malloc(sizeof(cy_semaphore_t));
    if(sem == NULL)
    {
        return NULL;
    }
    cy_rslt_t res = cy_rtos_init_semaphore(sem, configNPU_COUNT, 0);
    if (res != CY_RSLT_SUCCESS) {
        free(sem);
        return NULL;
    }

    return (void *)sem;
}

/**
 * \brief : Take semaphore using API of RTOS abstraction.
 *
 * \param[in]   sem  : semaphore handle to take
 */
int mtb_ml_sem_take(void *sem, uint64_t timeout)
{
    cy_semaphore_t *handle = (cy_semaphore_t *)sem;
    cy_rslt_t res;

    /* Check if we're in an ISR */
    if (__get_IPSR() != 0)
    {
        res = cy_rtos_get_semaphore(handle, 0, true);
    }
    else
    {
        /* ML_NPU_SEMAPHORE_TIMEOUT */
        res = cy_rtos_get_semaphore(handle, timeout, false);
    }

    return res;
}

/**
 * \brief : Semaphore destroy (freeing memory) using API of RTOS abstraction.
 *
 * \param[in]   sem  : semaphore handle to be unlocked
 */
void mtb_ml_sem_destroy(void *sem)
{
    if (sem != NULL)
    {
        if (cy_rtos_deinit_semaphore(sem) == CY_RSLT_SUCCESS)
        {
            free(sem);
        }
    }
}

#endif /* COMPONENT_U55 || COMPONENT_NNLITE2 */
#endif /* CY_RTOS_AWARE */
