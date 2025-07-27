/*********************************************************************************
 * COPYRIGHT (C) ERMA Group s.r.o. Slovakia
 * ALL RIGHTS RESERVED
 * The copying, use, distribution or disclosure of the confidential and
 * proprietary information contained in this document(s) is strictly prohibited
 * without prior written consent. Any breach shall subject the infringing party
 * to remedies. The owner reserves all rights in the event of the grant of a
 * patent or the registration of a utility model or design.
 *********************************************************************************/
/**
 * \file        estf_def.h
 * \brief       Definition of I2C standards.
 * \details     For MISRA programming guideline the definition of compiler independent
 *              types is required. The definition of the types is done on basis of
 *              the standard C types. However, check of precision is necessary.
 * \author      Rastislav Oravec
 * \compiler    
 * \controller  all CPUs used in electronic development
 *****************************************************************************/

/**** include guard **********************************************************/
#ifndef ESTF_DEF_HINC
#define ESTF_DEF_HINC

#ifdef __cplusplus
extern "C" {
#endif
/**** includes ***************************************************************/
#include <stdint.h>

/**** type definitions *******************************************************/
#define ESTF_MAX_DATA_LEN 32U
#define ESTF_SAUCE_SIZE   3U
#define ESTF_HEADER_SIZE  2U

typedef struct {
    uint8_t  frameId;      // Unique identifier for the frame
    uint8_t  dataLength;   // Length of the data
    uint8_t  data[ESTF_MAX_DATA_LEN];         // Pointer to the data buffer
    uint8_t  checksum;     // Checksum for error checking
} ESTF_Frame_t;

typedef union {
    ESTF_Frame_t frame;
    uint8_t raw[sizeof(ESTF_Frame_t)];
} ESTF_Packet_t;

typedef struct {
    uint8_t index;
    uint8_t buffer[sizeof(ESTF_Packet_t)];
} ESTF_ReceiverBuffer_t;

/**** function prototypes ****************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ESTF_DEF_HINC */
/**** end of file ************************************************************/
