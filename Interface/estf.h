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
 * \file        estf.h
 * \brief       Definition of I2C standards.
 * \details     For MISRA programming guideline the definition of compiler independent
 *              types is required. The definition of the types is done on basis of
 *              the standard C types. However, check of precision is necessary.
 * \author      Rastislav Oravec
 * \compiler    
 * \controller  all CPUs used in electronic development
 *****************************************************************************/

/**** include guard **********************************************************/
#ifndef ESTF_HINC
#define ESTF_HINC

/**** includes ***************************************************************/
#include "estf_def.h"

/**** type definitions *******************************************************/


/**** function prototypes ****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

ESTF_Packet_t ESTF_CreatePacket(uint8_t au8_frameId, uint8_t * au8_dataP, uint8_t au8_dataLength);
void ESTF_ProcessByte(uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* ESTF_HINC */
/**** end of file ************************************************************/
