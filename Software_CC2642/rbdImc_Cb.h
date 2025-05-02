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
//! \brief  Callbacks from ImcIf
//!
//----------------------------------------------------------------------------

#ifndef RBD_IMC_CB_H
#define RBD_IMC_CB_H

#include "ImcIf_Api.h"
#include "rbdElTime_Api.h"

//============================================================================
// Public Macros
//============================================================================




//============================================================================
// Public Types
//============================================================================


//============================================================================
// Public Variables
//============================================================================


//============================================================================
// Public Functions
//============================================================================
/*AXIVION Disable MisraC2012-8.6: no problem to define prototype always */
// Simple PduR interface
void rbdImc_IfTxConfirmation( ImcIf_MsgType_et const MsgId );
void rbdImc_IfRxIndication(
        ImcIf_MsgType_et const    MsgId,
        rbdImc_Length_t  const Length,
        uint8            const *  Msg,
        rbd_ElTime_Value_t const  timestamp
        );

// TP interface
void rbdImc_TpRxIndication(
        ImcIf_MsgType_et const   MsgId,
        rbdImc_Length_t  const   Length,
        uint8            const * Msg,
        rbd_ElTime_Value_t const timestamp
        );

void rbdImc_IfTxInterrupt( ImcIf_MsgType_et const MsgId );

void rbdImc_TpTxConfirmation(
        ImcIf_MsgType_et const   MsgId
        );

// Ping function, for time measurement.
void rbdImc_PingRxIndication(
        ImcIf_MsgType_et const   MsgId,
        rbdImc_Length_t  const   Length,
        uint8            const * const Msg,
        rbd_ElTime_Value_t const timestamp
        );

#endif // Include guard

//-------------------- End of File -------------------------------------------
