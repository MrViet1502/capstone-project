//----------------------------------------------------------------------------
// CONFIDENTIAL
//
// COPYRIGHT RESERVED, 2018 Robert Bosch (Australia) Pty Ltd.
// All rights reserved.  The reproduction, distribution and utilisation of
// this document as well as the communication of its contents to others
// without explicit authorisation is prohibited.  Offenders will be held liable
// for the payment of damages.  All rights reserved in the event of the grant
// of a patent, utility model or design.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//! \file
//! \brief  Public Header File of the CanIf Module
//!
//! Module Description
//!     See corresponding C file
//----------------------------------------------------------------------------


#ifndef CANIF_CBK_H
#define CANIF_CBK_H

//============================================================================
// Include Files
//============================================================================
#include "CanIf_Types.h"


//============================================================================
// Public Macros
//============================================================================

//============================================================================
// Public Types
//============================================================================
typedef void (*CanIf_PduUserRxIndicationNameType)(PduIdType RxPduId, const PduInfoType* PduInfoPtr);
typedef void (*CanIf_PduUserTxConfirmationNameType)(PduIdType TxPduId);

typedef struct CanIf_PduUserCallback
{
    CanIf_PduUserRxIndicationNameType   UserRxIndicationFn;
    CanIf_PduUserTxConfirmationNameType UserTxConfirmationFn;
    PduIdType                           FakeTxConfirmPduId;     //!< If a Rx frame is received, fake a Tx confirmation
} CanIf_PduUserCallbackNameType;


//============================================================================
// Public Variables
//============================================================================

//============================================================================
// Public Functions
//============================================================================
#ifdef __cplusplus
extern "C"{
#endif

    void CanIf_TxConfirmation( PduIdType CanTxPduId );

    void CanIf_RxIndication( const Can_HwType *Mailbox, PduInfoType *PduInfoPtr );

    void CanIf_ControllerModeIndication( uint8 ControllerId, CanIf_ControllerModeType ControllerMode );

    void CanIf_TrcvModeIndication( uint8 TransceiverId, CanTrcv_TrcvModeType TransceiverMode );

    void CanIf_ControllerBusOff( uint8 ControllerId );


#ifdef __cplusplus
}
#endif

#endif /* CANIF_CBK_H */


//-------------------- End of File -------------------------------------------
