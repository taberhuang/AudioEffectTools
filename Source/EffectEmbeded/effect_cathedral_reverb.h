// effect_cathedral_reverb.H - PC Platform Version
#ifndef __EFFECT_CATHEDRAL_REVERB_H__
#define __EFFECT_CATHEDRAL_REVERB_H__

#include <stdint.h>
#include <string.h>
#include <math.h>

// ==================== Basic Definitions ====================
#define FX_CATHEDRAL_REVB_READ_BUFFER_SIZE      48
#define FX_CATHEDRAL_REVB_WRITE_BUFFER_SIZE     22
#define FX_CATHEDRAL_REVB_RE_WR_BUFFER_SIZE     (FX_CATHEDRAL_REVB_READ_BUFFER_SIZE + FX_CATHEDRAL_REVB_WRITE_BUFFER_SIZE)

// PC platform memory size definition - 512KB
#define FX_CATHEDRAL_REVB_RAM_SIZE              (512 * 1024)  // 512KB
#define FX_CATHEDRAL_REVB_RAM_SIZE_FLOATS       (FX_CATHEDRAL_REVB_RAM_SIZE / sizeof(float))  // In float units

// ==================== Parameter Definitions ====================
// Parameter Change
#define FX_CATHEDRAL_REVB_DEZAP_A0              0.001f
#define FX_CATHEDRAL_REVB_PARA_DAMP15           0.9012505f
#define FX_CATHEDRAL_REVB_PARA_DAMP10           0.9330330f
#define FX_CATHEDRAL_REVB_PARA_CAL_A1           0.91f
#define FX_CATHEDRAL_REVB_PARA_CAL_B1           0.078f
#define FX_CATHEDRAL_REVB_PARA_CAL_A0           0.18470375f
#define FX_CATHEDRAL_REVB_PARA_DARK             0.877306f           // Dark   1K
#define FX_CATHEDRAL_REVB_PARA_BRIGHT           0.519703f           // Bright 5K

// Input HP
#define FX_CATHEDRAL_REVB_IN_HP_A1              -0.98917424f
#define FX_CATHEDRAL_REVB_IN_HP_A0              0.98917424f
#define FX_CATHEDRAL_REVB_IN_HP_B1              0.97834849f

// Reverb Diffusor
#define FX_CATHEDRAL_REVB_DIFF_D                0.7f

// LFO
#define FX_CATHEDRAL_REVB_LFO_DELTA             0.000005f
#define FX_CATHEDRAL_REVB_COMB_MODULATION_BASE  299.0f

// Comb Input Mix
#define FX_CATHEDRAL_REVB_COMB_CROSSFEEDBACK    0.25f
#define FX_CATHEDRAL_REVB_COMB_INPUT_GN         0.25f

// ER Diffusor
#define FX_CATHEDRAL_REVB_EARLY_DIFF_D          0.8f

// Output Mixer
#define FX_CATHEDRAL_REVB_OUT_ER_GAIN           0.328f
#define FX_CATHEDRAL_REVB_OUT_REV_GAIN          0.492f

// PC platform compatibility macros
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

void FxCathedralRevbInit(float fDecay, float fTone);
void FxCathedralRevbParaUpdate(float fDecay, float fTone);
void FxCathedralRevbParaDezap(void);
void FxCathedralRevbProcess(float fXnL, float fXnR, float *OutL, float *OutR);
void FxCathedralRevbUpdateAddTable(void);
void FxCathedralRevbInputFilter(float fXnL, float fXnR);
void FxCathedralRevbDiffusor(void);
void FxCathedralRevbModulation(void);
void FxCathedralRevbDecayAndMix(void);
void FxCathedralRevbEarlyRefAndOut(float *OutL, float *OutR);

// PC platform test functions
void FxCathedralRevbClearMemory(void);
void FxCathedralRevbPrintStatus(void);

#ifdef __cplusplus
}
#endif

#endif // __EFFECT_CATHEDRAL_REVERB_H__