// ============================================================
// effect_cathedral_reverb.c - PC Platform Version
// ============================================================

#include <stdio.h>
#include <stdlib.h>
#include "effect_cathedral_reverb.h"

// ==================== PC Platform Memory Simulation ====================
// Use static array to simulate embedded system SDRAM
static float FxCathedralRevbMemory[FX_CATHEDRAL_REVB_RAM_SIZE_FLOATS];

// ==================== User Adjustable Parameters ====================
float FxCathedralRevbDecaySet;
float FxCathedralRevbToneSet;

// Input LP
float FxCathedralRevbInputLpB1   = 0.52071842f;
float FxCathedralRevbInputLpA0   = 0.47928158f;

// Reverb
float FxCathedralRevbDecayDampB1 = 0.81529625f;
float FxCathedralRevbDecayDampA0 = 0.09f;
float FxCathedralRevbDecayDampC  = 0.76f;

// Parameter Change
float FxCathedralRevbParaDezYn   = 0.0f;

// ER Gains
float FxCathedralEarlyGains[32];

// ==================== Buffer and Read/Write Pointer ====================
// PC Platform Modification: Use indices instead of pointers
uint32_t FxCathedralRevbReadIndices[FX_CATHEDRAL_REVB_READ_BUFFER_SIZE];
uint32_t FxCathedralRevbWriteIndices[FX_CATHEDRAL_REVB_WRITE_BUFFER_SIZE];
uint32_t FxCathedralRevbReadPtr = 0;   // Current read pointer index
uint32_t FxCathedralRevbWritePtr = 0;  // Current write pointer index
uint32_t FxCathedralRevbMemOffsetPtr = 0;
uint32_t FxCathedralRevbReadWriteBaseAddr[FX_CATHEDRAL_REVB_RE_WR_BUFFER_SIZE];

// ==================== States ====================
float FxCathedralRevbInLpYnL    = 0.0f;
float FxCathedralRevbInLpYnR    = 0.0f;
float FxCathedralRevbInHpYnL    = 0.0f;
float FxCathedralRevbInHpYnR    = 0.0f;

float FxCathedralRevbDiffOutL   = 0.0f;
float FxCathedralRevbDiffOutR   = 0.0f;
float FxCathedralRevbCmb1InBuff = 0.0f;

float FxCathedralRevbLfo        = 0.0f;

float FxCathedralRevbCombModBuff[8];
float FxCathedralRevbCombModFrac[8];

float FxCathedralRevbCombInBuff[8];
float FxCathedralRevbDecayYn[8];

float FxCathedralRevbCombModDelay[8] = {
    -FX_CATHEDRAL_REVB_COMB_MODULATION_BASE,
     FX_CATHEDRAL_REVB_COMB_MODULATION_BASE,
    -FX_CATHEDRAL_REVB_COMB_MODULATION_BASE,
     FX_CATHEDRAL_REVB_COMB_MODULATION_BASE,
    -FX_CATHEDRAL_REVB_COMB_MODULATION_BASE,
     FX_CATHEDRAL_REVB_COMB_MODULATION_BASE,
    -FX_CATHEDRAL_REVB_COMB_MODULATION_BASE,
     FX_CATHEDRAL_REVB_COMB_MODULATION_BASE
};

uint32_t FxCathedralRevbCombDelayBase[8] = {
    (0x24000/4) - 2593,
    (0x28000/4) - 3036,
    (0x2C000/4) - 3358,
    (0x30000/4) - 3756,
    (0x34000/4) - 2775,
    (0x38000/4) - 3050,
    (0x3C000/4) - 3490,
    (0x40000/4) - 4057
};

// ==================== PC Platform Helper Functions ====================
// Clear memory
void FxCathedralRevbClearMemory(void) {
    memset(FxCathedralRevbMemory, 0, sizeof(FxCathedralRevbMemory));
}

// Print status information
void FxCathedralRevbPrintStatus(void) {
    printf("Cathedral Reverb Status:\n");
    printf("  Decay: %.2f\n", FxCathedralRevbDecaySet);
    printf("  Tone: %.2f\n", FxCathedralRevbToneSet);
    printf("  Memory Offset: %u\n", FxCathedralRevbMemOffsetPtr);
    printf("  LFO: %.4f\n", FxCathedralRevbLfo);
}

// Memory read/write helper functions
static inline float ReadFromMemory(uint32_t index) {
    if (index < FX_CATHEDRAL_REVB_RAM_SIZE_FLOATS) {
        return FxCathedralRevbMemory[index];
    }
    return 0.0f;
}

static inline void WriteToMemory(uint32_t index, float value) {
    if (index < FX_CATHEDRAL_REVB_RAM_SIZE_FLOATS) {
        FxCathedralRevbMemory[index] = value;
    }
}

// ==================== Initialization Function ====================
void FxCathedralRevbInit(float fDecay, float fTone) {
    float tempFloat, tempDecay;
    
    // Clear memory
    FxCathedralRevbClearMemory();
    
    FxCathedralRevbParaDezYn = (1.0f - FX_CATHEDRAL_REVB_DEZAP_A0) * FxCathedralRevbParaDezYn + 
                               FX_CATHEDRAL_REVB_DEZAP_A0 * FxCathedralRevbDecaySet;
    
    tempDecay = FX_CATHEDRAL_REVB_PARA_CAL_A1 + FX_CATHEDRAL_REVB_PARA_CAL_B1 * FxCathedralRevbParaDezYn;
    
    // Dark / Bright
    tempFloat = FX_CATHEDRAL_REVB_PARA_DARK + (FX_CATHEDRAL_REVB_PARA_BRIGHT - FX_CATHEDRAL_REVB_PARA_DARK) * FxCathedralRevbToneSet;
    
    FxCathedralRevbInputLpB1 = tempFloat;
    FxCathedralRevbInputLpA0 = 0.5f * (1.0f - tempFloat);
    
    // Decay / Damping
    tempFloat = FX_CATHEDRAL_REVB_PARA_DAMP15 + (FX_CATHEDRAL_REVB_PARA_DAMP10 - FX_CATHEDRAL_REVB_PARA_DAMP15) * FxCathedralRevbToneSet;
    
    FxCathedralRevbDecayDampC  = tempDecay * tempFloat;
    FxCathedralRevbDecayDampA0 = FX_CATHEDRAL_REVB_PARA_CAL_A0 * tempDecay * (1.0f - tempFloat);
    
    // ER Gains
    FxCathedralEarlyGains[0]  =  0.1104f;
    FxCathedralEarlyGains[1]  =  0.0751f;
    FxCathedralEarlyGains[2]  = -0.0563f;
    FxCathedralEarlyGains[3]  =  0.1843f;
    FxCathedralEarlyGains[4]  =  0.1403f;
    FxCathedralEarlyGains[5]  = -0.0429f;
    FxCathedralEarlyGains[6]  =  0.0584f;
    FxCathedralEarlyGains[7]  = -0.0179f;
    FxCathedralEarlyGains[8]  =  0.0421f;
    FxCathedralEarlyGains[9]  =  0.0000f;
    FxCathedralEarlyGains[10] =  0.0904f;
    FxCathedralEarlyGains[11] = -0.0276f;
    FxCathedralEarlyGains[12] = -0.0252f;
    FxCathedralEarlyGains[13] =  0.0823f;
    FxCathedralEarlyGains[14] =  0.0000f;
    FxCathedralEarlyGains[15] =  0.0480f;
    FxCathedralEarlyGains[16] = -0.0140f;
    FxCathedralEarlyGains[17] =  0.0458f;
    FxCathedralEarlyGains[18] = -0.0125f;
    FxCathedralEarlyGains[19] =  0.0408f;
    FxCathedralEarlyGains[20] =  0.0000f;
    FxCathedralEarlyGains[21] =  0.0303f;
    FxCathedralEarlyGains[22] =  0.0153f;
    FxCathedralEarlyGains[23] =  0.0000f;
    FxCathedralEarlyGains[24] = -0.0111f;
    FxCathedralEarlyGains[25] =  0.0364f;
    FxCathedralEarlyGains[26] =  0.0293f;
    FxCathedralEarlyGains[27] = -0.0090f;
    FxCathedralEarlyGains[28] =  0.0108f;
    FxCathedralEarlyGains[29] =  0.0000f;
    FxCathedralEarlyGains[30] =  0.0233f;
    FxCathedralEarlyGains[31] = -0.0071f;
    
    // Initialize base addresses (converted to float indices)
    FxCathedralRevbReadWriteBaseAddr[0]  = 0x18BB0 / 4;
    FxCathedralRevbReadWriteBaseAddr[1]  = 0x19A74 / 4;
    FxCathedralRevbReadWriteBaseAddr[2]  = 0x092FC / 4;
    FxCathedralRevbReadWriteBaseAddr[3]  = 0x152FC / 4;
    FxCathedralRevbReadWriteBaseAddr[4]  = 0x1A93C / 4;
    FxCathedralRevbReadWriteBaseAddr[5]  = 0x1B800 / 4;
    FxCathedralRevbReadWriteBaseAddr[6]  = 0x1C6C8 / 4;
    FxCathedralRevbReadWriteBaseAddr[7]  = 0x1D58C / 4;
    FxCathedralRevbReadWriteBaseAddr[8]  = 0x1E454 / 4;
    FxCathedralRevbReadWriteBaseAddr[9]  = 0x1F1DC / 4;
    FxCathedralRevbReadWriteBaseAddr[10] = (0x24000 / 4) - 2;
    FxCathedralRevbReadWriteBaseAddr[11] = (0x24000 / 4) - 1;
    FxCathedralRevbReadWriteBaseAddr[12] = (0x28000 / 4) - 2;
    FxCathedralRevbReadWriteBaseAddr[13] = (0x28000 / 4) - 1;
    FxCathedralRevbReadWriteBaseAddr[14] = (0x2C000 / 4) - 2;
    FxCathedralRevbReadWriteBaseAddr[15] = (0x2C000 / 4) - 1;
    FxCathedralRevbReadWriteBaseAddr[16] = (0x30000 / 4) - 2;
    FxCathedralRevbReadWriteBaseAddr[17] = (0x30000 / 4) - 1;
    FxCathedralRevbReadWriteBaseAddr[18] = (0x34000 / 4) - 2;
    FxCathedralRevbReadWriteBaseAddr[19] = (0x34000 / 4) - 1;
    FxCathedralRevbReadWriteBaseAddr[20] = (0x38000 / 4) - 2;
    FxCathedralRevbReadWriteBaseAddr[21] = (0x38000 / 4) - 1;
    FxCathedralRevbReadWriteBaseAddr[22] = (0x3C000 / 4) - 2;
    FxCathedralRevbReadWriteBaseAddr[23] = (0x3C000 / 4) - 1;
    FxCathedralRevbReadWriteBaseAddr[24] = (0x40000 / 4) - 2;
    FxCathedralRevbReadWriteBaseAddr[25] = (0x40000 / 4) - 1;
    FxCathedralRevbReadWriteBaseAddr[26] = 0x0AAB8 / 4;
    FxCathedralRevbReadWriteBaseAddr[27] = 0x15B08 / 4;
    FxCathedralRevbReadWriteBaseAddr[28] = 0x09080 / 4;
    FxCathedralRevbReadWriteBaseAddr[29] = 0x14E70 / 4;
    FxCathedralRevbReadWriteBaseAddr[30] = 0x07298 / 4;
    FxCathedralRevbReadWriteBaseAddr[31] = 0x11CA8 / 4;
    FxCathedralRevbReadWriteBaseAddr[32] = 0x04D00 / 4;
    FxCathedralRevbReadWriteBaseAddr[33] = 0x10970 / 4;
    FxCathedralRevbReadWriteBaseAddr[34] = 0x03810 / 4;
    FxCathedralRevbReadWriteBaseAddr[35] = 0x0E1A8 / 4;
    FxCathedralRevbReadWriteBaseAddr[36] = 0x01DB0 / 4;
    FxCathedralRevbReadWriteBaseAddr[37] = 0x0D770 / 4;
    FxCathedralRevbReadWriteBaseAddr[38] = 0x01568 / 4;
    FxCathedralRevbReadWriteBaseAddr[39] = 0x0CC00 / 4;
    FxCathedralRevbReadWriteBaseAddr[40] = 0x006C8 / 4;
    FxCathedralRevbReadWriteBaseAddr[41] = 0x0C2D8 / 4;
    FxCathedralRevbReadWriteBaseAddr[42] = (0x41000 / 4) - 200;
    FxCathedralRevbReadWriteBaseAddr[43] = (0x42000 / 4) - 220;
    FxCathedralRevbReadWriteBaseAddr[44] = (0x43000 / 4) - 182;
    FxCathedralRevbReadWriteBaseAddr[45] = (0x44000 / 4) - 194;
    FxCathedralRevbReadWriteBaseAddr[46] = (0x24000 / 4) - 2;
    FxCathedralRevbReadWriteBaseAddr[47] = (0x34000 / 4) - 2;
    
    // Write addresses
    FxCathedralRevbReadWriteBaseAddr[48] = 0x0C000 / 4;
    FxCathedralRevbReadWriteBaseAddr[49] = 0x18000 / 4;
    FxCathedralRevbReadWriteBaseAddr[50] = 0x19000 / 4;
    FxCathedralRevbReadWriteBaseAddr[51] = 0x1A000 / 4;
    FxCathedralRevbReadWriteBaseAddr[52] = 0x1B000 / 4;
    FxCathedralRevbReadWriteBaseAddr[53] = 0x1C000 / 4;
    FxCathedralRevbReadWriteBaseAddr[54] = 0x1D000 / 4;
    FxCathedralRevbReadWriteBaseAddr[55] = 0x1E000 / 4;
    FxCathedralRevbReadWriteBaseAddr[56] = 0x1F000 / 4;
    FxCathedralRevbReadWriteBaseAddr[57] = 0x20000 / 4;
    FxCathedralRevbReadWriteBaseAddr[58] = 0x24000 / 4;
    FxCathedralRevbReadWriteBaseAddr[59] = 0x28000 / 4;
    FxCathedralRevbReadWriteBaseAddr[60] = 0x2C000 / 4;
    FxCathedralRevbReadWriteBaseAddr[61] = 0x30000 / 4;
    FxCathedralRevbReadWriteBaseAddr[62] = 0x34000 / 4;
    FxCathedralRevbReadWriteBaseAddr[63] = 0x38000 / 4;
    FxCathedralRevbReadWriteBaseAddr[64] = 0x3C000 / 4;
    FxCathedralRevbReadWriteBaseAddr[65] = 0x40000 / 4;
    FxCathedralRevbReadWriteBaseAddr[66] = 0x41000 / 4;
    FxCathedralRevbReadWriteBaseAddr[67] = 0x42000 / 4;
    FxCathedralRevbReadWriteBaseAddr[68] = 0x43000 / 4;
    FxCathedralRevbReadWriteBaseAddr[69] = 0x44000 / 4;
    
    FxCathedralRevbDecaySet = fDecay;
    FxCathedralRevbToneSet = fTone;
}

// ==================== Parameter Update ====================
void FxCathedralRevbParaUpdate(float fDecay, float fTone) {
    FxCathedralRevbDecaySet = fDecay;
    FxCathedralRevbToneSet = fTone;
}

// ==================== Parameter Smoothing ====================
void FxCathedralRevbParaDezap(void) {
    float tempFloat, tempDecay;
    
    FxCathedralRevbParaDezYn = (1.0f - FX_CATHEDRAL_REVB_DEZAP_A0) * FxCathedralRevbParaDezYn + 
                               FX_CATHEDRAL_REVB_DEZAP_A0 * FxCathedralRevbDecaySet;
    
    tempDecay = FX_CATHEDRAL_REVB_PARA_CAL_A1 + FX_CATHEDRAL_REVB_PARA_CAL_B1 * FxCathedralRevbParaDezYn;
    
    // Dark / Bright
    tempFloat = FX_CATHEDRAL_REVB_PARA_DARK + (FX_CATHEDRAL_REVB_PARA_BRIGHT - FX_CATHEDRAL_REVB_PARA_DARK) * FxCathedralRevbToneSet;
    
    FxCathedralRevbInputLpB1 = tempFloat;
    FxCathedralRevbInputLpA0 = 0.5f * (1.0f - tempFloat);
    
    // Decay / Damping
    tempFloat = FX_CATHEDRAL_REVB_PARA_DAMP15 + (FX_CATHEDRAL_REVB_PARA_DAMP10 - FX_CATHEDRAL_REVB_PARA_DAMP15) * FxCathedralRevbToneSet;
    
    FxCathedralRevbDecayDampC = tempDecay * tempFloat;
    FxCathedralRevbDecayDampA0 = FX_CATHEDRAL_REVB_PARA_CAL_A0 * tempDecay * (1.0f - tempFloat);
}

// ==================== Main Processing Function ====================
void FxCathedralRevbProcess(float fXnL, float fXnR, float *OutL, float *OutR) {
    FxCathedralRevbParaDezap();
    FxCathedralRevbUpdateAddTable();
    FxCathedralRevbInputFilter(fXnL, fXnR);
    FxCathedralRevbDiffusor();
    FxCathedralRevbModulation();
    FxCathedralRevbDecayAndMix();
    FxCathedralRevbEarlyRefAndOut(OutL, OutR);
}

// ==================== Update Address Table ====================
void FxCathedralRevbUpdateAddTable(void) {
    uint32_t i, Offset;
    
    // Update memory offset (circular buffer)
    Offset = FxCathedralRevbMemOffsetPtr + 1;  // PC platform uses float units
    FxCathedralRevbMemOffsetPtr = (Offset >= (0x80000 / 4)) ? (Offset - (0x80000 / 4)) : Offset;
    
    // Update read/write indices
    for (i = 0; i < FX_CATHEDRAL_REVB_READ_BUFFER_SIZE; i++) {
        Offset = FxCathedralRevbReadWriteBaseAddr[i] + FxCathedralRevbMemOffsetPtr;
        FxCathedralRevbReadIndices[i] = Offset % (0x80000 / 4);
    }
    
    for (i = 0; i < FX_CATHEDRAL_REVB_WRITE_BUFFER_SIZE; i++) {
        Offset = FxCathedralRevbReadWriteBaseAddr[FX_CATHEDRAL_REVB_READ_BUFFER_SIZE + i] + FxCathedralRevbMemOffsetPtr;
        FxCathedralRevbWriteIndices[i] = Offset % (0x80000 / 4);
    }
    
    // Reset pointers
    FxCathedralRevbReadPtr = 0;
    FxCathedralRevbWritePtr = 0;
}

// ==================== Input Filter ====================
void FxCathedralRevbInputFilter(float fXnL, float fXnR) {
    float YnL, YnR;
    
    // Low-pass filtering
    YnL = FxCathedralRevbInputLpA0 * fXnL + FxCathedralRevbInputLpB1 * FxCathedralRevbInLpYnL;
    YnR = FxCathedralRevbInputLpA0 * fXnR + FxCathedralRevbInputLpB1 * FxCathedralRevbInLpYnR;
    
    // High-pass filtering
    FxCathedralRevbInHpYnL = FX_CATHEDRAL_REVB_IN_HP_A1 * FxCathedralRevbInLpYnL + 
                             FX_CATHEDRAL_REVB_IN_HP_A0 * YnL + 
                             FX_CATHEDRAL_REVB_IN_HP_B1 * FxCathedralRevbInHpYnL;
    FxCathedralRevbInHpYnR = FX_CATHEDRAL_REVB_IN_HP_A1 * FxCathedralRevbInLpYnR + 
                             FX_CATHEDRAL_REVB_IN_HP_A0 * YnR + 
                             FX_CATHEDRAL_REVB_IN_HP_B1 * FxCathedralRevbInHpYnR;
    
    FxCathedralRevbInLpYnL = YnL;
    FxCathedralRevbInLpYnR = YnR;
    
    // Write to memory
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], FxCathedralRevbInHpYnL);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], FxCathedralRevbInHpYnR);
}

// ==================== Diffuser ====================
void FxCathedralRevbDiffusor(void) {
    float XnL, XnR, YnL, YnR;
    
    // AP1
    XnL = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
    XnR = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
    
    YnL = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
    YnR = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
    
    YnL = YnL - FX_CATHEDRAL_REVB_DIFF_D * XnL;
    YnR = YnR - FX_CATHEDRAL_REVB_DIFF_D * XnR;
    YnL = MAX(MIN(YnL, 0.999999f), -0.999999f);
    YnR = MAX(MIN(YnR, 0.999999f), -0.999999f);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], YnL);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], YnR);
    
    XnL = XnL + FX_CATHEDRAL_REVB_DIFF_D * YnL;
    XnR = XnR + FX_CATHEDRAL_REVB_DIFF_D * YnR;
    
    // AP2
    YnL = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
    YnR = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
    
    XnL = XnL - FX_CATHEDRAL_REVB_DIFF_D * YnL;
    XnR = XnR - FX_CATHEDRAL_REVB_DIFF_D * YnR;
    XnL = MAX(MIN(XnL, 0.999999f), -0.999999f);
    XnR = MAX(MIN(XnR, 0.999999f), -0.999999f);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], XnL);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], XnR);
    
    YnL = YnL + FX_CATHEDRAL_REVB_DIFF_D * XnL;
    YnR = YnR + FX_CATHEDRAL_REVB_DIFF_D * XnR;
    
    // AP3
    XnL = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
    XnR = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
    
    YnL = YnL - FX_CATHEDRAL_REVB_DIFF_D * XnL;
    YnR = YnR - FX_CATHEDRAL_REVB_DIFF_D * XnR;
    YnL = MAX(MIN(YnL, 0.999999f), -0.999999f);
    YnR = MAX(MIN(YnR, 0.999999f), -0.999999f);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], YnL);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], YnR);
    
    XnL = XnL + FX_CATHEDRAL_REVB_DIFF_D * YnL;
    XnR = XnR + FX_CATHEDRAL_REVB_DIFF_D * YnR;
    
    // AP4
    YnL = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
    YnR = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
    
    XnL = XnL - FX_CATHEDRAL_REVB_DIFF_D * YnL;
    XnR = XnR - FX_CATHEDRAL_REVB_DIFF_D * YnR;
    XnL = MAX(MIN(XnL, 0.999999f), -0.999999f);
    XnR = MAX(MIN(XnR, 0.999999f), -0.999999f);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], XnL);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], XnR);
    
    FxCathedralRevbDiffOutL = YnL + FX_CATHEDRAL_REVB_DIFF_D * XnL;
    FxCathedralRevbDiffOutR = YnR + FX_CATHEDRAL_REVB_DIFF_D * XnR;
}

// ==================== Modulation ====================
void FxCathedralRevbModulation(void) {
    float fTRI, fracTemp, Xn1, Xn2, Yn;
    uint32_t IntYn, Add;
    
    // LFO
    FxCathedralRevbLfo = FxCathedralRevbLfo + FX_CATHEDRAL_REVB_LFO_DELTA;
    FxCathedralRevbLfo = (FxCathedralRevbLfo >= 2.0f) ? (FxCathedralRevbLfo - 2.0f) : FxCathedralRevbLfo;
    
    fTRI = (FxCathedralRevbLfo > 1.0f) ? (2.0f - FxCathedralRevbLfo) : FxCathedralRevbLfo;
    
    for(uint8_t i = 0; i < 8; i++) {
        fracTemp = FxCathedralRevbCombModFrac[i];
        
        Yn = (float)(FxCathedralRevbCombDelayBase[i]) + FxCathedralRevbCombModDelay[i] * fTRI;
        
        IntYn = (uint32_t)Yn;
        FxCathedralRevbCombModFrac[i] = Yn - (float)IntYn;
        
        // Update modulation address
        Add = IntYn;
        FxCathedralRevbReadWriteBaseAddr[10 + 2*i] = Add;
        FxCathedralRevbReadWriteBaseAddr[11 + 2*i] = Add + 1;
        
        // Linear interpolation
        Xn2 = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
        Xn1 = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
        
        FxCathedralRevbCombModBuff[i] = (1.0f - fracTemp) * Xn2 + fracTemp * Xn1;
    }
}

// ==================== Decay and Mix ====================
void FxCathedralRevbDecayAndMix(void) {
    float tCmbSum, XnL, XnR;
    
    tCmbSum = 0.0f;
    
    for(uint8_t i = 0; i < 8; i++) {
        FxCathedralRevbDecayYn[i] = FxCathedralRevbDecayDampB1 * FxCathedralRevbDecayYn[i] + 
                                    FxCathedralRevbDecayDampA0 * FxCathedralRevbCombModBuff[i];
        
        FxCathedralRevbCombInBuff[i] = FxCathedralRevbDecayYn[i] + 
                                       FxCathedralRevbDecayDampC * FxCathedralRevbCombModBuff[i];
        
        tCmbSum += FX_CATHEDRAL_REVB_COMB_CROSSFEEDBACK * FxCathedralRevbCombInBuff[i];
    }
    
    XnL = tCmbSum + FX_CATHEDRAL_REVB_COMB_INPUT_GN * FxCathedralRevbDiffOutL;
    XnR = tCmbSum + FX_CATHEDRAL_REVB_COMB_INPUT_GN * FxCathedralRevbDiffOutR;
    
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], FxCathedralRevbCmb1InBuff);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], FxCathedralRevbCombInBuff[0] - XnL);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], FxCathedralRevbCombInBuff[1] - XnL);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], FxCathedralRevbCombInBuff[2] - XnL);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], FxCathedralRevbCombInBuff[3] - XnL);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], FxCathedralRevbCombInBuff[4] - XnR);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], FxCathedralRevbCombInBuff[5] - XnR);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], FxCathedralRevbCombInBuff[6] - XnR);
    
    FxCathedralRevbCmb1InBuff = FxCathedralRevbCombInBuff[7] - XnR;
}

// ==================== Early Reflections and Output ====================
void FxCathedralRevbEarlyRefAndOut(float *OutL, float *OutR) {
    float YnL, YnR, XnL, XnR;
    float temp;
    
    // Early reflection mixing
    YnL = 0.0f;
    YnR = 0.0f;
    
    for(int i = 0; i < 16; i++) {
        temp = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
        YnL += FxCathedralEarlyGains[i*2] * temp;
        YnR += FxCathedralEarlyGains[i*2+1] * temp;
    }
    
    // Early reflection diffuser
    XnL = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
    XnR = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
    
    YnL -= FX_CATHEDRAL_REVB_EARLY_DIFF_D * XnL;
    YnR -= FX_CATHEDRAL_REVB_EARLY_DIFF_D * XnR;
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], YnL);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], YnR);
    
    YnL = XnL + FX_CATHEDRAL_REVB_EARLY_DIFF_D * YnL;
    YnR = XnR + FX_CATHEDRAL_REVB_EARLY_DIFF_D * YnR;
    
    XnL = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
    XnR = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
    
    YnL -= FX_CATHEDRAL_REVB_EARLY_DIFF_D * XnL;
    YnR -= FX_CATHEDRAL_REVB_EARLY_DIFF_D * XnR;
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], YnL);
    WriteToMemory(FxCathedralRevbWriteIndices[FxCathedralRevbWritePtr++], YnR);
    
    YnL = 2.0f * (XnL + FX_CATHEDRAL_REVB_EARLY_DIFF_D * YnL);
    YnR = 2.0f * (XnR + FX_CATHEDRAL_REVB_EARLY_DIFF_D * YnR);
    
    // Final output mixing
    XnL = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr++]);
    XnR = ReadFromMemory(FxCathedralRevbReadIndices[FxCathedralRevbReadPtr]);
    
    YnL = 2.0f * (FX_CATHEDRAL_REVB_OUT_ER_GAIN * YnL + FX_CATHEDRAL_REVB_OUT_REV_GAIN * XnL);
    YnR = 2.0f * (FX_CATHEDRAL_REVB_OUT_ER_GAIN * YnR + FX_CATHEDRAL_REVB_OUT_REV_GAIN * XnR);
    
    YnL = MAX(MIN(YnL, 0.999999f), -0.999999f);
    YnR = MAX(MIN(YnR, 0.999999f), -0.999999f);
    
    *OutL = YnL;
    *OutR = YnR;
}