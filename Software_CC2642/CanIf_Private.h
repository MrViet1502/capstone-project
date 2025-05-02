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
//! \brief  Private Header File of the CanIf Module
//!
//! Module Description
//!     See corresponding C file
//----------------------------------------------------------------------------


#ifndef CANIF_PRIVATE_H
#define CANIF_PRIVATE_H

#ifdef __cplusplus
extern "C"{
#endif

//============================================================================
// Include Files
//============================================================================


//============================================================================
// Public Macros
//============================================================================
#define DET_MODULEID_CANIF				(60)	//!< Module ID for CAN Interface

#define CANIF_API_INIT                  (1u)
#define CANIF_API_TRANSMIT              (2u)
#define CANIF_API_RXINDICATION          (3u)
#define CANIF_API_TXCONFIRMATION        (4u)
#define CANIF_API_GET_PDUID             (5u)
#define CANIF_API_SET_ANCHOR_CANID      (6u)
#define CANIF_API_SET_CTRL_MODE         (7u)
#define CANIF_API_GET_CTRL_MODE         (8u)
#define CANIF_API_SET_TRCV_MODE         (9u)
#define CANIF_API_GET_TRCV_MODE         (10u)
#define CANIF_API_CTRL_BUSOFF           (16u)
#define CANIF_API_CTRL_MODE_IND         (17u)
#define CANIF_API_TRCV_MODE_IND         (22u)

#define CANIF_ERR_MCAN_TRANSITION       (1u)
#define CANIF_ERR_TRCV_TRANSITION       (2u)
#define CANIF_ERR_WKUP_CTRL_REG         (3u)
#define CANIF_ERR_PARAM_POINTER         (4u)
#define CANIF_ERR_PARAM_PDUID           (5u)
#define CANIF_ERR_PARAM_CTRLID          (6u)
#define CANIF_ERR_PARAM_TRCVID          (7u)
#define CANIF_ERR_PARAM_CTRLMODE        (8u)
#define CANIF_ERR_PARAM_TRCVMODE        (9u)
#define CANIF_ERR_TRANSMIT_FAILED       (10u)
#define CANIF_ERR_NVM_BLE_FAILED        (11u)
#define CANIF_ERR_PARAM_BLE_INVALID     (12u)
#define CANIF_ERR_PARAM_VOLTAGE         (13u)
#define CANIF_ERR_EXTD_CANID_FAILED     (14u)

#define CANIF_NO_FAKE_CONFIRM           CANIF_PDU_MAX  //!< Out of range PDU, indicates no fake TxConfirmation should be used.

//============================================================================
// Module variables
//============================================================================


#ifdef __cplusplus
}
#endif

#endif /* include guard */


//-------------------- End of File -------------------------------------------
