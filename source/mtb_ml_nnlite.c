/******************************************************************************
* File Name: mtb_ml_nnlite.c
*
* Description: This file contains overloads for the weak implementations of
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
#if defined(COMPONENT_NNLITE2)

#include "cy_pdl.h"
#include <stdlib.h>
#include "mtb_ml.h"
#include "cy_nnlite.h"
#include "cy_nn_kernel.h"

#if defined(CY_RTOS_AWARE)
#include "cyabs_rtos.h"
#include "cyabs_rtos_impl.h"
#endif

cy_kernel_config_t cy_kernel_config = {0};

__WEAK void Cy_NNLite_Lpm_Lock(void) {
    /* To be substituted at application level */
}

__WEAK void Cy_NNLite_Lpm_Unlock(void) {
    /* To be substituted at application level */
}

/*******************************************************************************
 * Private Functions
*******************************************************************************/

#if defined(CY_RTOS_AWARE)
/******************************************************************************
 * Semaphore/Mutex function overrides for NNLite NPU
 *
 * Overwrite the weak default implementations for NPU driver
 * with real mutex and semaphore implementations.
 ******************************************************************************/
cy_mutex_t nnliteMutex;
cy_semaphore_t nnliteSem;

/**
 * \brief : Initialize mutex using API of RTOS abstraction.
 *
 * \param[in]  mutex : mutex handle to be initialized
 *
 * \return  res : result of operation
 */
uint32_t Cy_NNLite_Mutex_Create(void *mutex)
{
    cy_mutex_t *handle = (cy_mutex_t *)mutex;
    cy_rslt_t res = cy_rtos_init_mutex2(handle, false);
    return (uint32_t)res;
}

/**
 * \brief : Lock mutex using API of RTOS abstraction.
 *
 * \param[in]  mutex : mutex handle to be locked
 *
 * \return  res : result of operation
 */
uint32_t Cy_NNLite_Mutex_Lock(void *mutex)
{
    cy_mutex_t *handle = (cy_mutex_t *)mutex;
    cy_rslt_t res = cy_rtos_get_mutex(handle, ML_NPU_MUTEX_TIMEOUT);
    return (uint32_t)res;
}

/**
 * \brief : Unlock mutex using API of RTOS abstraction.
 *
 * \param[in]  mutex : mutex handle to be unlocked
 *
 * \return : res : result of operation
 */
uint32_t Cy_NNLite_Mutex_Unlock(void *mutex)
{
    cy_mutex_t *handle = (cy_mutex_t *)mutex;
    cy_rslt_t res = cy_rtos_set_mutex(handle);
    return (uint32_t)res;
}

/**
 * \brief : Delete NNLite mutex using API of RTOS abstraction.
 *
 * \param[in]  mutex : NNLite mutex handle to delete
 *
 * \return  res : result of operation
 */
uint32_t Cy_NNLite_Mutex_Delete(void *mutex)
{
    cy_mutex_t *handle = (cy_mutex_t *)mutex;
    cy_rslt_t res = cy_rtos_mutex_deinit(handle);
    return (uint32_t)res;
}

/**
 * \brief : Initialize semaphore using API of RTOS abstraction.
 *
 * \param[in]  sem : semaphore handle to be initialized
 *
 * \return  res : result of operation
 */
uint32_t Cy_NNLite_Sem_Create(void *sem)
{
    cy_semaphore_t *handle = (cy_semaphore_t *)sem;
    cy_rslt_t res = cy_rtos_init_semaphore(handle, configNPU_COUNT, 0);
    return (uint32_t)res;
}

/**
 * \brief : Take semaphore using API of RTOS abstraction.
 *
 * \param[in]  sem : semaphore handle to take
 *
 * \return  res : result of operation
 */
uint32_t Cy_NNLite_Sem_Wait(void *sem)
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
        res = cy_rtos_get_semaphore(handle, ML_NPU_SEMAPHORE_TIMEOUT, false);
    }
    return (uint32_t)res;
}

/**
 * \brief : Give semaphore using API of RTOS abstraction.
 *
 * \param[in]  sem : semaphore handle to give
 *
 * \return : res: result of operation
 */
uint32_t Cy_NNLite_Sem_Give(void *sem)
{
    cy_semaphore_t *handle = (cy_semaphore_t *)sem;
    cy_rslt_t res = cy_rtos_set_semaphore(handle, 0 != __get_IPSR());
    return (uint32_t)res;
}

/**
 * \brief : Delete semaphore using API of RTOS abstraction.
 *
 * \param[in]  sem : semaphore handle to delete
 *  *
 * \return  res : result of operation
 */
uint32_t Cy_NNLite_Sem_Delete(void *sem)
{
    cy_semaphore_t *handle = (cy_semaphore_t *)sem;
    cy_rslt_t res = cy_rtos_semaphore_deinit(handle);
    return (uint32_t)res;
}

#else /* defined(CY_RTOS_AWARE) */
static volatile uint32_t cy_kernel_operation_progress = 1;
/**
 * \brief   : Create mutex and return handle using API of RTOS abstraction.
 *
 * \return  : Mutex handle
 */
uint32_t Cy_NNLite_Mutex_Create(void *mutex)
{
    (void)mutex;
    return CY_NNLITE_SUCCESS;
}
/**
 * \brief : Lock mutex using API of RTOS abstraction.
 *
 * \param[in]   mutex  : mutex handle to be locked
 */
uint32_t Cy_NNLite_Mutex_Lock(void *mutex)
{
    (void)mutex;
    return CY_NNLITE_SUCCESS;
}
/**
 * \brief : Unlock mutex using API of RTOS abstraction.
 *
 * \param[in]   mutex  : mutex handle to be unlocked
 */
uint32_t Cy_NNLite_Mutex_Unlock(void *mutex)
{
    (void)mutex;
    return CY_NNLITE_SUCCESS;
}
/**
 * \brief : Delete NNLite mutex using API of RTOS abstraction.
 *
 * \param[in]   mutex  : NNLite mutex handle to delete
 */
uint32_t Cy_NNLite_Mutex_Delete(void *mutex)
{
    (void)mutex;
    return CY_NNLITE_SUCCESS;
}
/**
 * \brief   : Create semaphore and return handle using API of RTOS abstraction.
 *
 * \return  : Semaphore handle
 */
uint32_t Cy_NNLite_Sem_Create(void *sem)
{
    (void)sem;
    return CY_NNLITE_SUCCESS;
}
/**
 * \brief : Take semaphore using API of RTOS abstraction.
 *
 * \param[in]   sem  : semaphore handle to take
 */
uint32_t Cy_NNLite_Sem_Wait(void *sem)
{
    (void)sem;
    /* volatile flag signalling NPU has done its work
    * cleared from ISR */
    do {
    } while(cy_kernel_operation_progress);
    cy_kernel_operation_progress = 1;
    return CY_NNLITE_SUCCESS;
}
/**
 * \brief : Give NNLite semaphore using API of RTOS abstraction.
 *
 * \param[in]   sem  : NNLite semaphore handle to give
 */
uint32_t Cy_NNLite_Sem_Give(void *sem)
{
    (void)sem;
    /* signalling with volatile flag that NPU is done
    * Cy_NNLite_Sem_Give() is called from ISR */
    cy_kernel_operation_progress = 0;
    return CY_NNLITE_SUCCESS;
}
/**
 * \brief : Delete NNLite semaphore using API of RTOS abstraction.
 *
 * \param[in]   mutex  : NNLite semaphore handle to delete
 */
uint32_t Cy_NNLite_Sem_Delete(void *sem)
{
    (void)sem;
    return CY_NNLITE_SUCCESS;
}
#endif  /* defined(CY_RTOS_AWARE) */

/* NNLite NPU-related cycles count */
static uint64_t mtb_ml_nnlite_frame_start;
extern uint64_t mtb_ml_npu_cycles;

void mtb_ml_nnlite_nop(void *ptr) {
  (void)ptr;
}

/* This one stores _Start() point and accumulate total NPU time */
void mtb_ml_nnlite_prof_get(void *ptr, uint32_t profilePoint)
{
  (void)ptr;
  uint64_t nnlite_prof_tmp;
  /* pre-Start() */
  if (profilePoint == CY_NNLITE_PP_ACCELERATOR_START)
  {
      mtb_ml_model_profile_get_tsc((uint64_t *)&mtb_ml_nnlite_frame_start);
  }
  /* NNLITE ISR */
  else if (profilePoint == CY_NNLITE_PP_ACCELERATOR_DONE)
  {
      mtb_ml_model_profile_get_tsc((uint64_t *)&nnlite_prof_tmp);
      /* accumulate pure NPU time in case kernel is composite - SoftMax/LayerNorm/etc */
      mtb_ml_npu_cycles += (nnlite_prof_tmp - mtb_ml_nnlite_frame_start);
  }
}

/**
 * \brief : Initializes NNLite Driver SW stack and HW
 *
 * \param[in]   priority  : NNLite interrupt priority
 */
cy_rslt_t mtb_ml_nnlite_init(uint32_t priority)
{
#if defined(CY_RTOS_AWARE)
    /* initialize with proper mutex/sema */
    cy_kernel_config.nnliteMutex        = &nnliteMutex;
    cy_kernel_config.nnliteSem          = &nnliteSem;
#else
    cy_kernel_config.nnliteMutex        = NULL; /* ignoring for baremetal */
    cy_kernel_config.nnliteSem          = NULL; /* ignoring for baremetal */
#endif
    cy_kernel_config.cbArg              = NULL;
    cy_kernel_config.completionCbFunc   = NULL;
    cy_kernel_config.mutexCreateFunc    = Cy_NNLite_Mutex_Create; /* rtos_mutex_init */
    cy_kernel_config.mutexDeleteFunc    = Cy_NNLite_Mutex_Delete;
    cy_kernel_config.mutexLockFunc      = Cy_NNLite_Mutex_Lock;
    cy_kernel_config.mutexUnlockFunc    = Cy_NNLite_Mutex_Unlock;
    cy_kernel_config.SemCreateFunc      = Cy_NNLite_Sem_Create; /* rtos_sem_init */
    cy_kernel_config.SemDeleteFunc      = Cy_NNLite_Sem_Delete;
    cy_kernel_config.SemWaitFunc        = Cy_NNLite_Sem_Wait;
    cy_kernel_config.SemGiveFunc        = Cy_NNLite_Sem_Give;
    cy_kernel_config.LpmLockFunc        = Cy_NNLite_Lpm_Lock;
    cy_kernel_config.LpmUnlockFunc      = Cy_NNLite_Lpm_Unlock;
    cy_kernel_config.profStart          = mtb_ml_nnlite_nop;
    cy_kernel_config.profStop           = mtb_ml_nnlite_nop;
    cy_kernel_config.profGetCount       = mtb_ml_nnlite_prof_get;
    cy_kernel_config.profArg            = NULL;
    if (Cy_NNLite_KernelInit(&cy_kernel_config) != CY_NNLITE_SUCCESS)
    {
        printf("Failed to initialize MXNNLITE driver");
        return MTB_ML_RESULT_NPU_INIT_ERROR;
    }
    /* Will rely on a Core clock time-base */
    mtb_ml_npu_clk_freq = (uint32_t)SystemCoreClock;
    return MTB_ML_RESULT_SUCCESS;
}

cy_rslt_t mtb_ml_nnlite_deinit(void)
{
    return Cy_NNLite_KernelDeInit();
}

#endif /* defined(COMPONENT_NNLITE2) */
