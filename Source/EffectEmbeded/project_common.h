/******************************************************************************

******************************************************************************/
#ifndef __project_common_h__
#define __project_common_h__


#include <stdio.h>
#include <stdint.h>  // For uint8_t, uint16_t, uint32_t definitions

#include "project_config.h"

// Byte manipulation macros - used by project_config.h for firmware version
#define BYTE_TO_WORD(hh,hl,lh,ll)       ((uint32_t) ((((uint32_t)(hh)) << 24) | (((uint32_t)(hl)) << 16) \
                                          | (((uint32_t)(lh)) << 8) | ((uint32_t)(ll))))
#define BYTE_TO_HALF_WORD(high, low)    ((uint16_t)((((uint16_t)(high)) << 8 ) | ((uint16_t)(low))))
#define HALF_WORD_TO_WORD(high, low)    ((uint32_t)((((uint32_t)(high)) << 16 ) | ((uint32_t)(low))))


#endif
