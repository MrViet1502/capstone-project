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
//! \brief  Configurtion Header File of the Imc Module
//!
//! This file is private and must not be directly included by any other
//! module.
//----------------------------------------------------------------------------

#ifndef RBD_IMC_PDUR_H
#define RBD_IMC_PDUR_H

#include "rbd_Pk.h"
#include "ComStack_Types.h"
#include "ImcIf_Api.h"
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
// PduR simple interface
#ifndef RBD_PK_MASTER_MCU
// this function has prototype in Autosar\Appl\GenData\Components\rbdImcCom.h
Std_ReturnType rbdImcCom_Transmit(PduIdType TxPduId, PduInfoType const * PduInfoPtr);
#endif
Std_ReturnType rbdImcCom_CancelTransmit(PduIdType CanTxPduId);

// TP interface
#ifndef RBD_PK_MASTER_MCU
// this function has prototype in Autosar\Appl\GenData\Components\rbdImcTp.h
Std_ReturnType rbdImcTp_Transmit(PduIdType TxPduId, PduInfoType const * PduInfoPtr);
#endif
Std_ReturnType rbdImcTp_CancelTransmit(PduIdType CanTxPduId);

#endif // Include guard

//-------------------- End of File -------------------------------------------
