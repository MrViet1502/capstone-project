//----------------------------------------------------------------------------
// CONFIDENTIAL
//
// COPYRIGHT RESERVED, 2020 Robert Bosch (Australia) Pty Ltd.
// All rights reserved.  The reproduction, distribution and utilisation of
// this document as well as the communication of its contents to others
// without explicit authorisation is prohibited.  Offenders will be held liable
// for the payment of damages.  All rights reserved in the event of the grant
// of a patent, utility model or design.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//! \file
//! \brief
//----------------------------------------------------------------------------

/*AXIVION Disable Style CertC-MSC24 : Standard function memcpy is used and string.h is included */
//============================================================================
//  Includes
//============================================================================

#include "rbd_Pk.h"

// Standard includes
#include <string.h>
#include <assert.h>     // For static_assert()
#include "DimOf.h"

// BSW & RTE
#include "Os.h"
#if defined(RBD_PK_MASTER_MCU)
#include "PduR_rbdAnchorComTp.h"
#include "rbdAnchorComTp_Cbk.h"
#else
#include "PduR.h"
#include "rbdAnchorCom_Cb.h"
#endif

// ASW
#include "rbdAnchorCom_Api.h"
#include "rbdAnchorCom_Private.h"

#if defined(RBD_PK_ANCHOR_BLE) || defined(RBD_PK_MINI_APP) || defined(RBD_PK_ANCHOR_BLE_FBL) || defined(RBD_PK_MASTER_BLE_FBL) || defined(RBD_PK_MINI_FBL)|| defined(RBD_PK_BLE_FBL) || defined(RBD_PK_KEYFOB_BLE)
#include "rbUds_ApiCfg.h"
#endif

#if defined(RBD_PK_CPU_CC26xx)
#include "Fee_Cfg.h"
#endif

#if defined(RBD_PK_CPU_NCJ29D5)
#include "R4Sec_Api.h"
#include "rbdUwbInfo.h"
#include "rbdR4Slave_Api.h"
#endif
#define TP_INCLUDE      //!< Indicate to include TP Rx header files in CfgMsg.h

// RxIndication functions
#include "rbdAnchorCom_CfgMsg.h"

#ifdef  RBDANCHORCOMTP_TIMING           // Define this to measure transmit and receive timing
#include "rbdElTime_Api.h"
#endif

#if defined(RBD_PK_MASTER_MCU)
// Kludge due to RTE configuration not allowing take-address
#undef Rte_Call_RP_PingMaster_RxIndication
#undef Rte_Call_RP_Ranging_RxIndication

#ifdef RBD_PK_CONTRACT_PHASE_ETAS
    // Adjust for macro differences between ETAS and DaVinci.
    #define Rte_Call_RP_PingMaster_RxIndication     Rte_Call_rbdAnchorCom_RP_PingMaster_RxIndication
    #define Rte_Call_RP_Ranging_RxIndication        Rte_Call_rbdAnchorCom_RP_Ranging_RxIndication
#else //#ifdef RBD_PK_CONTRACT_PHASE_ETAS

static Std_ReturnType Rte_Call_RP_PingMaster_RxIndication(VAR(uint8, AUTOMATIC) OtherEndIdx,
                                                             VAR(uint8, AUTOMATIC) PduId,
                                                             VAR(uint16, AUTOMATIC) Length,
                                                             P2CONST(uint8, AUTOMATIC, RTE_APPL_DATA) Buf)
{
    rbdPingMaster_Rx( OtherEndIdx, PduId, Length, Buf);
    return E_OK;
}

static Std_ReturnType Rte_Call_RP_Ranging_RxIndication(VAR(uint8, AUTOMATIC) OtherEndIdx,
                                                             VAR(uint8, AUTOMATIC) PduId,
                                                             VAR(uint16, AUTOMATIC) Length,
                                                             P2CONST(uint8, AUTOMATIC, RTE_APPL_DATA) Buf)
{
    rbdRangingCoord_AnchorComRx( OtherEndIdx, PduId, Length, Buf);
    return E_OK;
}
#endif //#ifdef RBD_PK_CONTRACT_PHASE_ETAS
#endif //#if defined(RBD_PK_MASTER_MCU)

//============================================================================
// Local Macros
//============================================================================
#define ANCHORCOM_PERFORMANCE_TEST	 STD_OFF	// Enable AnchorCom message counter and extra processing buffer

#if 1 == RBDANCHORCOM_TP_NUM_LL_TX
#define RBDANCHORCOMTP_BUFINIT(Name) { TxBuffer_##Name[0] } /*AXIVION Line MisraC2012Directive-4.9 : Macro is used to generate data in rbdAnchorCom_CfgMsg during build */
#define RBDANCHORCOMTP_NOBUF         { NULL }

#elif 6 == RBDANCHORCOM_TP_NUM_LL_TX
/*AXIVION Next Line MisraC2012-20.10 MisraC2012Directive-4.9: The code is obscure, but very useful. It can't be a function. */
#define RBDANCHORCOMTP_BUFINIT(Name) { \
    TxBuffer_##Name[0], \
    TxBuffer_##Name[1], \
    TxBuffer_##Name[2], \
    TxBuffer_##Name[3], \
    TxBuffer_##Name[4], \
    TxBuffer_##Name[5], \
    }
#define RBDANCHORCOMTP_NOBUF { NULL, NULL, NULL, NULL, NULL, NULL }

#elif 7 == RBDANCHORCOM_TP_NUM_LL_TX
/*AXIVION Next Line MisraC2012-20.10 MisraC2012Directive-4.9: The code is obscure, but very useful. It can't be a function. */
#define RBDANCHORCOMTP_BUFINIT(Name) { \
    TxBuffer_##Name[0], \
    TxBuffer_##Name[1], \
    TxBuffer_##Name[2], \
    TxBuffer_##Name[3], \
    TxBuffer_##Name[4], \
    TxBuffer_##Name[5], \
    TxBuffer_##Name[6], \
    }
#define RBDANCHORCOMTP_NOBUF { NULL, NULL, NULL, NULL, NULL, NULL, NULL }

#elif 8 == RBDANCHORCOM_TP_NUM_LL_TX
/*AXIVION Next Line MisraC2012-20.10 MisraC2012Directive-4.9: The code is obscure, but very useful. It can't be a function. */
#define RBDANCHORCOMTP_BUFINIT(Name) { \
    TxBuffer_##Name[0], \
    TxBuffer_##Name[1], \
    TxBuffer_##Name[2], \
    TxBuffer_##Name[3], \
    TxBuffer_##Name[4], \
    TxBuffer_##Name[5], \
    TxBuffer_##Name[6], \
    TxBuffer_##Name[7], \
    }
#define RBDANCHORCOMTP_NOBUF { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }

#elif 9 == RBDANCHORCOM_TP_NUM_LL_TX
/*AXIVION Next Line MisraC2012-20.10 MisraC2012Directive-4.9: The code is obscure, but very useful. It can't be a function. */
#define RBDANCHORCOMTP_BUFINIT(Name) { \
    TxBuffer_##Name[0], \
    TxBuffer_##Name[1], \
    TxBuffer_##Name[2], \
    TxBuffer_##Name[3], \
    TxBuffer_##Name[4], \
    TxBuffer_##Name[5], \
    TxBuffer_##Name[6], \
    TxBuffer_##Name[7], \
    TxBuffer_##Name[8], \
    }
#define RBDANCHORCOMTP_NOBUF { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }

#elif 10 == RBDANCHORCOM_TP_NUM_LL_TX
/*AXIVION Next Line MisraC2012-20.10 MisraC2012Directive-4.9: The code is obscure, but very useful. It can't be a function. */
#define RBDANCHORCOMTP_BUFINIT(Name) { \
    TxBuffer_##Name[0], \
    TxBuffer_##Name[1], \
    TxBuffer_##Name[2], \
    TxBuffer_##Name[3], \
    TxBuffer_##Name[4], \
    TxBuffer_##Name[5], \
    TxBuffer_##Name[6], \
    TxBuffer_##Name[7], \
    TxBuffer_##Name[8], \
    TxBuffer_##Name[9], \
    }
#define RBDANCHORCOMTP_NOBUF { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }

#elif 11 == RBDANCHORCOM_TP_NUM_LL_TX
/*AXIVION Next Line MisraC2012-20.10 MisraC2012Directive-4.9: The code is obscure, but very useful. It can't be a function. */
#define RBDANCHORCOMTP_BUFINIT(Name) { \
    TxBuffer_##Name[0], \
    TxBuffer_##Name[1], \
    TxBuffer_##Name[2], \
    TxBuffer_##Name[3], \
    TxBuffer_##Name[4], \
    TxBuffer_##Name[5], \
    TxBuffer_##Name[6], \
    TxBuffer_##Name[7], \
    TxBuffer_##Name[8], \
    TxBuffer_##Name[9], \
    TxBuffer_##Name[10], \
    }
#define RBDANCHORCOMTP_NOBUF { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }

#elif 12 == RBDANCHORCOM_TP_NUM_LL_TX
/*AXIVION Next Line MisraC2012-20.10 MisraC2012Directive-4.9: The code is obscure, but very useful. It can't be a function. */
#define RBDANCHORCOMTP_BUFINIT(Name) { \
    TxBuffer_##Name[0], \
    TxBuffer_##Name[1], \
    TxBuffer_##Name[2], \
    TxBuffer_##Name[3], \
    TxBuffer_##Name[4], \
    TxBuffer_##Name[5], \
    TxBuffer_##Name[6], \
    TxBuffer_##Name[7], \
    TxBuffer_##Name[8], \
    TxBuffer_##Name[9], \
    TxBuffer_##Name[10], \
    TxBuffer_##Name[11], \
    }
#define RBDANCHORCOMTP_NOBUF { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }

#else
#error
#endif

#define RBDANCHORCOM_TP_MAX_MSG_TX          (4096u)   //!< Maximum transmit length = maximum length of ISO-TP

//============================================================================
// Local Types
//============================================================================





//------------------------------------------------------------------------------
//! \brief  Configuration relating to each data item
//!
//! Data items only have one configuration record. It's not necessary to
//! multiply this by the number of channels or anchors.
//------------------------------------------------------------------------------
typedef struct rbdAnchorComTp_Cfg_s
{
    uint8                          PduId;                           //!< Our PduId
    rbdAnchorComTp_Ch_et           Channel;                         //!< Channel (BLE or UWB)
    rbdAnchorCom_TpRxIndication_fp RxIndication;                    //!< RxIndication function poitner
    uint8 *                        Buffer[RBDANCHORCOM_TP_NUM_LL_TX];  //!< Pointer to buffers
    uint16                         Length;                          //!< Maximum data length
    boolean                        EncDecFlag;                      //!< Enccrypt/Decrypt enable flag
    boolean                        Broadcast;                       //!< Broadcast flag
} rbdAnchorComTp_Cfg_st;

//------------------------------------------------------------------------------
//!\brief  Runtime information about one PduR-TP channel
//------------------------------------------------------------------------------



#ifdef RBDANCHORCOMTP_TIMING
static rbdElTime_Accumulate_st rbdAnchorComTp_TxCsmTime_us;
static rbdElTime_Accumulate_st rbdAnchorComTp_RxTime_ms;
#endif

//============================================================================
// Global Contants
//============================================================================


//============================================================================
// Local Variables
//============================================================================

// doxygen complains about TP_MSG_TX defined twice, so ignore this macros
/// \cond DO_NOT_DOCUMENT

// Create the transmit buffers for each message
#define TP_MSG_TX(Name,Id,Channel,Length,EncDec,BroadCast,RxIndication) static uint8 TxBuffer_##Name[RBDANCHORCOM_TP_NUM_LL_TX][((Length) + RBDANCHORCOM_TP_RESERVED_LEN)]; /*AXIVION Line MisraC2012Directive-4.9 : Macro is used to generate data in rbdAnchorCom_CfgMsg during build */ /*AXIVION Line CertC-PRE11 : Macro is substituted by its expansion */
#include "rbdAnchorCom_CfgMsg.h"    /*AXIVION Line MisraC2012-20.1 : Header file is used to generate data during build */
#undef TP_MSG_TX  /*AXIVION Line MisraC2012-20.5 :undef allowed for special macro files */

/// \endcond

// Create the message configuration array
/* AXIVION Next Line CertC-ARR02 : Size of array will be computed at compile time */
static const rbdAnchorComTp_Cfg_st rbdAnchorComTp_MsgCfg[] = {
    #ifndef DOXYGEN
    /*AXIVION Next Line MisraC2012Directive-4.9, CertC-PRE02 : Macro is used to generate data in rbdAnchorCom_CfgMsg during build */
    #define TP_MSG_TX(Name,Id,Channel,Length,EncDec,BroadCast,RxIndication) { rbdAnchorCom_TpPdu_##Name, Channel, NULL,         RBDANCHORCOMTP_BUFINIT(Name), Length, EncDec, BroadCast},   //!< Transmit messages configuration
    /*AXIVION Next Line MisraC2012Directive-4.9, CertC-PRE02 : Macro is used to generate data in rbdAnchorCom_CfgMsg during build */
    #define TP_MSG_RX(Name,Id,Channel,Length,EncDec,BroadCast,RxIndication) { rbdAnchorCom_TpPdu_##Name, Channel, RxIndication, RBDANCHORCOMTP_NOBUF,         Length, EncDec, BroadCast},   //!< Receiving messages configuration
    #include "rbdAnchorCom_CfgMsg.h"    /*AXIVION Line MisraC2012-20.1 : Header file is used to generate data during build */
    #undef TP_MSG_TX  /*AXIVION Line MisraC2012-20.5 :undef allowed for special macro files */
    #undef TP_MSG_RX  /*AXIVION Line MisraC2012-20.5 :undef allowed for special macro files */
    #endif
};
#define RBDANCHORCOM_TP_NUM_MSG             DIMOF(rbdAnchorComTp_MsgCfg)   //!< Number of messages in the configuration.

//! \brief  The runtime state of each message / data item
//!
//! Each anchor/node has its own status.
static rbdAnchorComTp_MsgInfo_st rbdAnchorComTp_MsgTxInfo[RBDANCHORCOM_TP_NUM_LL_TX][RBDANCHORCOM_TP_NUM_MSG];

//! \brief  The queue for sending/receiving messages
//!
//! To maintain FIFO sequence of all messages, we put all TX/RX messages into a queue
//! Size of the queue needs to be designed, now put he same size of message counts for tx queue
//! Put double size of peer nodes number for RX queue

// Now we put Tx/Rx queue size to 16.
#define RBDANCHORCOM_TP_QUEUE_SIZE      (16u)
static rbdAnchorComTp_Queue_st rbdAnchorComTp_TxMsgQueue[RBDANCHORCOM_TP_NUM_LL_TX][RBDANCHORCOM_TP_QUEUE_SIZE];
static rbdAnchorComTp_RxQueue_st rbdAnchorComTp_RxMsgQueue[RBDANCHORCOM_TP_NUM_LL_RX][RBDANCHORCOM_TP_QUEUE_SIZE];
static uint8 rbdAnchorComTp_TxHead[RBDANCHORCOM_TP_NUM_LL_TX], rbdAnchorComTp_TxTail[RBDANCHORCOM_TP_NUM_LL_TX];
static uint8 rbdAnchorComTp_RxHead[RBDANCHORCOM_TP_NUM_LL_RX], rbdAnchorComTp_RxTail[RBDANCHORCOM_TP_NUM_LL_RX];

//! \brief  The runtime state of the underlying TP/PduR channels.
static rbdAnchorComTp_ChTxInfo_st rbdAnchorComTp_ChTxInfo[RBDANCHORCOM_TPCH_MAX][RBDANCHORCOM_TP_NUM_LL_TX] = {0};
static rbdAnchorComTp_ChRxInfo_st rbdAnchorComTp_ChRxInfo[RBDANCHORCOM_TPCH_MAX][RBDANCHORCOM_TP_NUM_LL_RX] = {0};

//! \brief  Buffer to store all received messages and flags for cyclic processing.
static rbdAnchorComTp_ChRxDataInfo_st rbdAnchorComTp_ChRxBuff[RBDANCHORCOM_TPCH_MAX][RBDANCHORCOM_TP_NUM_LL_RX] = {0};
static rbdAnchorComTp_ChRxDataInfo_st rbdAnchorComTp_ChRxTemp[RBDANCHORCOM_TPCH_MAX][RBDANCHORCOM_TP_NUM_LL_RX] = {0};

#if (ANCHORCOM_PERFORMANCE_TEST == STD_ON)
static uint32 rbdAnchorComTp_RxCounter[RBDANCHORCOM_TPCH_MAX][RBDANCHORCOM_TP_NUM_LL_RX] = {0};
static uint32 rbdAnchorComTp_RxBusyCounter[RBDANCHORCOM_TPCH_MAX][RBDANCHORCOM_TP_NUM_LL_RX] = {0};
static uint32 rbdAnchorComTp_VerifyCounter[RBDANCHORCOM_TPCH_MAX][RBDANCHORCOM_TP_NUM_LL_RX] = {0};
#endif


//============================================================================
// Local Function Prototypes
//============================================================================


//============================================================================
// Function declarations.
//============================================================================
/// \cond DO_NOT_DOCUMENT

#if !defined(RBD_PK_MASTER_MCU)
// Replacements for RTE-generated code
LOCAL_INLINE Std_ReturnType PduR_rbdAnchorComTpTransmit(PduIdType id, const PduInfoType* info)
{
    return PduR_TpTransmit( id, info );
}
#endif

/// \endcond

//------------------------------------------------------------------------------
//! \brief  Find the configuration index
//!
//! \return  The array index of the PDU, used for the configuration and status
//!          arrays.
//------------------------------------------------------------------------------
static uint32 rbdAnchorComTp_FindIdx(
        uint8 const PduId //!< PduId to search for
    )
{
    uint32 idx;

    for( idx=0; idx<RBDANCHORCOM_TP_NUM_MSG; idx++ )
    {
        if( rbdAnchorComTp_MsgCfg[idx].PduId == PduId )
        {
            break;
        }
    }
    return idx;
}

//------------------------------------------------------------------------------
//! \brief  Get the pointer to first message
//------------------------------------------------------------------------------
static rbdAnchorComTp_Queue_st* rbdAnchorComTp_GetTxQueueHead(uint32_t OtherEndIdx)
{
    rbdAnchorComTp_Queue_st* HeaderPtr = NULL;

    SuspendAllInterrupts();
    // It's empty, nothing to do
    if (rbdAnchorComTp_TxTail[OtherEndIdx] == rbdAnchorComTp_TxHead[OtherEndIdx])
    {
        // Pointer is alread NULL
    }
    else {
        HeaderPtr = &rbdAnchorComTp_TxMsgQueue[OtherEndIdx][rbdAnchorComTp_TxHead[OtherEndIdx]];
    }
    ResumeAllInterrupts();

    return HeaderPtr;
}

//------------------------------------------------------------------------------
//! \brief  Remove the head in the queue, after the item is fully processed
//------------------------------------------------------------------------------
static Std_ReturnType rbdAnchorComTp_RemoveTxQueueHead(uint32_t OtherEndIdx)
{
    Std_ReturnType ret = E_OK;

    SuspendAllInterrupts();
    // It's empty, nothing to do
    if (rbdAnchorComTp_TxTail[OtherEndIdx] == rbdAnchorComTp_TxHead[OtherEndIdx])
    {
        ret = E_NOT_OK;
    }
    else
    {
        rbdAnchorComTp_TxHead[OtherEndIdx] = (rbdAnchorComTp_TxHead[OtherEndIdx] + 1u) % RBDANCHORCOM_TP_QUEUE_SIZE;
    }
    ResumeAllInterrupts();

    return ret;
}

//------------------------------------------------------------------------------
//! \brief  Add a new message to TX queue
//------------------------------------------------------------------------------
static Std_ReturnType rbdAnchorComTp_AddTxQueue(uint32 const OtherEndIdx, uint8 const msgIdx)
{
    Std_ReturnType ret = E_OK;

    SuspendAllInterrupts();

    // Queue is full
    if (((rbdAnchorComTp_TxTail[OtherEndIdx] + 1u) % RBDANCHORCOM_TP_QUEUE_SIZE) == rbdAnchorComTp_TxHead[OtherEndIdx])
    {
        ret = E_NOT_OK;
    }
    else
    {
        rbdAnchorComTp_TxMsgQueue[OtherEndIdx][rbdAnchorComTp_TxTail[OtherEndIdx]].MsgIdx = msgIdx;
        rbdAnchorComTp_TxMsgQueue[OtherEndIdx][rbdAnchorComTp_TxTail[OtherEndIdx]].AnchorId = OtherEndIdx; /* AXIVION Line MisraC2012-10.3: checked ok */
        rbdAnchorComTp_TxMsgQueue[OtherEndIdx][rbdAnchorComTp_TxTail[OtherEndIdx]].Channel = rbdAnchorComTp_MsgCfg[msgIdx].Channel; /* AXIVION Line MisraC2012-10.3: checked ok */
        rbdAnchorComTp_TxMsgQueue[OtherEndIdx][rbdAnchorComTp_TxTail[OtherEndIdx]].QueueState = RBDANCHORCOM_TX_QUEUED;
        rbdAnchorComTp_TxMsgQueue[OtherEndIdx][rbdAnchorComTp_TxTail[OtherEndIdx]].ErrCounter = 0;
        rbdAnchorComTp_TxTail[OtherEndIdx] = (rbdAnchorComTp_TxTail[OtherEndIdx] + 1u) % RBDANCHORCOM_TP_QUEUE_SIZE;
    }
    ResumeAllInterrupts();
    return ret;
}
//------------------------------------------------------------------------------
//! \brief  Get the pointer to first message
//------------------------------------------------------------------------------
static rbdAnchorComTp_RxQueue_st* rbdAnchorComTp_GetRxQueueHead(uint32_t OtherEndIdx)
{
    rbdAnchorComTp_RxQueue_st* HeaderPtr = NULL;

    SuspendAllInterrupts();

    // It's empty, nothing to do
    if (rbdAnchorComTp_RxTail[OtherEndIdx] == rbdAnchorComTp_RxHead[OtherEndIdx])
    {
        // Pointer is alread NULL
    }
    else
    {
        HeaderPtr = &rbdAnchorComTp_RxMsgQueue[OtherEndIdx][rbdAnchorComTp_RxHead[OtherEndIdx]];
    }

   ResumeAllInterrupts();

    return HeaderPtr;
}

//------------------------------------------------------------------------------
//! \brief  Remove the head in the queue, after the item is fully processed
//------------------------------------------------------------------------------
static Std_ReturnType rbdAnchorComTp_RemoveRxQueueHead(uint32_t OtherEndIdx)
{
    Std_ReturnType ret = E_OK;

    SuspendAllInterrupts();

    // It's empty, nothing to do
    if (rbdAnchorComTp_RxTail[OtherEndIdx] == rbdAnchorComTp_RxHead[OtherEndIdx]) {
        ret = E_NOT_OK;
    }
    else
    {
        rbdAnchorComTp_RxMsgQueue[OtherEndIdx][rbdAnchorComTp_RxHead[OtherEndIdx]].QueueState = RBDANCHORCOM_TX_IDLE;
        rbdAnchorComTp_RxHead[OtherEndIdx] = (rbdAnchorComTp_RxHead[OtherEndIdx] + 1u) % RBDANCHORCOM_TP_QUEUE_SIZE;
    }

    ResumeAllInterrupts();

    return ret;
}

//------------------------------------------------------------------------------
//! \brief  Add a new message to RX queue
//------------------------------------------------------------------------------
static Std_ReturnType rbdAnchorComTp_AddRxQueue(uint32 const Channel, uint32 const OtherEndIdx, boolean IsTempBuf)
{
    Std_ReturnType ret = E_OK;

    SuspendAllInterrupts();

    // Queue is full
    if (((rbdAnchorComTp_RxTail[OtherEndIdx] + 1u) % RBDANCHORCOM_TP_QUEUE_SIZE) == rbdAnchorComTp_RxHead[OtherEndIdx])
    {
        ret = E_NOT_OK;
    }
    else
    {
        rbdAnchorComTp_RxMsgQueue[OtherEndIdx][rbdAnchorComTp_RxTail[OtherEndIdx]].Channel    = Channel; /* AXIVION Line MisraC2012-10.3: checked ok */
        rbdAnchorComTp_RxMsgQueue[OtherEndIdx][rbdAnchorComTp_RxTail[OtherEndIdx]].AnchorId   = OtherEndIdx; /* AXIVION Line MisraC2012-10.3: checked ok */
        rbdAnchorComTp_RxMsgQueue[OtherEndIdx][rbdAnchorComTp_RxTail[OtherEndIdx]].IsTempBuf  = IsTempBuf;
        rbdAnchorComTp_RxMsgQueue[OtherEndIdx][rbdAnchorComTp_RxTail[OtherEndIdx]].QueueState = RBDANCHORCOM_RX_QUEUED;
        rbdAnchorComTp_RxMsgQueue[OtherEndIdx][rbdAnchorComTp_RxTail[OtherEndIdx]].ErrCounter = 0;

        if (IsTempBuf == TRUE)
        {
            rbdAnchorComTp_RxMsgQueue[OtherEndIdx][rbdAnchorComTp_RxTail[OtherEndIdx]].BufferPtr  = rbdAnchorComTp_ChRxTemp[Channel][OtherEndIdx].RxBuffer;
            rbdAnchorComTp_RxMsgQueue[OtherEndIdx][rbdAnchorComTp_RxTail[OtherEndIdx]].DataLen    = rbdAnchorComTp_ChRxTemp[Channel][OtherEndIdx].RxOffset;
        }
        else
        {
            rbdAnchorComTp_RxMsgQueue[OtherEndIdx][rbdAnchorComTp_RxTail[OtherEndIdx]].BufferPtr = rbdAnchorComTp_ChRxBuff[Channel][OtherEndIdx].RxBuffer;
            rbdAnchorComTp_RxMsgQueue[OtherEndIdx][rbdAnchorComTp_RxTail[OtherEndIdx]].DataLen   = rbdAnchorComTp_ChRxBuff[Channel][OtherEndIdx].RxOffset;
        }

        rbdAnchorComTp_RxTail[OtherEndIdx] = (rbdAnchorComTp_RxTail[OtherEndIdx] + 1u) % RBDANCHORCOM_TP_QUEUE_SIZE;
    }

    ResumeAllInterrupts();

    return ret;
}

//------------------------------------------------------------------------------
//! \brief  Find a lower-layer (PduR) ID
//!
//! This function searches the configuration arrays for a matching PduId. It
//! returns the Channel and Other End (Node/Anchor) index of the match.
//!
//! \retval  TRUE if found
//! \retval  FALSE if not found
//------------------------------------------------------------------------------
static boolean rbdAnchorComTp_FindByLLPdu(
        PduIdType const PduId,          //!< PduId to search for
        const PduIdType* PduList,       //!< Array to search
        uint32 const TpPduNum,          //!< PDU number in each TP channel
        uint32 * const ChannelPtr,      //!< Used to return the channel
        uint32 * const OtherEndIdxPtr   //!< Used to return the index of the other end
        )
{
    uint32  Ch;
    uint32  OtherEndIdx = 0u;
    boolean found = FALSE;

    for( Ch=0u; (Ch<(uint32)RBDANCHORCOM_TPCH_MAX); Ch++ ) /*AXIVION Line CertC-STR34 : comparison is of same types*/
    {
        for( OtherEndIdx=0u; OtherEndIdx<TpPduNum; OtherEndIdx++ )
        {
            if( PduList[(Ch*TpPduNum) + OtherEndIdx] == PduId ) /* AXIVION Line MisraC2012-18.1, CertC-ARR30: checked ok */
            {
                if( NULL != ChannelPtr )    /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3: defensive pointer check */
                {
                    *ChannelPtr = Ch;
                }
                if( NULL != OtherEndIdxPtr )    /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3: defensive pointer check */
                {
                    *OtherEndIdxPtr = OtherEndIdx;
                }
                found = TRUE;
                break;
            }
        }

		if (found == TRUE)	// Misra don't like to check found in the for statement
		{
			break;
		}
    }

    return found;
}

//------------------------------------------------------------------------------
//! \brief  Check if it is possible to transmit
//!
//!
//! \retval     E_OK        A buffer can be found
//! \retval     E_NOT_OK    No free buffer
//------------------------------------------------------------------------------
Std_ReturnType rbdAnchorCom_IsTxPossible(
        uint8                  const OtherEndIdx,   //!< Peer index, normally is AnchorId
        uint8                  const PduId,         //!< Message PduId
        boolean                const OverwriteOldData   //!< Flag to overwrite existing data
        )
{
    uint32 const Idx = rbdAnchorComTp_FindIdx( PduId );
    Std_ReturnType result = E_NOT_OK;

    if( Idx >= (uint32)RBDANCHORCOM_TP_NUM_MSG ) /*AXIVION Line CertC-INT00 : Casting same sized integral types*/
    {
        rbdAnchorCom_ReportError( (uint32)PduId, RBD_ANCHORCOM_API_TP_GETBUFFER, RBD_ANCHORCOM_ERR_PDU );
    }
    else if( OtherEndIdx >= RBDANCHORCOM_TP_NUM_LL_TX)    /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : defensive code for input checking */
    {
        rbdAnchorCom_ReportError( OtherEndIdx, RBD_ANCHORCOM_API_TP_GETBUFFER, RBD_ANCHORCOM_ERR_OTHERENDIDX );
    }
#if STD_ON == RBDANCHORCOM_TEST// If test mode is enabled, suppress non-test messages
    // If test mode is enabled, suppress non-test messages
#if defined(RBD_PK_MASTER_MCU)
    else if( (PduId != rbdAnchorCom_TpPdu_BleTest) && (PduId != rbdAnchorCom_TpPdu_UwbTest) && (PduId != rbdAnchorCom_TpPdu_BroadCastTest)
        && (PduId != rbdAnchorCom_TpPdu_PingBle_M2A) && (PduId != rbdAnchorCom_TpPdu_PingUwb_M2A) )
    {
        // No action required, result is already E_NOT_OK
    }
#endif
#if defined(RBD_PK_CPU_CC26xx)
    else if ((PduId != rbdAnchorCom_TpPdu_BleTestReply) && (PduId != rbdAnchorCom_TpPdu_PingBle_A2M))
    {
        // No action required, result is already E_NOT_OK
    }
#endif
#if defined(RBD_PK_CPU_NCJ29D5)
    else if((PduId != rbdAnchorCom_TpPdu_UwbTestReply) && (PduId != rbdAnchorCom_TpPdu_PingUwb_A2M))
    {
        // No action required, result is already E_NOT_OK
    }
#endif
#endif
    else
    {
        // Broadcast messages use a special broadcast target.
        uint8 OtherEndIdxToUse = OtherEndIdx;
        if( rbdAnchorComTp_MsgCfg[Idx].Broadcast == TRUE )
        {
            OtherEndIdxToUse = RBD_PK_MAX_NODES;
        }

        if(   (RBDANCHORCOM_TPSTATE_IDLE == rbdAnchorComTp_MsgTxInfo[OtherEndIdxToUse][Idx].BufState)/*AXIVION Line CertC-STR34 : comparison is of same types*/
           || ((RBDANCHORCOM_TPSTATE_HASDATA == rbdAnchorComTp_MsgTxInfo[OtherEndIdxToUse][Idx].BufState) && (TRUE==OverwriteOldData)))/*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            result = E_OK;
        }
    }

    return result;
}


//------------------------------------------------------------------------------
//! \brief  Flag the data for transmission
//!
//! This function must be called after filling the buffer. It flags the buffer
//! for transmission.
//------------------------------------------------------------------------------
Std_ReturnType rbdAnchorCom_Transmit(
        uint8                  const OtherEndIdx,
        uint8                  const PduId,
        uint16                 const Length,
        uint8          const * const Buf,
        uint16                 const Length2,
        uint8          const * const Buf2,
        boolean                const OverwriteOldData
        )
{
    uint32 const Idx = rbdAnchorComTp_FindIdx( PduId );
    uint16 const TotalLength = Length + Length2;
    Std_ReturnType result = E_NOT_OK;

    if( Idx >= (uint32)RBDANCHORCOM_TP_NUM_MSG ) /*AXIVION Line CertC-INT00 : Casting same sized integral types*/
    {
        rbdAnchorCom_ReportError( (uint32)PduId, RBD_ANCHORCOM_API_TP_TRANSMIT, RBD_ANCHORCOM_ERR_PDU );
    }
    else if( OtherEndIdx >= RBDANCHORCOM_TP_NUM_LL_TX)    /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : defensive code for input checking */
    {
        rbdAnchorCom_ReportError( OtherEndIdx, RBD_ANCHORCOM_API_TP_GETBUFFER, RBD_ANCHORCOM_ERR_OTHERENDIDX );
    }
    else if( TotalLength > rbdAnchorComTp_MsgCfg[Idx].Length )
    {
        rbdAnchorCom_ReportError( Length, RBD_ANCHORCOM_API_TP_TRANSMIT, RBD_ANCHORCOM_ERR_LENGTH );
    }
    else if( (NULL == Buf) || ((Length2 > 0u) && (NULL == Buf2)) ) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        rbdAnchorCom_ReportError( 0, RBD_ANCHORCOM_API_TP_TRANSMIT, RBD_ANCHORCOM_ERR_NULL );
    }
    else
    {
        // Broadcast messages use a special broadcast target.
        uint8 OtherEndIdxToUse = OtherEndIdx;
        if( rbdAnchorComTp_MsgCfg[Idx].Broadcast == TRUE )
        {
            OtherEndIdxToUse = RBD_PK_MAX_NODES;
        }

        SuspendAllInterrupts();
        if(   (RBDANCHORCOM_TPSTATE_IDLE == rbdAnchorComTp_MsgTxInfo[OtherEndIdxToUse][Idx].BufState)/*AXIVION Line CertC-STR34 : comparison is of same types*/
           || ((RBDANCHORCOM_TPSTATE_HASDATA == rbdAnchorComTp_MsgTxInfo[OtherEndIdxToUse][Idx].BufState) && (TRUE==OverwriteOldData)))/*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            // Copy the data
            memcpy( &rbdAnchorComTp_MsgCfg[Idx].Buffer[OtherEndIdxToUse][RBDANCHORCOM_HEADER_LEN], Buf, Length ); /*AXIVION Line MisraC2012-21.18: size ok*/
            if( Length2 > 0u ) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
            {
                memcpy( &rbdAnchorComTp_MsgCfg[Idx].Buffer[OtherEndIdxToUse][RBDANCHORCOM_HEADER_LEN+Length], Buf2, Length2 ); /*AXIVION Line MisraC2012-21.18: size ok*/
            }

            // Store the length
            rbdAnchorComTp_MsgTxInfo[OtherEndIdxToUse][Idx].Length = TotalLength;
            rbdAnchorComTp_MsgTxInfo[OtherEndIdx][Idx].BufState = RBDANCHORCOM_TPSTATE_HASDATA;

            // Put this message into TX queue
            if (E_OK == rbdAnchorComTp_AddTxQueue(OtherEndIdxToUse, (uint8)Idx))
            {
                // Successfully added to the queue
                rbdAnchorComTp_MsgTxInfo[OtherEndIdxToUse][Idx].BufState = RBDANCHORCOM_TPSTATE_HASDATA;
                result = E_OK;
            }

            AnchorComLog(DET_MODULEID_ANCHORCOM, "rbdAnchorCom_Transmit pdu %d len %d\r\n", PduId, Length);
            AnchorCom_PrintData ("Tp_Transmit", &rbdAnchorComTp_MsgCfg[Idx].Buffer[OtherEndIdx][RBDANCHORCOM_HEADER_LEN], TotalLength);
        }
        ResumeAllInterrupts();
    }
    return result;
}

/**********************************************************************************************************************/
/** Get the channel ID of the message.
 *
 * The Chan ID is pre-pended to the message before MAC calculation.
 * Effectively, it is combination of CAN channel and ANchor ID.
 * This guards against an attacker changing data from one CAN ID to another.
 */
/**********************************************************************************************************************/
static uint8 rbdAnchorComTp_GetChanId(
        uint8   const CanIdx,       //!< CanIdx of the node (master is 0)
        boolean const IsBroadcast,  //!< Is this a broadcast message?
        boolean const IsBleChannel, //!< Is this for BLE TP channel?
        boolean const FromMaster    //!< Is this sent by master?
        )
{
    uint8 ChanId = CanIdx & 0x0Fu;

    if( IsBroadcast == TRUE )
    {
        ChanId |= 0x80u;
    }

    if(IsBleChannel == TRUE) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        ChanId |= 0x40u;
    }
    else
    {
        ChanId |= 0x10u;
    }

    if( FromMaster == TRUE )
    {
        ChanId |= 0x20u;
    }

    return ChanId;
}

#if defined(RBD_PK_CPU_CC26xx)
static uint8 rbdAnchorComTp_GetTxChanId(
    rbdAnchorComTp_Queue_st* const QueuePtr
        )
{
    return rbdAnchorComTp_GetChanId(
           NvMBuf_AnchorCanIdx.Anchor_CanIdx,
           rbdAnchorComTp_MsgCfg[QueuePtr->MsgIdx].Broadcast,
           TRUE,   // This is BLE channel
           FALSE    // from BLE
           );
}
#elif defined(RBD_PK_CPU_NCJ29D5)
static uint8 rbdAnchorComTp_GetTxChanId(
    rbdAnchorComTp_Queue_st* const QueuePtr
        )
{
    return rbdAnchorComTp_GetChanId(
           rbdUwbInfo_GetCANIdx(),
           rbdAnchorComTp_MsgCfg[QueuePtr->MsgIdx].Broadcast,
           FALSE,  // This is UWB channel
           FALSE    // from UWB
           );
}
#elif defined(RBD_PK_MASTER_MCU)
static uint8 rbdAnchorComTp_GetTxChanId(
    rbdAnchorComTp_Queue_st* const QueuePtr
        )
{
    return rbdAnchorComTp_GetChanId(
           rbdAnchorComTp_MsgCfg[QueuePtr->MsgIdx].Broadcast ? 0u : QueuePtr->AnchorId+1u, // Use id 0 for broadcast messages
           rbdAnchorComTp_MsgCfg[QueuePtr->MsgIdx].Broadcast,
           (rbdAnchorComTp_MsgCfg[QueuePtr->MsgIdx].Channel == RBDANCHORCOM_TPCH_BLE) ? TRUE : FALSE,
           TRUE    // from MASTER
           );
}
#else
#error
#endif

//------------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_PrepareTxData
//! Before we process any TX messages, we need to prepare following data :
//!   1 byte channel ID
//!   If encryption is required, add padding bytes to make payload 16 bytes aligned
//!     Save the padding information if needed
//!   8 bytes freshness value
//------------------------------------------------------------------------------
static rbdAnchorCom_Result_et rbdAnchorComTp_TxFormatForCrypto(
        rbdAnchorComTp_Queue_st* const QueuePtr     //!< Pointer to the queue entry
        )
{
    rbdAnchorCom_Result_et ret = RBDANCHORCOM_FATAL_FAIL;

    uint8   ChanId  = 0u, TruncFresh = 0;
	uint32  FreshLen = RBDANCHORCOM_FRESH_LEN;
	uint8   FreshArray[RBDANCHORCOM_FRESH_LEN];

    if (NULL == QueuePtr) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        rbdAnchorCom_ReportError(0, RBD_ANCHORCOM_API_TX_FORMAT, RBD_ANCHORCOM_ERR_NULL);
    }
    else if (E_OK == rbdAnchorCom_Fresh_GetTxFreshnessTruncData(FreshArray, FreshLen, &TruncFresh))
    {
        uint8   MsgIdx  = QueuePtr->MsgIdx, PeerIdx = QueuePtr->AnchorId;
        uint16  PayloadLen = rbdAnchorComTp_MsgTxInfo[PeerIdx][MsgIdx].Length;
        uint8*  Buffer = rbdAnchorComTp_MsgCfg[MsgIdx].Buffer[PeerIdx];

        // Get Freshness value
        // Although we only save the freshness value at the last part of the buffer
        // We need to check the result first before wasting time on saving other information
        // Calculate the Channel ID
        ChanId = rbdAnchorComTp_GetTxChanId(QueuePtr);

        // Fill in ChannelID
        Buffer[0u] = ChanId; /* AXIVION Line MisraC2012Directive-4.1, CertC-EXP34 : pointer not NULL */

        // Fill the PDU ID
        Buffer[1u] = rbdAnchorComTp_MsgCfg[MsgIdx].PduId;

        // Padding zero if encryption is needed, and data is not 16B aligned
        if (rbdAnchorComTp_MsgCfg[MsgIdx].EncDecFlag == TRUE)
        {
            uint8_t Padding = (uint8)((PayloadLen) % RBDANCHORCOM_AES_KEY_LEN);

            // Encrypt flag
            Buffer[2u] = 0x80u;

            // Now the length is pure payload length
            if (0u != Padding)
            {
                Padding = RBDANCHORCOM_AES_KEY_LEN - Padding;
                for (uint8 i = 0u; (i < Padding); i++)
                {
                    Buffer[PayloadLen + i + RBDANCHORCOM_HEADER_LEN] = 0u;  // Add padding data to make it 16 bytes aligned
                }

                /*AXIVION Next Line MisraC2012Directive-4.1, CertC-INT08 : newLen is smaller than 284, add to a value less than 16, no overflow */
                PayloadLen += Padding;
                rbdAnchorComTp_MsgTxInfo[PeerIdx][MsgIdx].Length += Padding; /*AXIVION Line MisraC2012Directive-4.1, CertC-INT08 : no overflow */

                // Fill in the pad information
                Buffer[2u] |= Padding & 0x0Fu;
            }
            else
            {
                // Fill in the pad information
                Buffer[2u] = 0u;
            }

        }

        // Fill in the freshness value
        // Buffer is payload length + RBDANCHORCOM_TP_MAX_PADDING_LEN, so no overflow risk
        memcpy (&Buffer[PayloadLen + RBDANCHORCOM_HEADER_LEN], FreshArray, FreshLen);
#if defined(RBD_PK_MASTER_MCU) && (STD_ON == RBDANCHORCOM_TEST)
        // If test mode is enabled, we want to see freshness value used for CMAC
        if( (rbdAnchorComTp_MsgCfg[MsgIdx].PduId == rbdAnchorCom_TpPdu_BleTest)
            || (rbdAnchorComTp_MsgCfg[MsgIdx].PduId == rbdAnchorCom_TpPdu_UwbTest)
            || (rbdAnchorComTp_MsgCfg[MsgIdx].PduId == rbdAnchorCom_TpPdu_BroadCastTest))
        {
            // In test cyclic, it already set value: [0]->sequence , [1-4]->test time
            memcpy (&Buffer[RBDANCHORCOM_HEADER_LEN + 5u], &FreshArray[4u], 4u);
        }

#endif
        QueuePtr->TruncFresh = TruncFresh;

        AnchorCom_PrintData ("TxFormatForCrypto", &Buffer[RBDANCHORCOM_HEADER_LEN], PayloadLen);

        if (rbdAnchorComTp_MsgCfg[MsgIdx].EncDecFlag == TRUE)
        {
            QueuePtr->QueueState = RBDANCHORCOM_TX_ENC_READY;
        }
        else
        {
            QueuePtr->QueueState = RBDANCHORCOM_TX_CMAC_READY;
        }

        ret = RBDANCHORCOM_OK;
    }
    else
    {
        ret = RBDANCHORCOM_FAIL;
    }

    return ret;
}

//------------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_EncryptData
//!
//! Call CSM encrypt function to encrypt the payload
//------------------------------------------------------------------------------
static rbdAnchorCom_Result_et rbdAnchorComTp_TxStartEncryption(
    rbdAnchorComTp_Queue_st* const QueuePtr     //!< Pointer to the queue entry
    )
{
    rbdAnchorCom_Result_et ret = RBDANCHORCOM_FATAL_FAIL;

    if (NULL == QueuePtr) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        rbdAnchorCom_ReportError(0, RBD_ANCHORCOM_API_TX_FORMAT, RBD_ANCHORCOM_ERR_NULL);
    }
    else
    {
        uint8   MsgIdx  = QueuePtr->MsgIdx, PeerIdx = QueuePtr->AnchorId;
        uint8 * PayloadPtr = rbdAnchorComTp_MsgCfg[MsgIdx].Buffer[PeerIdx] + RBDANCHORCOM_HEADER_LEN;
        uint16  PayloadLen = rbdAnchorComTp_MsgTxInfo[PeerIdx][MsgIdx].Length;
        uint8 * FreshPtr = PayloadPtr + PayloadLen;

        ret = rbdAnchorComCsm_Encrypt(PayloadPtr, PayloadLen, FreshPtr);
    }

    return ret;
}

#if RBDANCHORCOM_CSM_ASYNC_CALLS == TRUE
//----------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_TxEncryptFinish
//!
//! In Async mode, CSM will call this functioin when decryption is finished
//----------------------------------------------------------------------------
void rbdAnchorComTp_TxEncryptFinish(uint32 OtherEndIdx)
{
    rbdAnchorComTp_Queue_st *TxHeadPtr = rbdAnchorComTp_GetTxQueueHead(OtherEndIdx);

    if (NULL != TxHeadPtr) && (TxHeadPtr->QueueState == RBDANCHORCOM_TX_ENC_RUNNING)
    {
        TxHeadPtr->QueueState = RBDANCHORCOM_TX_CMAC_READY;
    }
}
#endif

//------------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_AuthenData
//!
//! Call CSM MAC generate function to calculate CMAC vlaue
//------------------------------------------------------------------------------
static rbdAnchorCom_Result_et rbdAnchorComTp_TxStartCmac(
    rbdAnchorComTp_Queue_st* const QueuePtr     //!< Pointer to the queue entry
    )
{
    rbdAnchorCom_Result_et ret = RBDANCHORCOM_FATAL_FAIL;

    if (NULL == QueuePtr) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        rbdAnchorCom_ReportError(0, RBD_ANCHORCOM_API_TX_FORMAT, RBD_ANCHORCOM_ERR_NULL);
    }
    else
    {
        uint8 * DataPtr = rbdAnchorComTp_MsgCfg[QueuePtr->MsgIdx].Buffer[QueuePtr->AnchorId];
        /*AXIVION Next Line MisraC2012Directive-4.1, CertC-INT08 : no overflow */
        uint16  DataLen = rbdAnchorComTp_MsgTxInfo[QueuePtr->AnchorId][QueuePtr->MsgIdx].Length + RBDANCHORCOM_HEADER_LEN + RBDANCHORCOM_FRESH_LEN;
        uint8 * CmacPtr = QueuePtr->CmacValue;

        ret = rbdAnchorComCsm_Authenticate(DataPtr, DataLen, CmacPtr);
    }

    return ret;
}

#if RBDANCHORCOM_CSM_ASYNC_CALLS == TRUE
//----------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_TxCmacFinish
//!
//! In Async mode, CSM will call this functioin when CAMC generation is finished
//----------------------------------------------------------------------------
void rbdAnchorComTp_TxCmacFinish(uint32 OtherEndIdx)
{
    rbdAnchorComTp_Queue_st *TxHeadPtr = rbdAnchorComTp_GetTxQueueHead(OtherEndIdx);

    if (NULL != TxHeadPtr) && (TxHeadPtr->QueueState == RBDANCHORCOM_TX_CMAC_RUNNING)
    {
        TxHeadPtr->QueueState = RBDANCHORCOM_TX_TRANS_READY;
    }
}
#endif

//------------------------------------------------------------------------------
//! \brief  rbdAnchorCom_TransmitData
//!
//! Call PDUR transmit to send out message
//! If this is a broadcast message, also need to send to IMC
//------------------------------------------------------------------------------
static rbdAnchorCom_Result_et rbdAnchorComTp_TxStartTx(
    rbdAnchorComTp_Queue_st* const QueuePtr     //!< Pointer to the queue entry
    )
{
    rbdAnchorCom_Result_et ret = RBDANCHORCOM_FATAL_FAIL;

    if (NULL == QueuePtr) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        rbdAnchorCom_ReportError(0, RBD_ANCHORCOM_API_TX_FORMAT, RBD_ANCHORCOM_ERR_NULL);
    }
    else
    {
        PduInfoType info;
        PduIdType PduId;


        // Build parameters for PduR
        info.SduLength = rbdAnchorComTp_MsgTxInfo[QueuePtr->AnchorId][QueuePtr->MsgIdx].Length;
        info.SduDataPtr = NULL;  // No metadata

        rbdAnchorComTp_ChTxInfo[QueuePtr->Channel][QueuePtr->AnchorId].TxInProgress = TRUE;
        rbdAnchorComTp_ChTxInfo[QueuePtr->Channel][QueuePtr->AnchorId].TxOffset = 0u;
        rbdAnchorComTp_ChTxInfo[QueuePtr->Channel][QueuePtr->AnchorId].TxMsgIdx = QueuePtr->MsgIdx;


        // Call TP transmit function to send out
        PduId = rbdAnchorComTp_LowerTxCallPdu[QueuePtr->Channel][QueuePtr->AnchorId];
        AnchorComLog(DET_MODULEID_ANCHORCOM, "Tp_TxStartTx chan %d anchor id %d pdu %d len %d\r\n", QueuePtr->Channel, QueuePtr->AnchorId, PduId, info.SduLength);

        if( E_OK == PduR_rbdAnchorComTpTransmit(PduId, &info) )
        {
            ret = RBDANCHORCOM_OK;
        }

#if STD_ON==RBD_PK_MASTER_HAS_RF
        // For broadcast messages, it should be sent on both CAN and IMC, the previous code will send broadcast on CAN
        // We need to copy the buffer and call PDUR for IMC
        // So TP_copyTX will use two callback PDU ID for copying data from same message
        if (rbdAnchorComTp_MsgCfg[QueuePtr->MsgIdx].Broadcast)
        {
            uint8 ImcBroadcastId = QueuePtr->AnchorId + 1u; // QueuePtr->AnchorId is set to MAX_NODE, so IMC is last one

            rbdAnchorComTp_ChTxInfo[QueuePtr->Channel][ImcBroadcastId].TxInProgress = TRUE;
            rbdAnchorComTp_ChTxInfo[QueuePtr->Channel][ImcBroadcastId].TxOffset = 0u;
            rbdAnchorComTp_ChTxInfo[QueuePtr->Channel][ImcBroadcastId].TxMsgIdx = QueuePtr->MsgIdx;

            // Call TP transmit function to send out
            PduId = rbdAnchorComTp_LowerTxCallPdu[QueuePtr->Channel][ImcBroadcastId];

            if( E_OK == PduR_rbdAnchorComTpTransmit(PduId, &info) )
            {
                ret = RBDANCHORCOM_OK;
            }

        }
#endif
    }

    return ret;
}

//------------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_FinishData
//!
//! Free the buffer when data is finished being process, release the queue head
//------------------------------------------------------------------------------
static rbdAnchorCom_Result_et rbdAnchorComTp_FinishData(
    rbdAnchorComTp_Queue_st* const QueuePtr     //!< Pointer to the queue entry
    )
{
    rbdAnchorCom_Result_et ret = RBDANCHORCOM_FATAL_FAIL;

    if (NULL == QueuePtr) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        rbdAnchorCom_ReportError(0, RBD_ANCHORCOM_API_TX_FORMAT, RBD_ANCHORCOM_ERR_NULL);
    }
    else
    {
        // Release the buffer
        rbdAnchorComTp_MsgTxInfo[QueuePtr->AnchorId][QueuePtr->MsgIdx].BufState = RBDANCHORCOM_TPSTATE_IDLE;
        // Remove head
        (void)rbdAnchorComTp_RemoveTxQueueHead(QueuePtr->AnchorId);

        ret = RBDANCHORCOM_OK;
    }

    return ret;
}


//------------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_TxPart1
//!
//! Handle tx part one, mainly prepare data and encryptoin if needed
//------------------------------------------------------------------------------
static rbdAnchorCom_Result_et rbdAnchorComTp_TxPart1(
    rbdAnchorComTp_Queue_st* const QueuePtr     //!< Pointer to the queue entry
    )
{
    rbdAnchorCom_Result_et ret = RBDANCHORCOM_FATAL_FAIL;
    if (NULL == QueuePtr) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        rbdAnchorCom_ReportError(0, RBD_ANCHORCOM_API_TX_FORMAT, RBD_ANCHORCOM_ERR_NULL);
    }
    else if (QueuePtr->QueueState == RBDANCHORCOM_TX_QUEUED)/*AXIVION Line CertC-STR34 : comparison is of same types*/
    {
        // Data is just queued, so we need to gather all information we need
        ret = rbdAnchorComTp_TxFormatForCrypto(QueuePtr);
        if (RBDANCHORCOM_OK != ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            QueuePtr->ErrCounter++;
        }

        // Data is ready for encryption, call enc
        if (QueuePtr->QueueState == RBDANCHORCOM_TX_ENC_READY)/*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            ret = rbdAnchorComTp_TxStartEncryption(QueuePtr);

            if (RBDANCHORCOM_OK == ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
            {
                QueuePtr->QueueState = RBDANCHORCOM_TX_CMAC_READY;
            }
            else if (RBDANCHORCOM_FAIL == ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
            {
                QueuePtr->ErrCounter++;
            }
            else if (RBDANCHORCOM_PENDING == ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
            {
                QueuePtr->QueueState = RBDANCHORCOM_TX_ENC_RUNNING;
            }
            else // RBDANCHORCOM_FATAL_FAIL
            {
                // Make the error counter max so that it will be dropped
                QueuePtr->ErrCounter += RBDANCHORCOM_MAX_RETRY_COUNT; /* AXIVION Line MisraC2012Directive-4.1, CertC-INT08: no overflow */
            }
        }

        AnchorComLog(DET_MODULEID_ANCHORCOM, "rbdAnchorComTp_TxPart1 ret %d\r\n", ret);
    }
    else
    {
        // Nothing to do, it's valid result, so return OK
        ret = RBDANCHORCOM_OK;
    }

    return ret;
}

//------------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_TxPart2
//!
//! Tx part 2, calculate CMAC value of whole message
//------------------------------------------------------------------------------
static rbdAnchorCom_Result_et rbdAnchorComTp_TxPart2(
    rbdAnchorComTp_Queue_st* const QueuePtr     //!< Pointer to the queue entry
    )
{
    rbdAnchorCom_Result_et ret = RBDANCHORCOM_FATAL_FAIL;

    if (NULL == QueuePtr) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        rbdAnchorCom_ReportError(0, RBD_ANCHORCOM_API_TX_FORMAT, RBD_ANCHORCOM_ERR_NULL);
    }
    else if (QueuePtr->QueueState == RBDANCHORCOM_TX_CMAC_READY)/*AXIVION Line CertC-STR34 : comparison is of same types*/
    {
        // Data is ready for CMAC, call cmac
        ret = rbdAnchorComTp_TxStartCmac(QueuePtr);
        if (RBDANCHORCOM_OK == ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            // update CMAC and length
            uint8* DataPtr = rbdAnchorComTp_MsgCfg[QueuePtr->MsgIdx].Buffer[QueuePtr->AnchorId];
            /* AXIVION Next Line MisraC2012Directive-4.1, CertC-INT08: no overflow */
            uint16 CmacOffset = rbdAnchorComTp_MsgTxInfo[QueuePtr->AnchorId][QueuePtr->MsgIdx].Length + RBDANCHORCOM_HEADER_LEN;

            /* AXIVION Next Line MisraC2012Directive-4.1, CertC-EXP34: not NULL */
            DataPtr[CmacOffset] = QueuePtr->TruncFresh; // Save 1B truncated freshness
            memcpy(&DataPtr[CmacOffset + 1u], QueuePtr->CmacValue, RBDANCHORCOM_CMAC_LEN);   // Save 3B CMAC value

            // Actual length for transmission is payload + 2B prefix + 4B CMAC and Fresh
            /* AXIVION Next Line MisraC2012Directive-4.1, CertC-INT08: no overflow */
            rbdAnchorComTp_MsgTxInfo[QueuePtr->AnchorId][QueuePtr->MsgIdx].Length += RBDANCHORCOM_APPEND_LEN + RBDANCHORCOM_PREFIX_LEN;

            QueuePtr->QueueState = RBDANCHORCOM_TX_TRANS_READY;
        }
        else if (RBDANCHORCOM_PENDING == ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            QueuePtr->QueueState = RBDANCHORCOM_TX_CMAC_RUNNING;
        }
        else if (RBDANCHORCOM_FAIL == ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            QueuePtr->ErrCounter++;
        }
        else // RBDANCHORCOM_FATAL_FAIL
        {
            // Make the error counter max so that it will be dropped
            QueuePtr->ErrCounter += RBDANCHORCOM_MAX_RETRY_COUNT; /* AXIVION Line MisraC2012Directive-4.1, CertC-INT08: no overflow */
        }

        AnchorComLog(DET_MODULEID_ANCHORCOM, "rbdAnchorComTp_TxPart2 ret %d\r\n", ret);
    }
    else
    {
        // Nothing to do, it's valid result, so return OK
        ret = RBDANCHORCOM_OK;
    }

    return ret;
}

//------------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_TxPart3
//!
//! TX part3, call the lower PDUR transmit function to send package
//------------------------------------------------------------------------------
static rbdAnchorCom_Result_et rbdAnchorComTp_TxPart3(
    rbdAnchorComTp_Queue_st* const QueuePtr     //!< Pointer to the queue entry
    )

{
    rbdAnchorCom_Result_et ret = RBDANCHORCOM_FATAL_FAIL;


     if (NULL == QueuePtr) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        rbdAnchorCom_ReportError(0, RBD_ANCHORCOM_API_TX_FORMAT, RBD_ANCHORCOM_ERR_NULL);
    }
    else if (QueuePtr->QueueState == RBDANCHORCOM_TX_TRANS_READY)/*AXIVION Line CertC-STR34 : comparison is of same types*/
    {
        QueuePtr->QueueState = RBDANCHORCOM_TX_TRANS_RUNNING;
        // Data is ready for transmission, start PDUR transmit
        ret = rbdAnchorComTp_TxStartTx(QueuePtr);

        if (RBDANCHORCOM_OK == ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            QueuePtr->QueueState = RBDANCHORCOM_TX_TRANS_RUNNING;
        }
        else if (RBDANCHORCOM_FAIL == ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            QueuePtr->ErrCounter++;
        }
        else // RBDANCHORCOM_FATAL_FAIL
        {
            // Make the error counter max so that it will be dropped
            QueuePtr->ErrCounter += RBDANCHORCOM_MAX_RETRY_COUNT; /* AXIVION Line MisraC2012Directive-4.1, CertC-INT08: no overflow */
        }

        AnchorComLog(DET_MODULEID_ANCHORCOM, "rbdAnchorComTp_TxPart3 ret %d\r\n", ret);
    }
    else
    {
        // Nothing to do, it's valid result, so return OK
        ret = RBDANCHORCOM_OK;
    }

    return ret;
}


#if defined(RBD_PK_CPU_CC26xx)
static uint8 rbdAnchorComTp_GetRxChanId(
    rbdAnchorComTp_RxQueue_st* const QueuePtr
        )
{
    return rbdAnchorComTp_GetChanId(
           NvMBuf_AnchorCanIdx.Anchor_CanIdx,
           rbdAnchorComTp_MsgCfg[QueuePtr->MsgIdx].Broadcast,
           TRUE,   // This is BLE channel
           TRUE    // from Master
           );
}
#elif defined(RBD_PK_CPU_NCJ29D5)
static uint8 rbdAnchorComTp_GetRxChanId(
    rbdAnchorComTp_RxQueue_st* const QueuePtr
        )
{
    return rbdAnchorComTp_GetChanId(
           rbdAnchorComTp_MsgCfg[QueuePtr->MsgIdx].Broadcast ? 0u : rbdUwbInfo_GetCANIdx(), /* AXIVION Line MisraC2012-10.1: checked ok */
           rbdAnchorComTp_MsgCfg[QueuePtr->MsgIdx].Broadcast,
           FALSE,  // This is UWB channel
           TRUE    // from Master
           );
}
#elif defined(RBD_PK_MASTER_MCU)
static uint8 rbdAnchorComTp_GetRxChanId(
    rbdAnchorComTp_RxQueue_st* const QueuePtr
        )
{
    return rbdAnchorComTp_GetChanId(
           QueuePtr->AnchorId + 1,
           rbdAnchorComTp_MsgCfg[QueuePtr->MsgIdx].Broadcast,
           (rbdAnchorComTp_MsgCfg[QueuePtr->MsgIdx].Channel == RBDANCHORCOM_TPCH_BLE) ? TRUE : FALSE,
           FALSE    // from Anchor
           );
}
#else
#error
#endif

//------------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_PrepareRxData
//!
//! We need to prepare the following information before we can process any RX message
//!   1 byte channel ID for this message
//!   Save the Freshness value and CMAC value we received
//!   Calculated 8 bytes freshness value from RX truncated fresh byte
//------------------------------------------------------------------------------
static rbdAnchorCom_Result_et rbdAnchorComTp_RxFormatForCmac(
        rbdAnchorComTp_RxQueue_st* const QueuePtr     //!< Pointer to the queue entry
        )
{
    rbdAnchorCom_Result_et ret = RBDANCHORCOM_FATAL_FAIL;
    uint8   ChanId  = 0u;
	uint32  FreshLen = RBDANCHORCOM_FRESH_LEN;
	uint8   FreshArray[RBDANCHORCOM_FRESH_LEN];
	uint8   PduId;
    uint32 MsgIdx;

    if (NULL == QueuePtr) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        rbdAnchorCom_ReportError(0, RBD_ANCHORCOM_API_TX_FORMAT, RBD_ANCHORCOM_ERR_NULL);
    }
    else
    {
        PduId = QueuePtr->BufferPtr[RBDANCHORCOM_CHANID_LEN];
        MsgIdx = rbdAnchorComTp_FindIdx( PduId );
        if( MsgIdx < (uint32)RBDANCHORCOM_TP_NUM_MSG) /*AXIVION Line CertC-INT00 : Casting same sized integral types*/
        {
            QueuePtr->MsgIdx = (uint8)MsgIdx;

    	    // Although we already checked in RxIndication, defensive checking
    	    if (QueuePtr->DataLen > 4u)
    	    {
    	        QueuePtr->TruncFresh = QueuePtr->BufferPtr[QueuePtr->DataLen - RBDANCHORCOM_APPEND_LEN]; /* AXIVION Line MisraC2012Directive-4.1, CertC-INT08: checked ok */
    	        memcpy(QueuePtr->CmacValue, &QueuePtr->BufferPtr[QueuePtr->DataLen - RBDANCHORCOM_CMAC_LEN], RBDANCHORCOM_CMAC_LEN); /* AXIVION Line MisraC2012Directive-4.1, CertC-INT08: checked ok */
    	    }

    	    if (E_OK == rbdAnchorCom_Fresh_GetRxFreshness(QueuePtr->TruncFresh, FreshArray, FreshLen))
    	    {
    	        // Calculate the Channel ID
    	        ChanId = rbdAnchorComTp_GetRxChanId(QueuePtr);

    	        // Fill in ChannelID
    	        QueuePtr->BufferPtr[0u] = ChanId;

    	        // Fill in the freshness value,
    	        // Truncated Fresh and CMAC are copied out, so we can overwrite here
    	        memcpy (&QueuePtr->BufferPtr[QueuePtr->DataLen - RBDANCHORCOM_APPEND_LEN], FreshArray, FreshLen); /* AXIVION Line MisraC2012Directive-4.1, CertC-INT08: checked ok */
    	        QueuePtr->DataLen += RBDANCHORCOM_APPEND_LEN; /* AXIVION Line MisraC2012Directive-4.1, CertC-INT08: no overflow */

    	        QueuePtr->QueueState = RBDANCHORCOM_RX_CMAC_READY;
                ret = RBDANCHORCOM_OK;
    	    }
    	    else
    	    {
                AnchorComLog(DET_MODULEID_ANCHORCOM, "\033[0;31m Fresh_GetRxFreshnes FAIL \033[0m\r\n");
    	        ret = RBDANCHORCOM_FAIL;
    	    }
    	}
    }
    return ret;
}

//------------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_VerifyData
//!
//! Call CSM CMAC verification function to check the validity of message
//------------------------------------------------------------------------------
static rbdAnchorCom_Result_et rbdAnchorComTp_RxStartCmac(
    rbdAnchorComTp_RxQueue_st* const QueuePtr     //!< Pointer to the queue entry
    )
{
    rbdAnchorCom_Result_et ret = RBDANCHORCOM_FATAL_FAIL;

    if (NULL == QueuePtr) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        rbdAnchorCom_ReportError(0, RBD_ANCHORCOM_API_TX_FORMAT, RBD_ANCHORCOM_ERR_NULL);
    }
    else
    {
        uint16 DataLen = QueuePtr->DataLen;
        uint8* DataPtr = QueuePtr->BufferPtr;
        uint8* CmacPtr = QueuePtr->CmacValue;

        ret = rbdAnchorComCsm_Verify(QueuePtr->AnchorId, DataPtr, DataLen, CmacPtr);
    }

    return ret;
}

#if RBDANCHORCOM_CSM_ASYNC_CALLS == TRUE
//----------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_RxCmacFinish
//!
//! In Async mode, CSM will call this functioin when CAMC verification is finished
//----------------------------------------------------------------------------
void rbdAnchorComTp_RxCmacFinish(uint32 OtherEndIdx)
{
    rbdAnchorComTp_RxQueue_st* RxHeadPtr = rbdAnchorComTp_GetRxQueueHead(OtherEndIdx);

    if (NULL != RxHeadPtr) && (RxHeadPtr->QueueState == RBDANCHORCOM_RX_CMAC_RUNNING)
    {
        RxHeadPtr->QueueState = RBDANCHORCOM_RX_DEC_READY;
    }
}
#endif

//------------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_DecryptData
//!
//! Call CSM AES CBC function to decrypt the message paylod
//------------------------------------------------------------------------------
static rbdAnchorCom_Result_et rbdAnchorComTp_RxStartDecryption(
    rbdAnchorComTp_RxQueue_st* const QueuePtr     //!< Pointer to the queue entry
    )
{
    rbdAnchorCom_Result_et ret = RBDANCHORCOM_FATAL_FAIL;

    if (NULL == QueuePtr) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        rbdAnchorCom_ReportError(0, RBD_ANCHORCOM_API_TX_FORMAT, RBD_ANCHORCOM_ERR_NULL);
    }
    else
    {
        uint8* PayloadPtr = &QueuePtr->BufferPtr[RBDANCHORCOM_HEADER_LEN];
        uint16 PayloadLen = QueuePtr->DataLen - RBDANCHORCOM_HEADER_LEN; /* AXIVION Line MisraC2012Directive-4.1, CertC-INT08: checked ok */
        uint8* FreshPtr   = PayloadPtr + PayloadLen;

        ret = rbdAnchorComCsm_Decrypt(QueuePtr->AnchorId, PayloadPtr, PayloadLen, FreshPtr);
    }

    return ret;
}

#if RBDANCHORCOM_CSM_ASYNC_CALLS == TRUE
//----------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_RxDecFinish
//!
//! In Async mode, CSM will call this functioin when decryptioin is finished
//----------------------------------------------------------------------------
void rbdAnchorComTp_RxDecFinish(uint32 OtherEndIdx)
{
    rbdAnchorComTp_RxQueue_st *RxHeadPtr = rbdAnchorComTp_GetRxQueueHead(OtherEndIdx);

    if (NULL != RxHeadPtr) && (RxHeadPtr->QueueState == RBDANCHORCOM_RX_DEC_RUNNING)
    {
        RxHeadPtr->QueueState = RBDANCHORCOM_RX_PROCESS_READY;
    }
}
#endif


//------------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_ProcessData
//!
//! Call upper layer callback to process this message
//------------------------------------------------------------------------------
static rbdAnchorCom_Result_et rbdAnchorComTp_RxDistributeMsg(
    rbdAnchorComTp_RxQueue_st* const QueuePtr     //!< Pointer to the queue entry
    )
{
    rbdAnchorCom_Result_et ret = RBDANCHORCOM_FATAL_FAIL;

    if (NULL == QueuePtr) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        rbdAnchorCom_ReportError(0, RBD_ANCHORCOM_API_TX_FORMAT, RBD_ANCHORCOM_ERR_NULL);
    }
    else
    {
        rbdAnchorCom_TpRxIndication_fp RxIndication = rbdAnchorComTp_MsgCfg[QueuePtr->MsgIdx].RxIndication;
        if( NULL != RxIndication ) /*AXIVION Line MisraC2012-14.3 MisraC2012-2.2: defensive pointer checking */
        {
            /*AXIVION Next Line MisraC2012Directive-4.1, CertC-INT08 : payloadLen is multiplier of 16, PaddingLen is 0 to 15, so no overflow risk*/
            uint16 RxPayloadLen = QueuePtr->DataLen - (uint16)RBDANCHORCOM_HEADER_LEN;

            (*RxIndication)( QueuePtr->AnchorId, rbdAnchorComTp_MsgCfg[QueuePtr->MsgIdx].PduId,
                RxPayloadLen, &QueuePtr->BufferPtr[RBDANCHORCOM_HEADER_LEN] );
            ret = RBDANCHORCOM_OK;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_RxPart1
//!
//! Rx message process part 1, verify CMAC values
//------------------------------------------------------------------------------
static rbdAnchorCom_Result_et rbdAnchorComTp_RxPart1(
    rbdAnchorComTp_RxQueue_st* const QueuePtr     //!< Pointer to the queue entry
    )
{
    rbdAnchorCom_Result_et ret = RBDANCHORCOM_FATAL_FAIL;

    if (NULL == QueuePtr) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        rbdAnchorCom_ReportError(0, RBD_ANCHORCOM_API_TX_FORMAT, RBD_ANCHORCOM_ERR_NULL);
    }
    else
    {
        if (QueuePtr->QueueState == RBDANCHORCOM_RX_QUEUED)/*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            // Data is just queued, so we need to gather all information we need
            ret = rbdAnchorComTp_RxFormatForCmac(QueuePtr);
            if (RBDANCHORCOM_OK != ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
            {
                QueuePtr->ErrCounter++;
            }
        }
        // Data is ready for CMAC verification, call cmac
        if (QueuePtr->QueueState == RBDANCHORCOM_RX_CMAC_READY)/*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            ret = rbdAnchorComTp_RxStartCmac(QueuePtr);
            if (RBDANCHORCOM_OK == ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
            {
                // Now we have finished using the Freshness value, remove it from data
                QueuePtr->DataLen -= RBDANCHORCOM_FRESH_LEN; /* AXIVION Line MisraC2012Directive-4.1, CertC-INT08: checked ok */

                // Freshness value is still used in decryption, so keep it
                if (rbdAnchorComTp_MsgCfg[QueuePtr->MsgIdx].EncDecFlag == TRUE)
                {
                    QueuePtr->QueueState = RBDANCHORCOM_RX_DEC_READY;
                }
                else
                {
                    QueuePtr->QueueState = RBDANCHORCOM_RX_PROCESS_READY;
                }
            }
            else if (RBDANCHORCOM_PENDING == ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
            {
                QueuePtr->QueueState = RBDANCHORCOM_RX_CMAC_RUNNING;
            }
            else if (RBDANCHORCOM_FAIL == ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
            {
                QueuePtr->ErrCounter++;
            }
            else // RBDANCHORCOM_FATAL_FAIL
            {
                // Make the error counter max so that it will be dropped
                QueuePtr->ErrCounter += RBDANCHORCOM_MAX_RETRY_COUNT; /* AXIVION Line MisraC2012Directive-4.1, CertC-INT08: no overflow */
            }
        }

        AnchorComLog(DET_MODULEID_ANCHORCOM, "rbdAnchorComTp_RxPart1 ret %d\r\n", ret);
    }

    return ret;
}

//------------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_RxPart2
//!
//! Rx part 2, Decrypt the payload if needed
//------------------------------------------------------------------------------
static rbdAnchorCom_Result_et rbdAnchorComTp_RxPart2(
    rbdAnchorComTp_RxQueue_st* const QueuePtr     //!< Pointer to the queue entry
    )
{
    rbdAnchorCom_Result_et ret = RBDANCHORCOM_FATAL_FAIL;

    if (NULL == QueuePtr) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        rbdAnchorCom_ReportError(0, RBD_ANCHORCOM_API_TX_FORMAT, RBD_ANCHORCOM_ERR_NULL);
    }
    else if (QueuePtr->QueueState == RBDANCHORCOM_RX_DEC_READY)/*AXIVION Line CertC-STR34 : comparison is of same types*/
    {
        // Data is ready for encryption, call enc
        ret = rbdAnchorComTp_RxStartDecryption(QueuePtr);

        if (RBDANCHORCOM_OK == ret) /*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            // Now we have finished using Freshness value, remove Freshness and padding 0
            uint8 padding = QueuePtr->BufferPtr[2u] & 0x0Fu;

            QueuePtr->DataLen -= padding; /* AXIVION Line MisraC2012Directive-4.1,CertC-INT08: checked ok */
            QueuePtr->QueueState = RBDANCHORCOM_RX_PROCESS_READY;
        }
        else if (RBDANCHORCOM_PENDING == ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            QueuePtr->QueueState = RBDANCHORCOM_RX_DEC_RUNNING;
        }
        else if (RBDANCHORCOM_FAIL == ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            QueuePtr->ErrCounter++;
        }
        else // RBDANCHORCOM_FATAL_FAIL
        {
            // Make the error counter max so that it will be dropped
            QueuePtr->ErrCounter += RBDANCHORCOM_MAX_RETRY_COUNT; /* AXIVION Line MisraC2012Directive-4.1, CertC-INT08: no overflow */
        }

        AnchorComLog(DET_MODULEID_ANCHORCOM, "rbdAnchorComTp_RxPart2 ret %d\r\n", ret);
    }
    else
    {
        // Nothing to do, return OK
        ret = RBDANCHORCOM_OK;
    }

    return ret;
}

//------------------------------------------------------------------------------
//! \brief  rbdAnchorComTp_RxPart3
//!
//! Rx part 2, Call the message process function
//------------------------------------------------------------------------------
static rbdAnchorCom_Result_et rbdAnchorComTp_RxPart3(
    rbdAnchorComTp_RxQueue_st* const QueuePtr     //!< Pointer to the queue entry
    )
{
    rbdAnchorCom_Result_et ret = RBDANCHORCOM_FATAL_FAIL;

    if (NULL == QueuePtr) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3 : not always false */
    {
        rbdAnchorCom_ReportError(0, RBD_ANCHORCOM_API_TX_FORMAT, RBD_ANCHORCOM_ERR_NULL);
    }
    else if (QueuePtr->QueueState == RBDANCHORCOM_RX_PROCESS_READY)/*AXIVION Line CertC-STR34 : comparison is of same types*/
    {
        // Data is ready for transmission, start PDUR transmit
        ret = rbdAnchorComTp_RxDistributeMsg(QueuePtr);
        if (RBDANCHORCOM_OK == ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            QueuePtr->QueueState = RBDANCHORCOM_RX_PROCESS_DONE;
        }
        else if (RBDANCHORCOM_FAIL == ret)/*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            QueuePtr->ErrCounter++;
        }
        else // RBDANCHORCOM_FATAL_FAIL
        {
            // Make the error counter max so that it will be dropped
            QueuePtr->ErrCounter += RBDANCHORCOM_MAX_RETRY_COUNT; /* AXIVION Line MisraC2012Directive-4.1, CertC-INT08: no overflow */
        }

        AnchorComLog(DET_MODULEID_ANCHORCOM, "rbdAnchorComTp_RxPart3 ret %d\r\n", ret);
    }
    else
    {
        // Nothing to do, return OK
        ret = RBDANCHORCOM_OK;
    }

    return ret;
}

//------------------------------------------------------------------------------
//! \brief  Cyclic function for handling TP messages
//!
//! Now we want to transmit/process TP messages in FIFO method.
//! So every Tx/Rx message is put in a queue
//! This cyclic will process the first message in Tx queue based on its state.
//! Then it will handle the first message in RX queue.
//------------------------------------------------------------------------------
void rbdAnchorComTp_CyclicTx( void )
{
    uint32 const TransmitAllowed = rbdAnchorCom_NodeOkToTx();
    rbdAnchorComTp_Queue_st* TxHeadPtr;
    uint32  OtherEndIdx;

#if defined(RBD_PK_CPU_NCJ29D5)
    if (TRUE == R4Sec_IsCryptoFree())
#endif
    {
        for( OtherEndIdx = 0u; OtherEndIdx < (uint32)RBDANCHORCOM_TP_NUM_LL_TX; OtherEndIdx++ ) /*AXIVION Line CertC-INT00 : Casting same sized integral types*/
        {
#if (RBDANCHORCOM_TEST == STD_OFF)
            uint32 const mask = 1Lu << OtherEndIdx;    // Bitmask of which anchor

            // For Broadcast messages, they are in index RBD_PK_MAX_NODES and (RBD_PK_MAX_NODES+1), so they should be sent out regardless of the mask
            if(( 0u != (TransmitAllowed & mask) ) || (OtherEndIdx >= RBD_PK_MAX_NODES)) /* AXIVION Line MisraC2012-2.2, MisraC2012-14.3: not dead on MasterMcu build */
#endif
            {
                TxHeadPtr = rbdAnchorComTp_GetTxQueueHead(OtherEndIdx);
                // If we receive TX confirmation after last cycle, then just remove head and process next message
                if (TxHeadPtr != NULL)
                {
                    if (TxHeadPtr->QueueState == RBDANCHORCOM_TX_TRANS_DONE)/*AXIVION Line CertC-STR34 : comparison is of same types*/
                    {
                        // Remove head, process next queue item
                        (void)rbdAnchorComTp_FinishData(TxHeadPtr);
                        TxHeadPtr = rbdAnchorComTp_GetTxQueueHead(OtherEndIdx);
                    }
                    else if (TxHeadPtr->QueueState == RBDANCHORCOM_TX_TRANS_RUNNING)/*AXIVION Line CertC-STR34 : comparison is of same types*/
                    {
                        // We started TP transmit in last cycle, but it still not finished
                        // Add a counter in case missing TP TX confirmation.
                        TxHeadPtr->ErrCounter++;
                    }
                    else
                    {
                        // Do nothing
                    }
                }

                if (TxHeadPtr != NULL)
                {
                    // Format and encrypt
                    (void)rbdAnchorComTp_TxPart1(TxHeadPtr);

                    // CMAC generate
                    (void)rbdAnchorComTp_TxPart2(TxHeadPtr);

                    // Transmit
                    (void)rbdAnchorComTp_TxPart3(TxHeadPtr);

                    if (TxHeadPtr->ErrCounter >= RBDANCHORCOM_MAX_RETRY_COUNT)
                    {
                        rbdAnchorComTp_MsgTxInfo[TxHeadPtr->AnchorId][TxHeadPtr->MsgIdx].BufState = RBDANCHORCOM_TPSTATE_IDLE;
                        rbdAnchorComTp_ChTxInfo[TxHeadPtr->Channel][TxHeadPtr->AnchorId].TxInProgress = FALSE;

                        (void)rbdAnchorComTp_RemoveTxQueueHead(TxHeadPtr->AnchorId);
                    }
                }
            }
        }
    }
}


//------------------------------------------------------------------------------
//! \brief  Cyclic function for handling TP messages
//!
//! Processes queued RX messages.
//------------------------------------------------------------------------------
void rbdAnchorCom_CyclicRx( void )
{
    rbdAnchorComTp_RxQueue_st* RxHeadPtr;
    uint32  OtherEndIdx;

#if defined(RBD_PK_CPU_NCJ29D5)
    if (TRUE == R4Sec_IsCryptoFree())
#endif
    {
        for( OtherEndIdx = 0u; OtherEndIdx < (uint32)RBDANCHORCOM_TP_NUM_LL_RX; OtherEndIdx++ ) /*AXIVION Line CertC-INT00 : Casting same sized integral types*/
        {
            // Process the RX message
            RxHeadPtr = rbdAnchorComTp_GetRxQueueHead(OtherEndIdx);
            if (RxHeadPtr != NULL)
            {
                // Format and CMAC verify
                (void)rbdAnchorComTp_RxPart1(RxHeadPtr);

                // Decrypt
                (void)rbdAnchorComTp_RxPart2(RxHeadPtr);

                // Distribute
                (void)rbdAnchorComTp_RxPart3(RxHeadPtr);

                if ((RxHeadPtr->ErrCounter >= RBDANCHORCOM_MAX_RETRY_COUNT)/*AXIVION Line CertC-STR34 : comparison is of same types*/
                    || (RxHeadPtr->QueueState == RBDANCHORCOM_RX_PROCESS_DONE))/*AXIVION Line CertC-STR34 : comparison is of same types*/
                {
                    if (RxHeadPtr->IsTempBuf == TRUE)
                    {
                        rbdAnchorComTp_ChRxTemp[RxHeadPtr->Channel][RxHeadPtr->AnchorId].NeedVerify = FALSE;
                    }
                    else
                    {
                        rbdAnchorComTp_ChRxBuff[RxHeadPtr->Channel][RxHeadPtr->AnchorId].NeedVerify = FALSE;
                    }

                    (void)rbdAnchorComTp_RemoveRxQueueHead(RxHeadPtr->AnchorId);
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
//! \brief  Data copy function for transmission
//!
//! This function is called by the lower layer (PduR) to obtain the data
//! for transmission.
//!
//! This function has to build a message with the PduId in the first byte, followed
//! by the message data.
//------------------------------------------------------------------------------
FUNC(BufReq_ReturnType, RBDANCHORCOM_CODE) rbdAnchorComTp_CopyTxData(
        PduIdType id,   //!< Pdu ID
        /*AXIVION Next Line MisraC2012-8.13 : info cannot set to const as need to maintain same interface with AutoSAR */
        P2VAR(PduInfoType, AUTOMATIC, RBDANCHORCOM_APPL_DATA) info, //!< Pointer and length for copy
        /*AXIVION Next Line MisraC2012-8.13 : retry cannot set to const as need to maintain same interface with AutoSAR */
        P2VAR(RetryInfoType, AUTOMATIC, RBDANCHORCOM_APPL_DATA) retry, //!< flag to indicate whether this is a retry
        P2VAR(PduLengthType, AUTOMATIC, RBDANCHORCOM_APPL_DATA) availableDataPtr  //!< Pointere to save availabe data length after this opereation
        )
{
    uint32            Channel = 0u;
    uint32            OtherEndIdx = 0u;
    BufReq_ReturnType result = BUFREQ_E_NOT_OK;

    AnchorComLog(DET_MODULEID_ANCHORCOM, "AnchorCom_CopyTx pdu %d\r\n", id);

    if ( NULL == info ) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3: defensive pointer check */
    {
        rbdAnchorCom_ReportError( id, RBD_ANCHORCOM_API_TP_TXCOPY, RBD_ANCHORCOM_ERR_NULL );
    }
    // Find the PDU in our configuration arrays
    else if( FALSE == rbdAnchorComTp_FindByLLPdu( id, &rbdAnchorComTp_LowerTxCbPdu[0][0], RBDANCHORCOM_TP_NUM_LL_TX, &Channel, &OtherEndIdx ))
    {
        AnchorComLog(DET_MODULEID_ANCHORCOM, "AnchorCom_CopyTx no LL\r\n");
        rbdAnchorCom_ReportError( id, RBD_ANCHORCOM_API_TP_TXCOPY, RBD_ANCHORCOM_ERR_PDU );
    }
    else
    {
        // PDU found, we can process it
        uint32 const MsgIdx      = rbdAnchorComTp_ChTxInfo[Channel][OtherEndIdx].TxMsgIdx;
        uint16       Offset      = rbdAnchorComTp_ChTxInfo[Channel][OtherEndIdx].TxOffset;
        uint16       BytesToCopy = (uint16)info->SduLength, BytesRemaining;
        uint8      * Buf = info->SduDataPtr;
        rbdAnchorComTp_Queue_st* HeadPtr = rbdAnchorComTp_GetTxQueueHead(OtherEndIdx);

        AnchorComLog(DET_MODULEID_ANCHORCOM, "AnchorCom_CopyTx chan %d other %d msgid %d offset %d copy %d\r\n", Channel, OtherEndIdx, MsgIdx, Offset, BytesToCopy);

#if defined(RBD_PK_MASTER_MCU) && (STD_ON==RBD_PK_MASTER_HAS_RF)
        if ((HeadPtr == NULL) && (id != rbdAnchorComTpConf_CddPduRUpperLayerTxPdu_Master0Hi_M2A_rbdAnchorCom2PdurSrc)) // For Broadcast IMC message, there is no queue item
#else
        if (HeadPtr == NULL)   // Nothing to transmit
#endif
        {
            AnchorComLog(DET_MODULEID_ANCHORCOM, "AnchorCom_CopyTx head NULL\r\n");
            rbdAnchorCom_ReportError( id, RBD_ANCHORCOM_API_TP_TXCOPY, RBD_ANCHORCOM_ERR_NULL );
        }
        else
        {
            if( (NULL != retry) && (retry->TpDataState == TP_DATARETRY) )   /*AXIVION Line CertC-STR34 : comparison is of same types*/ /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3: defensive pointer check */
            {
                // In a retry action, the supplied offset overrides our saved value
                Offset = (uint16)retry->TxTpDataCnt;
            }


            /*AXIVION Next Line MisraC2012Directive-4.1: Length is total bytes while Offset is transmitted bytes, so no overflow */
#if defined(RBD_PK_MASTER_MCU) && (STD_ON==RBD_PK_MASTER_HAS_RF)
            if (id == rbdAnchorComTpConf_CddPduRUpperLayerTxPdu_Master0Hi_M2A_rbdAnchorCom2PdurSrc) // For Master0Hi, we copy the data from Masterhi
            {
                BytesRemaining = rbdAnchorComTp_MsgTxInfo[OtherEndIdx - 1u][MsgIdx].Length - Offset; /* AXIVION Line MisraC2012Directive-4.1: checked ok */
            }
            else
#endif
            {
                BytesRemaining = rbdAnchorComTp_MsgTxInfo[OtherEndIdx][MsgIdx].Length - Offset; /* AXIVION Line MisraC2012Directive-4.1, CertC-INT08: checked ok */
            }
            if( BytesToCopy > BytesRemaining )
            {
                BytesToCopy = BytesRemaining;
            }

            AnchorCom_PrintData ("rbdAnchorComTp_CopyTxData", &rbdAnchorComTp_MsgCfg[MsgIdx].Buffer[OtherEndIdx][Offset + RBDANCHORCOM_CHANID_LEN], BytesToCopy);

            // Copy the data
            if( Buf != NULL )   /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3: defensive pointer check */
            {
#if defined(RBD_PK_MASTER_MCU) && (STD_ON==RBD_PK_MASTER_HAS_RF)
                if (id == rbdAnchorComTpConf_CddPduRUpperLayerTxPdu_Master0Hi_M2A_rbdAnchorCom2PdurSrc) // For Master0Hi, we copy the data from Masterhi
                {
                    /*AXIVION Next Line MisraC2012-21.18 : BytesToCopy is not bigger than caller's expected length info->SduLength, so no overflow risk*/
                    memcpy( Buf, &rbdAnchorComTp_MsgCfg[MsgIdx].Buffer[OtherEndIdx - 1u][Offset + RBDANCHORCOM_CHANID_LEN], BytesToCopy );
                }
                else
#endif
                {
                    /*AXIVION Next Line MisraC2012-21.18 : BytesToCopy is not bigger than caller's expected length info->SduLength, so no overflow risk*/
                    memcpy( Buf, &rbdAnchorComTp_MsgCfg[MsgIdx].Buffer[OtherEndIdx][Offset + RBDANCHORCOM_CHANID_LEN], BytesToCopy );

                }

                /*AXIVION Next Line MisraC2012Directive-4.1, CertC-INT08: Offset and BytesToCopy are both smaller than 284, so no overflow */
                rbdAnchorComTp_ChTxInfo[Channel][OtherEndIdx].TxOffset = (uint16)(Offset + BytesToCopy);
                BytesRemaining -= BytesToCopy; /*AXIVION Line MisraC2012Directive-4.1, CertC-INT08: BytesToCopy is checked to be smaller than BytesRemaining, so no overflow */

#if defined(RBD_PK_CPU_NCJ29D5)
    			// For UWB, it always use IMC, dont wait for TX confirmation
                if (0u == BytesRemaining)
                {
                    rbdAnchorComTp_ChTxInfo[Channel][OtherEndIdx].TxInProgress = FALSE;
                    HeadPtr->QueueState = RBDANCHORCOM_TX_TRANS_DONE;
                }
#endif
            }

            *availableDataPtr = BytesRemaining;

            result = BUFREQ_OK;
        }
    }
    return result;
}


//------------------------------------------------------------------------------
//! \brief  Transmit confirmation
//!
//! When the Transmit Confirmation occurs, we can mark the transmit complete
//! and move to the next message.
//------------------------------------------------------------------------------
FUNC(void, RBDANCHORCOM_CODE) rbdAnchorComTp_TpTxConfirmation(
    PduIdType id,   //!< PDU ID
    Std_ReturnType result   //!< Result of TP transmission, success or fail
    )
{
    uint32 Channel = 0u;
    uint32 OtherEndIdx = 0u;

    AnchorComLog(DET_MODULEID_ANCHORCOM, "AnchorCom_TxConf pdu %d\r\n", id);

    if( FALSE == rbdAnchorComTp_FindByLLPdu( id, &rbdAnchorComTp_LowerTxCbPdu[0][0], RBDANCHORCOM_TP_NUM_LL_TX, &Channel, &OtherEndIdx ))
    {
        rbdAnchorCom_ReportError( id, RBD_ANCHORCOM_API_TP_TXCONFIRMATION, RBD_ANCHORCOM_ERR_PDU );
    }
    else
    {
        rbdAnchorComTp_Queue_st* HeadPtr = rbdAnchorComTp_GetTxQueueHead(OtherEndIdx);

        if (HeadPtr == NULL)   // Nothing to transmit
        {
#if defined(RBD_PK_MASTER_MCU) && (STD_ON==RBD_PK_MASTER_HAS_RF)
            if (id == rbdAnchorComTpConf_CddPduRUpperLayerTxPdu_Master0Hi_M2A_rbdAnchorCom2PdurSrc) // For Broadcast IMC message, there is no queue item
            {
                if( TRUE == rbdAnchorComTp_ChTxInfo[Channel][OtherEndIdx].TxInProgress )
                {
                    rbdAnchorComTp_ChTxInfo[Channel][OtherEndIdx].TxInProgress = FALSE;
                }
            }
            else
#endif
            {
                rbdAnchorCom_ReportError( id, RBD_ANCHORCOM_API_TP_TXCOPY, RBD_ANCHORCOM_ERR_NULL );
            }
        }
        else
        {
            // Being slightly defensive, only clearing flags if they're in the correct state.
            if( TRUE == rbdAnchorComTp_ChTxInfo[Channel][OtherEndIdx].TxInProgress )
            {
                rbdAnchorComTp_ChTxInfo[Channel][OtherEndIdx].TxInProgress = FALSE;

                if( E_OK == result )
                {
                    HeadPtr->QueueState = RBDANCHORCOM_TX_TRANS_DONE;
                }
                else
                {
                    // Retry the transmission
                    HeadPtr->QueueState = RBDANCHORCOM_TX_TRANS_READY;
                    HeadPtr->ErrCounter++;
                }
            }
        }
    }
}


//------------------------------------------------------------------------------
//! \brief  Start of reception
//!
//! This function will be called from the lower layer (PduR) when the first
//! TP message is received.
//!
//! In the reception direction, we simply receive the entire message before we
//! try to decode it. The Start of Reception function has to find the correct
//! buffer and copy in the data.
//------------------------------------------------------------------------------
FUNC(BufReq_ReturnType, RBDANCHORCOM_CODE) rbdAnchorComTp_StartOfReception(
        PduIdType id,   //!< PDU ID
        /*AXIVION Next Line MisraC2012-8.13 : info cannot set to const as need to maintain same interface with AutoSAR */
        P2VAR(PduInfoType, AUTOMATIC, RBDANCHORCOM_APPL_DATA) info,     //!< Informatioin of data pointer and length for this operation
        PduLengthType TpSduLength,  //!< Total length of incoming message
        P2VAR(PduLengthType, AUTOMATIC, RBDANCHORCOM_APPL_DATA) bufferSizePtr   //!< Pointer to save Rx buffer size
        )
{
    BufReq_ReturnType result = BUFREQ_E_NOT_OK;
    uint32            Channel = 0u;
    uint32            OtherEndIdx = 0u;

    // Ignore metadata
    (void)info;

    AnchorComLog(DET_MODULEID_ANCHORCOM, "AnchorCom_Start pdu %d\r\n", id);

    // Search for the PduId in our configuration array
    if( FALSE == rbdAnchorComTp_FindByLLPdu( id, &rbdAnchorComTp_LowerRxPdu[0][0], RBDANCHORCOM_TP_NUM_LL_RX, &Channel, &OtherEndIdx ))
    {
        rbdAnchorCom_ReportError( id, RBD_ANCHORCOM_API_TP_RXSTART, RBD_ANCHORCOM_ERR_PDU );
    }
    else
    {
        if( TpSduLength > RBDANCHORCOM_TP_MAX_RX_BUFFER_LEN )
        {
            // Total message will be too long - reject
            result = BUFREQ_E_OVFL;
        }
        else
        {
            // Reserve 1 bytes for CHAN ID
            rbdAnchorComTp_ChRxInfo[Channel][OtherEndIdx].RxOffset = RBDANCHORCOM_CHANID_LEN;
            result = BUFREQ_OK;

            if( NULL != bufferSizePtr ) /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3: defensive pointer check */
            {
                *bufferSizePtr = RBDANCHORCOM_TP_MAX_RX_BUFFER_LEN;
            }
        }
    }
    return result;
}


//------------------------------------------------------------------------------
//! \brief  Copy received data
//!
//! This function will be called from the lower layer (PduR) when a continued
//! message is received. We simply copy the data into our buffer.
//------------------------------------------------------------------------------
FUNC(BufReq_ReturnType, RBDANCHORCOM_CODE) rbdAnchorComTp_CopyRxData(
        PduIdType id,   //!< PDU ID
        /*AXIVION Next Line MisraC2012-8.13 : info cannot set to const as need to maintain same interface with AutoSAR */
        P2VAR(PduInfoType, AUTOMATIC, RBDANCHORCOM_APPL_DATA) info, //!< Informatioin of data pointer and length for this operation
        P2VAR(PduLengthType, AUTOMATIC, RBDANCHORCOM_APPL_DATA) bufferSizePtr   //!< Pointer to save Rx buffer size
        )
{
    BufReq_ReturnType result = BUFREQ_E_NOT_OK;
    uint32            Channel = 0u;
    uint32            OtherEndIdx = 0u;

    AnchorComLog(DET_MODULEID_ANCHORCOM, "AnchorCom_CopyRx pdu %d\r\n", id);

    if( FALSE == rbdAnchorComTp_FindByLLPdu( id, &rbdAnchorComTp_LowerRxPdu[0][0], RBDANCHORCOM_TP_NUM_LL_RX, &Channel, &OtherEndIdx ))
    {
        rbdAnchorCom_ReportError( id, RBD_ANCHORCOM_API_TP_RXCOPY, RBD_ANCHORCOM_ERR_PDU );
    }
    else if ((NULL == info) || ( NULL == bufferSizePtr ))   /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3: defensive pointer check */
    {
        rbdAnchorCom_ReportError( id, RBD_ANCHORCOM_API_TP_RXCOPY, RBD_ANCHORCOM_ERR_NULL );
    }
    else
    {
        PduLengthType const Offset      = rbdAnchorComTp_ChRxInfo[Channel][OtherEndIdx].RxOffset;
        /*AXIVION Next Line MisraC2012Directive-4.1 : Offset and SduLength are both smaller than 278, so no overflow risk*/
        PduLengthType const NewLength   = Offset + info->SduLength;

        if( NewLength > RBDANCHORCOM_TP_MAX_RX_BUFFER_LEN)  // We reserved 2 bytes data ID and 4 bytes CMAC
        {
            // Too much data to copy
            result = BUFREQ_E_OVFL;
        }
        else
        {
            // When TP length is bigger than 64, the first function call is with SduDataPtr as NULL, and SduLength as 0
            if (NULL != info->SduDataPtr)
            {
                // Copy in the data
                /*AXIVION Next Line MisraC2012-21.18 : AnchorCom TP Buf is 264 bytes long, longer than max messagelength 263, so no overflow risk*/
                memcpy( &rbdAnchorComTp_ChRxInfo[Channel][OtherEndIdx].RxBuffer[Offset], info->SduDataPtr, info->SduLength );
                rbdAnchorComTp_ChRxInfo[Channel][OtherEndIdx].RxOffset = (uint16)NewLength;
            }

            *bufferSizePtr = RBDANCHORCOM_TP_MAX_RX_BUFFER_LEN - NewLength;
            result = BUFREQ_OK;
        }
    }

    return result;
}


//------------------------------------------------------------------------------
//! \brief  Receive Indication function
//!
//! This function will be called by the lower layer (PduR) when all frames have
//! been received, or an error occurred.
//!
//! If the result is ok (all frames received), we pass the receive message to
//! the upper-layer (application) module.
//!
//! If the result is not ok (error occurred), we simply ignore the message.
//------------------------------------------------------------------------------
FUNC(void, RBDANCHORCOM_CODE) rbdAnchorComTp_TpRxIndication(
    PduIdType id,   //!< PDU ID
    Std_ReturnType result   //!< Result of receiving operation
    )
{
    uint32 Channel = 0u;
    uint32 OtherEndIdx = 0u;

    AnchorComLog(DET_MODULEID_ANCHORCOM, "TpRxIndication id %d result %d\r\n", id, result);

    if( FALSE == rbdAnchorComTp_FindByLLPdu( id, &rbdAnchorComTp_LowerRxPdu[0][0], RBDANCHORCOM_TP_NUM_LL_RX, &Channel, &OtherEndIdx ) )
    {
        rbdAnchorCom_ReportError( id, RBD_ANCHORCOM_API_TP_RXINDICATION, RBD_ANCHORCOM_ERR_PDU );
    }
    else if( E_OK != result )
    {
        // Ignore the message
    }
    else
    {
        uint32 const Length      = rbdAnchorComTp_ChRxInfo[Channel][OtherEndIdx].RxOffset;

        // We need at least one byte of header and one byte of data
        // Also, we need one byte Truncated Freshness and three bytes CMAC
        if( Length >= RBDANCHORCOM_TP_MIN_RX_DATA_LEN)
        {
            AnchorCom_PrintData ("TpRxIndication", &rbdAnchorComTp_ChRxInfo[Channel][OtherEndIdx].RxBuffer[0], rbdAnchorComTp_ChRxInfo[Channel][OtherEndIdx].RxOffset);

            // If there is no data waiting to be verify
            if (FALSE == rbdAnchorComTp_ChRxBuff[Channel][OtherEndIdx].NeedVerify)
            {
	            // For MasterMCU, this function can be called from CAN SPI or IMC
	            // When from CAN SPI, it's inside of interrupt, that means it cannot call functions from HSM in Core B
	            // To make it unify, we copy data to other buffer and process them in cyclic
	            /*AXIVION Next Line MisraC2012-21.18 : max receive data is 284 bytes, the buffer is 288 bytes, so no overyflow */
	            memcpy (&rbdAnchorComTp_ChRxBuff[Channel][OtherEndIdx].RxBuffer[0], &rbdAnchorComTp_ChRxInfo[Channel][OtherEndIdx].RxBuffer[0], rbdAnchorComTp_ChRxInfo[Channel][OtherEndIdx].RxOffset);
	            rbdAnchorComTp_ChRxBuff[Channel][OtherEndIdx].RxOffset   = rbdAnchorComTp_ChRxInfo[Channel][OtherEndIdx].RxOffset;
	            rbdAnchorComTp_ChRxBuff[Channel][OtherEndIdx].PduId      = id;
	            rbdAnchorComTp_ChRxBuff[Channel][OtherEndIdx].NeedVerify = TRUE;

                if (E_NOT_OK == rbdAnchorComTp_AddRxQueue(Channel,OtherEndIdx, FALSE))
                {
                    AnchorComLog(DET_MODULEID_ANCHORCOM, "rbdAnchorComTp_AddRxQueue id %d FAIL\r\n", id);
                    rbdAnchorComTp_ChRxBuff[Channel][OtherEndIdx].NeedVerify = FALSE;
                }
            }
            else if (FALSE == rbdAnchorComTp_ChRxTemp[Channel][OtherEndIdx].NeedVerify)  // Otherwise, save it to temp array
            {
	            /*AXIVION Next Line MisraC2012-21.18 : max receive data is 284 bytes, the buffer is 288 bytes, so no overflow */
                memcpy (&rbdAnchorComTp_ChRxTemp[Channel][OtherEndIdx].RxBuffer[0], &rbdAnchorComTp_ChRxInfo[Channel][OtherEndIdx].RxBuffer[0], rbdAnchorComTp_ChRxInfo[Channel][OtherEndIdx].RxOffset);
                rbdAnchorComTp_ChRxTemp[Channel][OtherEndIdx].RxOffset   = rbdAnchorComTp_ChRxInfo[Channel][OtherEndIdx].RxOffset;
                rbdAnchorComTp_ChRxTemp[Channel][OtherEndIdx].PduId      = id;
                rbdAnchorComTp_ChRxTemp[Channel][OtherEndIdx].NeedVerify = TRUE;

                if (E_NOT_OK == rbdAnchorComTp_AddRxQueue(Channel,OtherEndIdx, TRUE))
                {
                    AnchorComLog(DET_MODULEID_ANCHORCOM, "rbdAnchorComTp_AddRxQueue id %d FAIL\r\n", id);
                    rbdAnchorComTp_ChRxTemp[Channel][OtherEndIdx].NeedVerify = FALSE;
                }
            }
            else
            {
                rbdAnchorComTp_RxQueue_st* pQueue = rbdAnchorComTp_GetRxQueueHead(OtherEndIdx);
                AnchorComLog(DET_MODULEID_ANCHORCOM, "TpRxIndication NO buffer available peer %d queue %x state %d counter %d\r\n",
                    OtherEndIdx, pQueue, pQueue->QueueState, pQueue->ErrCounter);
                // No buffer available, drop it
            }

        }
    }

#if defined(RBDANCHORCOM_XCP_CHANNEL)
    (void)Xcp_Event(RBDANCHORCOM_XCP_CHANNEL);
#endif
}



void rbdAnchorComTp_Init( void )
{
    uint32  OtherEndIdx;
    uint32  MsgIdx;
    uint32  Channel;

    // TP_MSG_TX/TP_MSG_RX are defined at start, doxygen complains about same macro defined twice, so ignore these macros
    /// \cond DO_NOT_DOCUMENT

    // Check for duplicate IDs.
    // This creates a tag name for each ID. If there is a duplicate, there will be a compiler error.
    enum rbdAnchorComTp_checkDuplicates {
        #define TP_MSG_TX(Name,Id,Channel,Length,EncDec,BroadCast,RxIndication)   checkDuplicate_##Id, /*AXIVION Line MisraC2012Directive-4.9 : Macro is used to generate data in rbdAnchorCom_CfgMsg during build */ /*AXIVION Line CertC-PRE02 : Macro is substituted by its expansion */
        #define TP_MSG_RX(Name,Id,Channel,Length,EncDec,BroadCast,RxIndication)   checkDuplicate_##Id, /*AXIVION Line MisraC2012Directive-4.9 : Macro is used to generate data in rbdAnchorCom_CfgMsg during build */ /*AXIVION Line CertC-PRE02 : Macro is substituted by its expansion */
        #include "rbdAnchorCom_CfgMsg.h"    /*AXIVION Line MisraC2012-20.1 MisraC2012-20.3: Header file is used to generate data during build */
        #undef TP_MSG_TX  /*AXIVION Line MisraC2012-20.5 :undef allowed for special macro files */
        #undef TP_MSG_RX  /*AXIVION Line MisraC2012-20.5 :undef allowed for special macro files */
    };

    // Check for invalid length.
    #define TP_MSG_TX(Name,Id,Channel,Length,EncDec,BroadCast,RxIndication)   static_assert((Length)<RBDANCHORCOM_TP_MAX_MSG_TX, "Length error for " #Name ); /*AXIVION Line MisraC2012Directive-4.9 : Macro is used to check data valid during build process */ /*AXIVION Line CertC-PRE11 : Macro is substituted by its expansion */
    #define TP_MSG_RX(Name,Id,Channel,Length,EncDec,BroadCast,RxIndication)   static_assert((Length)<RBDANCHORCOM_TP_MAX_MSG_RX, "Length error for " #Name ); /*AXIVION Line MisraC2012Directive-4.9 : Macro is used to check data valid during build process  */ /*AXIVION Line CertC-PRE11 : Macro is substituted by its expansion */
    #include "rbdAnchorCom_CfgMsg.h"    /*AXIVION Line MisraC2012-20.1 MisraC2012-20.3: Header file is used to generate data during build */
    #undef TP_MSG_TX  /*AXIVION Line MisraC2012-20.5 :undef allowed for special macro files */
    #undef TP_MSG_RX  /*AXIVION Line MisraC2012-20.5 :undef allowed for special macro files */
    /// \endcond

    for( OtherEndIdx=0u; OtherEndIdx<(uint32)RBDANCHORCOM_TP_NUM_LL_TX; OtherEndIdx++ )  /*AXIVION Line CertC-INT00 : Casting same sized integral types*/
    {
        for( MsgIdx=0u; MsgIdx<(uint32)RBDANCHORCOM_TP_NUM_MSG; MsgIdx++ )  /*AXIVION Line CertC-INT00 : Casting same sized integral types*/
        {
            rbdAnchorComTp_MsgTxInfo[OtherEndIdx][MsgIdx].BufState = RBDANCHORCOM_TPSTATE_IDLE;
        }

        for( Channel=0u; Channel<(uint32)RBDANCHORCOM_TPCH_MAX; Channel++) /*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            rbdAnchorComTp_ChTxInfo[Channel][OtherEndIdx].TxInProgress = FALSE;
        }

        rbdAnchorComTp_TxHead[OtherEndIdx] = 0u;
        rbdAnchorComTp_TxTail[OtherEndIdx] = 0u;
    }

    for( OtherEndIdx=0u; OtherEndIdx<(uint32)RBDANCHORCOM_TP_NUM_LL_RX; OtherEndIdx++ ) /*AXIVION Line CertC-INT00 : Casting same sized integral types*/ /*AXIVION Line CertC-STR34 : comparison is of same types*/
    {
        for( Channel=0u; Channel<(uint32)RBDANCHORCOM_TPCH_MAX; Channel++) /*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            rbdAnchorComTp_ChRxBuff[Channel][OtherEndIdx].NeedVerify = FALSE;
            rbdAnchorComTp_ChRxTemp[Channel][OtherEndIdx].NeedVerify = FALSE;
        }

        rbdAnchorComTp_RxHead[OtherEndIdx] = 0u;
        rbdAnchorComTp_RxTail[OtherEndIdx] = 0u;
    }


#ifdef RBDANCHORCOMTP_TIMING
    rbd_ElTime_Accumulate_Init( &rbdAnchorComTp_TxCsmTime_us );
    rbd_ElTime_Accumulate_Init( &rbdAnchorComTp_RxTime_ms );
#endif
}


//-------------------- End of File -------------------------------------------
