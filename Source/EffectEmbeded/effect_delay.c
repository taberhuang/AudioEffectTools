// ============================================================
// effect_delay.c - PC Platform Version
// ============================================================

#include <stdio.h>
#include <stdlib.h>
#include "effect_delay.h"

// ==================== PC Platform Memory Simulation ====================
// Static array to simulate embedded system SDRAM
static float FxDelayMemory[FX_DELAY_RAM_SIZE_FLOATS];

// ==================== User Adjustable Parameters ====================
float FxDelayRepeatSet = 0.5f;      // Feedback amount (0.0~1.0)
float FxDelayTimeSet   = 0.5f;      // Delay time (0.0~1.0 mapped to 50ms~1.35s)

// Internal Parameters
float FxDelayGnFeed;                // Smoothed feedback gain
float FxDelayFadeCntr;               // Fade counter for delay time changes
float FxDelayTime;                   // Current delay time

// ==================== Buffer and Read/Write Indices ====================
// PC Platform modification: Use indices instead of pointers
uint32_t FxDelayReadIndices[FX_DELAY_READ_BUFFER_SIZE];
uint32_t FxDelayWriteIndices[FX_DELAY_WRITE_BUFFER_SIZE];
uint32_t FxDelayReadPtr = 0;        // Current read index pointer
uint32_t FxDelayWritePtr = 0;       // Current write index pointer
uint32_t FxDelayMemOffsetPtr = 0;   // Memory offset for circular buffer
uint32_t FxDelayReadWriteBaseAddr[FX_DELAY_RE_WR_BUFFER_SIZE];

// ==================== Internal States ====================
float FxDelayGainYn   = 0.0f;       // Low-pass filtered gain
float FxDelayOutL     = 0.0f;       // Left channel output
float FxDelayOutR     = 0.0f;       // Right channel output

// ==================== PC Platform Utility Functions ====================

/**
 * Clear the delay memory buffer
 */
void FxDelayClearMemory(void) {
    memset(FxDelayMemory, 0, sizeof(FxDelayMemory));
}

/**
 * Print current delay status for debugging
 */
void FxDelayPrintStatus(void) {
    printf("Delay Effect Status:\n");
    printf("  Repeat (Feedback): %.2f\n", FxDelayRepeatSet);
    printf("  Time Setting: %.2f\n", FxDelayTimeSet);
    printf("  Actual Delay: %.2f ms\n", FxDelayGetCurrentDelayTime());
    printf("  Memory Offset: %u\n", FxDelayMemOffsetPtr);
    printf("  Fade Counter: %.4f\n", FxDelayFadeCntr);
    printf("  Current Gain: %.4f\n", FxDelayGainYn);
}

/**
 * Get current delay time in milliseconds
 */
float FxDelayGetCurrentDelayTime(void) {
    float samples = (FX_DELAY_TIME_MAX - FX_DELAY_TIME_MIN) * FxDelayTimeSet + FX_DELAY_TIME_MIN;
    return (samples / 48000.0f) * 1000.0f; // Convert to ms assuming 48kHz
}

/**
 * Memory read helper function
 */
static inline float ReadFromMemory(uint32_t index) {
    if (index < FX_DELAY_RAM_SIZE_FLOATS) {
        return FxDelayMemory[index];
    }
    return 0.0f;
}

/**
 * Memory write helper function
 */
static inline void WriteToMemory(uint32_t index, float value) {
    if (index < FX_DELAY_RAM_SIZE_FLOATS) {
        FxDelayMemory[index] = value;
    }
}

// ==================== Main Functions ====================

/**
 * Initialize delay effect
 * @param fRepeat - Feedback amount (0.0 to 1.0)
 * @param fTime - Delay time (0.0 to 1.0, mapped to 50ms to 1.35s)
 */
void FxDelayInit(float fRepeat, float fTime) {
    // Clear memory buffer
    FxDelayClearMemory();
    
    // Initialize parameters
    FxDelayGnFeed   = 0.0f;
    FxDelayFadeCntr = 0.0f;
    FxDelayTime     = 0.0f;
    
    // Initialize base addresses (converted to float indices)
    // These are the initial read positions for the delay lines
    FxDelayReadWriteBaseAddr[0] = 0x3DA80 / 4;  // Left channel read
    FxDelayReadWriteBaseAddr[1] = 0x7DA80 / 4;  // Right channel read
    
    // Write positions
    FxDelayReadWriteBaseAddr[2] = FX_DELAY_BASE_ADD_L;  // Left channel write
    FxDelayReadWriteBaseAddr[3] = FX_DELAY_BASE_ADD_R;  // Right channel write
    
    // Set user parameters
    FxDelayRepeatSet = fRepeat;
    FxDelayTimeSet   = fTime;
    
    printf("Delay initialized: Repeat=%.2f, Time=%.2f (%.1f ms)\n", 
           fRepeat, fTime, FxDelayGetCurrentDelayTime());
}

/**
 * Update user parameters
 * @param fRepeat - New feedback amount (0.0 to 1.0)
 * @param fTime - New delay time (0.0 to 1.0)
 */
void FxDelayParaUpdate(float fRepeat, float fTime) {
    FxDelayRepeatSet = fRepeat;
    FxDelayTimeSet   = fTime;
}

/**
 * Smooth parameter changes to avoid clicks and pops
 * Called every sample to gradually update parameters
 */
void FxDelayParaDezap(void) {
    uint32_t AddOffset;
    
    // Smooth feedback gain changes
    FxDelayGnFeed = (1.0f - FX_DELAY_DEZAP_A0) * FxDelayGnFeed + 
                    FX_DELAY_DEZAP_A0 * FxDelayRepeatSet;
    
    // Check if delay time has changed
    if(FxDelayTime != FxDelayTimeSet) {
        // Reset fade counter to smoothly transition to new delay time
        FxDelayFadeCntr = 0;
        FxDelayTime = FxDelayTimeSet;
        
        // Calculate new delay offset in samples
        AddOffset = (uint32_t)((FX_DELAY_TIME_MAX - FX_DELAY_TIME_MIN) * FxDelayTime + FX_DELAY_TIME_MIN);
        AddOffset = AddOffset;  // Already in float units for PC version
        
        // Update read positions based on new delay time
        FxDelayReadWriteBaseAddr[0] = FX_DELAY_BASE_ADD_L - AddOffset;
        FxDelayReadWriteBaseAddr[1] = FX_DELAY_BASE_ADD_R - AddOffset;
    }
}

/**
 * Main delay processing function
 * @param fXnL - Input left channel
 * @param fXnR - Input right channel
 * @param OutL - Output left channel pointer
 * @param OutR - Output right channel pointer
 */
void FxDelayProcess(float fXnL, float fXnR, float *OutL, float *OutR) {
    // Step 1: Smooth parameter changes
    FxDelayParaDezap();
    
    // Step 2: Update memory address table
    FxDelayUpdateAddTable();
    
    // Step 3: Calculate delay output
    FxDelayCalcOutput(fXnL, fXnR, OutL, OutR);
}

/**
 * Update address table for circular buffer operation
 * Manages the circular buffer by updating read/write indices
 */
void FxDelayUpdateAddTable(void) {
    uint32_t i, Offset;
    
    // Increment circular buffer offset (in float units)
    Offset = FxDelayMemOffsetPtr + 1;
    FxDelayMemOffsetPtr = (Offset >= (0x80000 / 4)) ? (Offset - (0x80000 / 4)) : Offset;
    
    // Update read indices
    for (i = 0; i < FX_DELAY_READ_BUFFER_SIZE; i++) {
        Offset = FxDelayReadWriteBaseAddr[i] + FxDelayMemOffsetPtr;
        FxDelayReadIndices[i] = Offset % (0x80000 / 4);  // Wrap around
    }
    
    // Update write indices
    for (i = 0; i < FX_DELAY_WRITE_BUFFER_SIZE; i++) {
        Offset = FxDelayReadWriteBaseAddr[FX_DELAY_READ_BUFFER_SIZE + i] + FxDelayMemOffsetPtr;
        FxDelayWriteIndices[i] = Offset % (0x80000 / 4);  // Wrap around
    }
    
    // Reset index pointers
    FxDelayReadPtr = 0;
    FxDelayWritePtr = 0;
}

/**
 * Calculate delay output and write new input to delay line
 * @param fXnL - Input left channel
 * @param fXnR - Input right channel
 * @param OutL - Output left channel pointer
 * @param OutR - Output right channel pointer
 */
void FxDelayCalcOutput(float fXnL, float fXnR, float *OutL, float *OutR) {
    float tempXn;
    
    // Update fade counter for smooth delay time changes
    FxDelayFadeCntr = FxDelayFadeCntr + FX_DELAY_FADE_DELTA;
    FxDelayFadeCntr = (FxDelayFadeCntr >= 1.0f) ? 1.0f : FxDelayFadeCntr;
    
    // Generate fade target (0 or 1)
    tempXn = (FxDelayFadeCntr >= 1.0f) ? 1.0f : 0.0f;
    
    // Low-pass filter the gain for smooth transitions
    FxDelayGainYn = FX_DELAY_LP_B1 * FxDelayGainYn + (1.0f - FX_DELAY_LP_B1) * tempXn;
    
    // Read delayed signals from memory
    FxDelayOutL = FxDelayGainYn * ReadFromMemory(FxDelayReadIndices[FxDelayReadPtr++]);
    FxDelayOutR = FxDelayGainYn * ReadFromMemory(FxDelayReadIndices[FxDelayReadPtr]);
    
    // Write new input mixed with feedback to delay line
    // Formula: Input * InputGain + DelayedSignal * FeedbackGain
    WriteToMemory(FxDelayWriteIndices[FxDelayWritePtr++], 
                  FX_DELAY_GN_INPUT * fXnL + FxDelayGnFeed * FxDelayOutL);
    WriteToMemory(FxDelayWriteIndices[FxDelayWritePtr], 
                  FX_DELAY_GN_INPUT * fXnR + FxDelayGnFeed * FxDelayOutR);
    
    // Output the delayed signals
    *OutL = FxDelayOutL;
    *OutR = FxDelayOutR;
}

// ==================== Additional Utility Functions ====================

/**
 * Mix dry and wet signals
 * @param dry - Dry (original) signal
 * @param wet - Wet (processed) signal
 * @param mix - Mix amount (0.0 = all dry, 1.0 = all wet)
 * @return Mixed signal
 */
float FxDelayMix(float dry, float wet, float mix) {
    return dry * (1.0f - mix) + wet * mix;
}

/**
 * Process with dry/wet mix
 * Convenience function that includes mixing
 */
void FxDelayProcessWithMix(float fXnL, float fXnR, float *OutL, float *OutR, float mix) {
    float wetL, wetR;
    
    // Process through delay
    FxDelayProcess(fXnL, fXnR, &wetL, &wetR);
    
    // Mix dry and wet signals
    *OutL = FxDelayMix(fXnL, wetL, mix);
    *OutR = FxDelayMix(fXnR, wetR, mix);
}