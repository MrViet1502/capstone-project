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
//! \brief  Interface between Imc and PduR
//----------------------------------------------------------------------------

#ifndef RBDIMC_PDUR_TYPES_H
#define RBDIMC_PDUR_TYPES_H

//============================================================================
//  Includes
//============================================================================

#include "Std_Types.h"
#include "ComStack_Types.h"
#include "ImcIf_Api.h"

//============================================================================
// Local Macros
//============================================================================





//============================================================================
// Local Types
//============================================================================

//----------------------------------------------------------------------------
//! \brief  Configuration for one PDU
//----------------------------------------------------------------------------
typedef struct
{
    PduIdType        UpperTxFromPduR; //!< PduR ID for the transmit direction
    PduIdType        UpperTxToPduR;   //!< PduR ID for the transmit direction
    PduIdType        UpperRxPduId;    //!< PduR ID for the receive direction
    ImcIf_MsgType_et ImcPduId;        //!< Imc ID
} rbdImc_TpMsgCfg_st;


//----------------------------------------------------------------------------
//! \brief  Runtime parameters - transmit
//----------------------------------------------------------------------------
typedef struct
{
    uint16  bytesRemaining;   //!< Number of bytes remaining in this transfer
    uint16  offset;           //!< Current position
    uint8   sequence;         //!< Sequence number
    uint8   numPktsPending;   //!< Number of Tx-confirmations we are waiting for
} rbdImc_TpTxInfo_st;


//----------------------------------------------------------------------------
//! \brief  Runtime parameters - receive
//----------------------------------------------------------------------------
typedef struct
{
    uint16  bytesRemaining;   //!< Number of bytes remaining in this transfer
    boolean active;           //!< Is the reception active?
    uint8   sequence;         //!< Sequence number
} rbdImc_TpRxInfo_st;




//----------------------------------------------------------------------------
//! \brief  Configuration for one PDU for rbdImc_PduR, Rx direction
//----------------------------------------------------------------------------
typedef struct
{
    PduIdType        UpperRxToPduR;   //!< PduR ID for the receive direction
    ImcIf_MsgType_et Imc_PduId;       //!< Imc ID
} rbdImc_PduRMsgCfgRx_st;


//----------------------------------------------------------------------------
//! \brief  Configuration for one PDU for rbdImc_PduR, Tx direction
//----------------------------------------------------------------------------
typedef struct
{
    PduIdType        UpperTxFromPduR; //!< PduR ID for the transmit direction
    PduIdType        UpperTxToPduR;   //!< PduR ID for the transmit direction
    ImcIf_MsgType_et Imc_PduId;       //!< Imc ID
} rbdImc_PduRMsgCfgTx_st;


//----------------------------------------------------------------------------
//! \brief  Configuration for rbdImc_PduR
//----------------------------------------------------------------------------
typedef struct
{
    uint32                               NumTxMsg;    //!< Number of messages configured
    uint32                               NumRxMsg;    //!< Number of messages configured
    rbdImc_PduRMsgCfgTx_st const * const TxCfg;       //!< Pointer to message configuration array
    rbdImc_PduRMsgCfgRx_st const * const RxCfg;       //!< Pointer to message configuration array
} rbdImc_PduRCfg_st;


//----------------------------------------------------------------------------
//! \brief  Configuration for rbdImc_PdurTp
//----------------------------------------------------------------------------
typedef struct
{
    uint32                           NumMsg;    //!< Number of messages configured
    rbdImc_TpMsgCfg_st const * const MsgCfg;    //!< Pointer to the message configuration
    rbdImc_TpTxInfo_st       * const TxInfo;    //!< Pointer to message status in RAM
    rbdImc_TpRxInfo_st       * const RxInfo;    //!< Pointer to message status in RAM
} rbdImc_TpCfg_st;

//============================================================================
// Local Function Prototypes
//============================================================================




//============================================================================
// Local Variables
//============================================================================

extern const rbdImc_TpCfg_st rbdImc_TpCfg;
extern const rbdImc_PduRCfg_st rbdImc_PduRCfg;




//============================================================================
// Function declarations.
//============================================================================


#endif // include guard

//-------------------- End of File -------------------------------------------
