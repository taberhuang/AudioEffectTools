/******************************************************************************

******************************************************************************/
#ifndef __project_config_h__
#define __project_config_h__



#define FIRMWARE_VERSION_MAIN       0
#define FIRMWARE_VERSION_SUB        6  
#define FIRMWARE_VERSION_TINY       14
#define FIRMWARE_VERSION            BYTE_TO_HALF_WORD(((FIRMWARE_VERSION_MAIN<<4) | FIRMWARE_VERSION_SUB),FIRMWARE_VERSION_TINY)


// Helper macro: convert month name to number
#define MONTH_TO_NUM(m) ( \
    (m)[0] == 'J' && (m)[1] == 'a' && (m)[2] == 'n' ? 1 : \
    (m)[0] == 'F' && (m)[1] == 'e' && (m)[2] == 'b' ? 2 : \
    (m)[0] == 'M' && (m)[1] == 'a' && (m)[2] == 'r' ? 3 : \
    (m)[0] == 'A' && (m)[1] == 'p' && (m)[2] == 'r' ? 4 : \
    (m)[0] == 'M' && (m)[1] == 'a' && (m)[2] == 'y' ? 5 : \
    (m)[0] == 'J' && (m)[1] == 'u' && (m)[2] == 'n' ? 6 : \
    (m)[0] == 'J' && (m)[1] == 'u' && (m)[2] == 'l' ? 7 : \
    (m)[0] == 'A' && (m)[1] == 'u' && (m)[2] == 'g' ? 8 : \
    (m)[0] == 'S' && (m)[1] == 'e' && (m)[2] == 'p' ? 9 : \
    (m)[0] == 'O' && (m)[1] == 'c' && (m)[2] == 't' ? 10 : \
    (m)[0] == 'N' && (m)[1] == 'o' && (m)[2] == 'v' ? 11 : \
    (m)[0] == 'D' && (m)[1] == 'e' && (m)[2] == 'c' ? 12 : 0 )

// Extract year, month, day from __DATE__
// __DATE__ format example: "Jan  1 2025" or "Dec 31 2025"
#define FIRMWARE_YEAR  ((__DATE__[7] - '0') * 1000 + (__DATE__[8] - '0') * 100 + (__DATE__[9] - '0') * 10 + (__DATE__[10] - '0'))
#define FIRMWARE_MONTH MONTH_TO_NUM(__DATE__)
#define FIRMWARE_DAY   (((__DATE__[4] >= '0' && __DATE__[4] <= '9') ? (__DATE__[4] - '0') * 10 : 0) + (__DATE__[5] - '0'))



#endif






