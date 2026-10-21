/******************************************************************************
* File Name: mtb_et_pal.cpp
*
* Description: ExecutorTorch Platform Abstraction Layer (PAL) implementation
*              for MTB ML middleware.
*
*******************************************************************************
* (c) 2026, Infineon Technologies AG, or
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

#include <cstdarg>
#include <cstdio>

#include "executorch/runtime/platform/platform.h"

extern "C" {
#include "cybsp.h"
}

static void convert_zu(char* out, size_t out_size,
                              const char* in) {
  /* Replace %zu with %u in format string */
  const char* p = in;
  size_t o = 0;

  while (*p && o + 2 < out_size) {
    if (p[0] == '%' && p[1] == 'z' && p[2] == 'u') {
      out[o++] = '%';
      out[o++] = 'u';
      p += 3;
    } else {
      out[o++] = *p++;
    }
  }
  out[o] = '\0';
}

extern "C" int __real_vsnprintf(char* buf, size_t size,
                                const char* fmt, va_list ap);

/* MTB's default printf-like functions come from newlib-nano, which does not support
 * the %zu format specifier used by executorch for logging. This wrapper function
 * converts %zu to %u before calling newlib's vsnprintf.
 * Note: Requires adding the --wrap=vsnprintf linker flag when building the project. */
extern "C" int __wrap_vsnprintf(char* buf, size_t size,
                                const char* fmt, va_list ap)
{
    char fixed_fmt[256];

    /* Replace %zu with %u in the format string to support newlib-nano */
    convert_zu(fixed_fmt, sizeof(fixed_fmt), fmt);

    /* Call newlib vsnprintf with the fixed format string */
    return __real_vsnprintf(buf, size, fixed_fmt, ap);
}

/*
 * Implementation of the et_pal_<funcs>()
 *
 * This functions are hardware adaption type of functions for things like
 * time/logging/memory allocation that could call your RTOS or need to to
 * be implemented in some way.
 */

/* Allow user to override this, e.g. to initialize UART redirection for early logging output
 * before reaching the main() function. */
__attribute__((weak)) void et_pal_init(void) {
}

ET_NORETURN void et_pal_abort(void) {
#if !defined(SEMIHOSTING)
  __builtin_trap();
#else
  _exit(-1);
#endif
}

/* Not used in current implementation, but required for PAL definition */
et_timestamp_t et_pal_current_ticks(void) {
    return 0;
}

/* Not used in current implementation, but required for PAL definition */
et_tick_ratio_t et_pal_ticks_to_ns_multiplier(void) {
  return {1, 1};
}

/*
 * Emit a log message via platform output (serial port, console, etc).
 */
__attribute__((weak)) void et_pal_emit_log_message(
    ET_UNUSED et_timestamp_t timestamp,
    et_pal_log_level_t level,
    const char* filename,
    ET_UNUSED const char* function,
    size_t line,
    const char* message,
    ET_UNUSED size_t length) {
#ifdef ET_LOG_DISABLED
  (void)timestamp;
  (void)level;
  (void)filename;
  (void)function;
  (void)line;
  (void)message;
  (void)length;
#else
  printf(
      "%c [executorch:%s:%u %s()] %s\r\n",
      level,
      filename,
      line,
      function,
      message);
#endif
}

/*
 * Dynamic memory allocators intended to be used by temp_allocator
 * to implement malloc()/free() type of allocations.
 * Currently not used.
 */

void* et_pal_allocate(ET_UNUSED size_t size) {
  return nullptr;
}

void et_pal_free(ET_UNUSED void* ptr) {}
