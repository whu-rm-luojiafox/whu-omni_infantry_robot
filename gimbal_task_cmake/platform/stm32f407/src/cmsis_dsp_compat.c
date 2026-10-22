/**
 * @file    cmsis_dsp_compat.c
 * @brief   Minimal CMSIS-DSP compatibility layer for the GCC build.
 *
 * The original Keil project linked arm_cortexM4lf_math.lib, which is an
 * ARM Compiler archive and cannot be used by arm-none-eabi-gcc. The
 * application only calls these two CMSIS-DSP functions, so they are
 * provided here on top of newlib's single-precision math routines.
 */

#include "arm_math.h"
#include <math.h>

float32_t arm_sin_f32(float32_t x)
{
    return sinf(x);
}

float32_t arm_cos_f32(float32_t x)
{
    return cosf(x);
}
