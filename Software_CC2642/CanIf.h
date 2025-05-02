/*##############################################################################
#                                ROBERT BOSCH                                  #
#                            Melbourne,  AUSTRALIA                             #
###########################################################################*//**
@file
@brief CanIf API

 The CAN Interface module provides a unique interface to manage different CAN 
 hardware device types like CAN Controllers and CAN Transceivers used by the 
 defined ECU hardware layout.  It also maps PDU IDs to CAN IDs.


*//*############################################################################
# CONFIDENTIAL                                                                 #
#                                                                              #
# COPYRIGHT RESERVED, 2023 Robert Bosch (Australia) Pty Ltd. All rights        #
# reserved.  The reproduction, distribution and utilisation of this document   #
# as well as the communication of its contents to others without explicit      #
# authorisation is prohibited.  Offenders will be held liable for the payment  #
# of damages.  All rights reserved in the event of the grant of a patent,      #
# utility model or design.                                                     #
#                                                                              #
##############################################################################*/

#ifndef CANIF_H
#define CANIF_H


/*==============================================================================

	INCLUDES

==============================================================================*/
#include "Std_Types.h"
#include "CanIf_Types.h"
#include "CanIf_Cbk.h"


/*==============================================================================

	DEFINES

==============================================================================*/
#if defined( RBD_PK_MINI )
#define CANIF_DEV_ERROR_DETECT                      (STD_OFF)
#else
#define CANIF_DEV_ERROR_DETECT                      (STD_ON)
#endif

												
/*==============================================================================

	TYPES

==============================================================================*/


/*==============================================================================

	GLOBAL VARIABLE DECLARATIONS

==============================================================================*/
//! The CAN configuration structure.  Note, this is declared in CanIf_Cfg.c per ECU.
extern const CanIf_ConfigType CanIf_Config;


/*==============================================================================

	INLINE FUNCTION DEFINITIONS

==============================================================================*/


/*==============================================================================

	FUNCTION DECLARATIONS

==============================================================================*/
#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

void CanIf_Init( void );

void CanIf_RefreshDynamicId( void );

Std_ReturnType CanIf_Transmit( PduIdType TxPduId, const PduInfoType* PduInfoPtr );

Std_ReturnType CanIf_SetControllerMode( uint8 ControllerId, CanIf_ControllerModeType ControllerMode );

Std_ReturnType CanIf_GetControllerMode( uint8 ControllerId,  CanIf_ControllerModeType *ControllerModePtr );

Std_ReturnType CanIf_SetTrcvMode( uint8 TransceiverId, CanTrcv_TrcvModeType TransceiverMode );

Std_ReturnType CanIf_GetTrcvMode( uint8 TransceiverId, CanTrcv_TrcvModeType *TransceiverModePtr);

#ifdef __cplusplus
}
#endif //__cplusplus


#endif /* CANIF_H */

