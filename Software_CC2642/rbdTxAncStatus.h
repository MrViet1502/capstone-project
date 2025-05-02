//----------------------------------------------------------------------------
// CONFIDENTIAL
//
// COPYRIGHT RESERVED, 2023 Robert Bosch (Australia) Pty Ltd.
// All rights reserved.  The reproduction, distribution and utilisation of
// this document as well as the communication of its contents to others
// without explicit authorisation is prohibited.  Offenders will be held liable
// for the payment of damages.  All rights reserved in the event of the grant
// of a patent, utility model or design.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//! \file   rbdTxAncStatus.h
//! \brief  Public Header File Tx Anchor Status Module
//!
//! Module Description
//!     See corresponding C file
//----------------------------------------------------------------------------


#ifndef RBD_TXANCSTATUS_H
#define RBD_TXANCSTATUS_H

#include "rbdTxAncStatus_Cfg.h"
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
void rbdTxAncStatus_Init(void);
void rbdTxAncStatus_Cyclic(void);
void rbdTxAncStatus_SetCanIdx(uint8 CanIdx);

#endif //RBD_TXANCSTATUS_H
