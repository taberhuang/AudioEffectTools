//effect_delay.h - PC Platform Version
#ifndef __EFFECT_DELAY_H__
#define __EFFECT_DELAY_H__

#include <stdint.h>
#include <string.h>
#include <math.h>

// ==================== Basic Definitions ====================
#define FX_DELAY_READ_BUFFER_SIZE       2
#define FX_DELAY_WRITE_BUFFER_SIZE      2
#define FX_DELAY_RE_WR_BUFFER_SIZE      (FX_DELAY_READ_BUFFER_SIZE + FX_DELAY_WRITE_BUFFER_SIZE)

// PC Platform Memory Size Definition - 512KB
#define FX_DELAY_RAM_SIZE               (512 * 1024)  // 512KB
#define FX_DELAY_RAM_SIZE_FLOATS        (FX_DELAY_RAM_SIZE / sizeof(float))  // in float units

// Delay Time Range
#define FX_DELAY_TIME_MIN               2400.0f         // ~50ms @ 48kHz
#define FX_DELAY_TIME_MAX               64800.0f        // ~1.35s @ 48kHz

// Base Addresses (converted to float indices)
#define FX_DELAY_BASE_ADD_L             (0x3FFFC / 4)   // Left channel base
#define FX_DELAY_BASE_ADD_R             (0x7FFFC / 4)   // Right channel base

// ==================== Parameter Definitions ====================
// Parameter Smoothing
#define FX_DELAY_DEZAP_A0               0.001f          // Smoothing coefficient

// Delay Module
#define FX_DELAY_FADE_DELTA             0.0003f         // Fade in/out speed
#define FX_DELAY_LP_B1                  0.995f          // Low-pass filter coefficient

// Input Mixer
#define FX_DELAY_GN_INPUT               0.707f          // Input gain (-3dB)

// PC Platform Compatibility Macros
#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


// ==================== Function Declarations ====================
#ifdef __cplusplus
extern "C" {
#endif

// ==================== Function Declarations ====================
void FxDelayInit(float fRepeat, float fTime);
void FxDelayParaUpdate(float fRepeat, float fTime);
void FxDelayParaDezap(void);
void FxDelayProcess(float fXnL, float fXnR, float *OutL, float *OutR);
void FxDelayUpdateAddTable(void);
void FxDelayCalcOutput(float fXnL, float fXnR, float *OutL, float *OutR);

// PC Platform Utility Functions
void FxDelayClearMemory(void);
void FxDelayPrintStatus(void);
float FxDelayGetCurrentDelayTime(void);
#ifdef __cplusplus
}
#endif

#endif // __EFFECT_DELAY_H__