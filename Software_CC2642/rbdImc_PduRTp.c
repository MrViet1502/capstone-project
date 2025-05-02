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



//============================================================================
//  Includes
//============================================================================

#include <stdlib.h>     // for NULL
#include "assert.h"

#include "rbdImc.h"
#include "rbdImc_PduR.h"
#include "rbdImc_Private.h"
#include "rbdImc_Cb.h"
#include "ImcIf_Api.h"
#include "rbdImc_PduR_Types.h"
#include "rbdMacros.h"

#if defined(RBD_PK_MASTER_MCU)
#include "rbdImcTp.h"
#include "PduR_rbdImcTp.h"
#include "Rte_rbdImc.h"
#include "Os.h"
#else
#include "PduR.h"
#endif

#include "Det.h"
#include "Det_rbModuleId.h"
#include "Det_rbApi.h"

#include "DimOf.h"



//============================================================================
// Local Macros
//============================================================================



#define RBDIMCTP_TRACE_BUF     STD_OFF   //!< Trace sent/received messages

// Macros for thread safety
// The potential unsafe operation is in the transmit direction, because the functions
// can be called from PduR and from the cyclic task.
// - rbdImcTp_Transmit(), starting a new transmission
// - rbdImcTp_CancelTransmit(), cancelling transmission
// - rbdImc_TpCyclic(), actually doing the transmission.
//
// In the receive direction, there should not be any thread problem, because all receive functions
// are called from the ImcIf RxIndication callback.
//
// Under OSEK / Autosar OS, the resource must be added to every task or interrupt which
// calls these functions. That should be:
// - SecOC cyclic call
// - rbdUdc cyclic call
#if defined(RBD_PK_CPU_SPC58ECx)
    #define RBDIMCTP_MUTEX_ACQUIRE()        SuspendAllInterrupts()
    #define RBDIMCTP_MUTEX_RELEASE()        ResumeAllInterrupts()
#else
    #define RBDIMCTP_MUTEX_ACQUIRE()        /*AXIVION Line MisraC2012Directive-4.9 : This Macro is used in ST Chorus, so set it empty in TI BLE */
    #define RBDIMCTP_MUTEX_RELEASE()        /*AXIVION Line MisraC2012Directive-4.9 : This Macro is used in ST Chorus, so set it empty in TI BLE */
#endif


//============================================================================
// Local Types
//============================================================================





//============================================================================
// Local Function Prototypes
//============================================================================




//============================================================================
// Local Variables
//============================================================================





//============================================================================
// Function declarations.
//============================================================================
/// \cond DO_NOT_DOCUMENT

#if !defined(RBD_PK_MASTER_MCU)
// Replacement for RTE-generated code
LOCAL_INLINE BufReq_ReturnType PduR_rbdImcTpCopyTxData(PduIdType id, PduInfoType* info, RetryInfoType* retry, PduLengthType* avail)
{
    return PduR_TpCopyTxData( id, info, retry, avail );
}

LOCAL_INLINE void PduR_rbdImcTpTxConfirmation(PduIdType id, Std_ReturnType result)
{
    PduR_TpTxConfirmation( id, result );
}

LOCAL_INLINE BufReq_ReturnType PduR_rbdImcTpStartOfReception(PduIdType id, PduInfoType* info, PduLengthType length, PduLengthType* size)
{
    return PduR_TpStartOfReception(id, info, length, size);
}

LOCAL_INLINE BufReq_ReturnType PduR_rbdImcTpCopyRxData(PduIdType id, PduInfoType* info, PduLengthType* size)
{
    return PduR_TpCopyRxData(id, info, size);
}

LOCAL_INLINE void PduR_rbdImcTpRxIndication(PduIdType id, Std_ReturnType result)
{
    PduR_TpRxIndication(id, result);
}
#endif

/// \endcond

//----------------------------------------------------------------------------
//! \brief  Output messages to the debug log / UART
//----------------------------------------------------------------------------
static void rbdImc_TpPrintBuf(char const * const comment, uint32 PduIdx, uint16_t len, uint8 const * const buf)
{
#if STD_ON == RBDIMCTP_TRACE_BUF
    uint32 i;

    // Start with a comment
    Det_RbLog(DET_MODULEID_IMC, "%s (%i)", comment, PduIdx );

    if( len>16 )
    {
        len = 16;
    }
    for (i = 0; i < len; i++)
    {
        Det_RbLog(DET_MODULEID_IMC, " %02x", buf[i]);
    }

    Det_RbLog(DET_MODULEID_IMC, "\r\n");
#else
    RBD_UNUSED(comment);
    RBD_UNUSED(PduIdx);
    RBD_UNUSED(len);
    RBD_UNUSED(buf);
#endif
}



//----------------------------------------------------------------------------
//! \brief  Find the information for a Pdu
//!
//! This function searches for the Pdu information, indexed by the Imc ID.
//!
//! \return  Index of the channel, or RBDIMC_TP_CHANNEL_MAX if not found.
//----------------------------------------------------------------------------
static uint32 rbdImc_TpFindByImcId( ImcIf_MsgType_et const id )
{
    uint32 retval = rbdImc_TpCfg.NumMsg;
    uint32 chId;

    for( chId=0; chId<rbdImc_TpCfg.NumMsg; chId++ )
    {
        if( id == rbdImc_TpCfg.MsgCfg[chId].ImcPduId )/*AXIVION Line MisraC2012-18.1: will not be out of bounds */
        {
            retval = chId;
            break;
        }
    }

    return retval;
}


//----------------------------------------------------------------------------
//! \brief  Find the information for a Pdu
//!
//! This function searches for the Pdu information, indexed by the PduR ID.
//!
//! \return  Index of the channel, or RBDIMC_TP_CHANNEL_MAX if not found.
//----------------------------------------------------------------------------
static uint32 rbdImc_TpFindByPdurTxId( PduIdType const id )
{
    uint32 retval = rbdImc_TpCfg.NumMsg;
    uint32 chId;

    for( chId=0; chId<rbdImc_TpCfg.NumMsg; chId++ )
    {
        if( id == rbdImc_TpCfg.MsgCfg[chId].UpperTxFromPduR )/*AXIVION Line MisraC2012-18.1: will not be out of bounds */
        {
            retval = chId;
            break;
        }
    }

    return retval;
}

//----------------------------------------------------------------------------
//! \brief TP message transmission
//!
//! \retval  E_OK      Transmission accepted
//! \retval  E_NOT_OK  Transmission rejected
//----------------------------------------------------------------------------
Std_ReturnType rbdImcTp_Transmit(PduIdType TxPduId, PduInfoType const * PduInfoPtr)
{
    uint32 const   PduIdx  = rbdImc_TpFindByPdurTxId( TxPduId );
    Std_ReturnType result = E_NOT_OK;

    static_assert(sizeof(rbdImc_TpCfg.TxInfo[PduIdx].bytesRemaining) <= sizeof(PduInfoPtr->SduLength), "Length Mismatch");

    if( NULL == PduInfoPtr )    /*AXIVION Line MisraC2012-14.3 MisraC2012-2.2: defensive input parameter checking */
    {
        (void)Det_ReportError( DET_MODULEID_IMC, 0, RBDIMCPDUR_TP_TX, RBDIMCPDUR_ERR_NULL ); /*AXIVION Line MisraC2012-2.2: could be configured as no function */
    }
    else if( PduIdx >= rbdImc_TpCfg.NumMsg  )
    {
        (void)Det_ReportError( DET_MODULEID_IMC, (uint8)TxPduId, RBDIMCPDUR_TP_TX, RBDIMCPDUR_ERR_PDU ); /*AXIVION Line MisraC2012-2.2: could be configured as no function */
    }
    else
    {
        // Thread protection
        RBDIMCTP_MUTEX_ACQUIRE();

        rbdImc_TpCfg.TxInfo[PduIdx].sequence       = 0;
        rbdImc_TpCfg.TxInfo[PduIdx].bytesRemaining = (uint16)PduInfoPtr->SduLength;
        rbdImc_TpCfg.TxInfo[PduIdx].offset         = 0;
        result = E_OK;

        RBDIMCTP_MUTEX_RELEASE();
    }

    return result;
}



//----------------------------------------------------------------------------
//! \brief  TP message cancellation
//!
//! \retval  E_OK      Cancellation accepted
//! \retval  E_NOT_OK  Cancellation rejected
//----------------------------------------------------------------------------
Std_ReturnType rbdImcTp_CancelTransmit(PduIdType CanTxPduId)
{
    uint32 const   PduIdx  = rbdImc_TpFindByPdurTxId( CanTxPduId );
    Std_ReturnType result = E_NOT_OK;

    if( PduIdx >= rbdImc_TpCfg.NumMsg  )
    {
        (void)Det_ReportError( DET_MODULEID_IMC, (uint8)CanTxPduId, RBDIMCPDUR_TP_CANCEL, RBDIMCPDUR_ERR_PDU ); /*AXIVION Line MisraC2012-2.2: could be configured as no function */
    }
    else if( 0u == rbdImc_TpCfg.TxInfo[PduIdx].bytesRemaining )
    {
        // No transfer in progress
    }
    else
    {
        // No thread / protection / mutex is required here, because it is a single, atomic write.

        rbdImc_TpCfg.TxInfo[PduIdx].bytesRemaining = 0;
        result = E_OK;
    }

    return result;
}



//----------------------------------------------------------------------------
//! \brief  Report an error to both Det and PduR
//----------------------------------------------------------------------------
static void rbdImc_TpReportRxError(
        uint32 const PduIdx,
        uint8  const Instance,
        uint8  const Service,
        uint8  const Error
        )
{
#if DET_ERROR == STD_OFF
    // Misra complains about unused parameters when Det_ReportRuntimeError is empty
    RBD_UNUSED(Instance);
    RBD_UNUSED(Service);
    RBD_UNUSED(Error);
#endif

    (void)Det_ReportRuntimeError( DET_MODULEID_IMC, Instance, Service, Error ); /*AXIVION Line MisraC2012-2.2: could be configured as no function */

    if( TRUE == rbdImc_TpCfg.RxInfo[PduIdx].active )
    {
        rbdImc_TpCfg.RxInfo[PduIdx].active = FALSE;
        (void)PduR_rbdImcTpRxIndication( rbdImc_TpCfg.MsgCfg[PduIdx].UpperRxPduId, E_NOT_OK );
    }
}


//----------------------------------------------------------------------------
//! \brief  Handle the first frame of a TP reception
//----------------------------------------------------------------------------
static void rbdImc_TpRxFirstFrame(
        uint32 const     PduIdx,
        rbdImc_Length_t  const   Length,
        uint8  const * Msg
        )
{
    uint16 const totalMsgLen = ((uint16)Msg[1u] << 8u) + Msg[2u];
    uint16 const bytesThisFrame = Length - 3u;  /*AXIVION Line MisraC2012Directive-4.1, CertC-INT08: before call this function, Length is checked to be bigger than 3, no overflow risk */

    if( bytesThisFrame > totalMsgLen )
    {
        rbdImc_TpReportRxError( PduIdx, (uint8)bytesThisFrame, RBDIMCPDUR_TP_RX, RBDIMCPDUR_ERR_LENGTH );
    }
    else
    {
        PduInfoType       info;
        BufReq_ReturnType PduRResult;
        PduLengthType     BufferSize;

        // Notify the start
        info.SduLength  = bytesThisFrame;
        info.SduDataPtr = NULL;
        PduRResult = PduR_rbdImcTpStartOfReception(
                rbdImc_TpCfg.MsgCfg[PduIdx].UpperRxPduId,
                &info,
                totalMsgLen,
                &BufferSize
                );

        if( PduRResult == BUFREQ_OK ) /*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            // Copy the first packet of data
            info.SduLength  = bytesThisFrame;
            info.SduDataPtr = (SduDataPtrType)&Msg[3]; /*AXIVION Line MisraC2012-11.8: cast is needed to meet function parameter requirement*/
            PduRResult = PduR_rbdImcTpCopyRxData(
                    rbdImc_TpCfg.MsgCfg[PduIdx].UpperRxPduId,
                    &info,
                    &BufferSize
                    );
        }

        if( PduRResult == BUFREQ_OK )  /*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            rbdImc_TpCfg.RxInfo[PduIdx].bytesRemaining = totalMsgLen - bytesThisFrame;    /*AXIVION Line MisraC2012Directive-4.1, CertC-INT08: frame is smaller than total length, no overflow risk */
            rbdImc_TpCfg.RxInfo[PduIdx].sequence = 1u;      // Next sequence number
            rbdImc_TpCfg.RxInfo[PduIdx].active   = TRUE;   // mark transfer in progress
        }
        else
        {
            rbdImc_TpCfg.RxInfo[PduIdx].active   = FALSE;      // mark as idle
        }
    }
}


//----------------------------------------------------------------------------
//! \brief  Handle consecutive frames of a TP reception
//----------------------------------------------------------------------------
static void rbdImc_TpRxConsecutiveFrame(
        uint32           const   PduIdx,
        rbdImc_Length_t  const   Length,
        uint8 const * Msg
        )
{
    uint16 const bytesThisFrame = Length - 1u;

    if( (Msg[0u] & 0xFu) != (rbdImc_TpCfg.RxInfo[PduIdx].sequence & 0xFu) )
    {
        rbdImc_TpReportRxError( PduIdx, Msg[0], RBDIMCPDUR_TP_RX, RBDIMCPDUR_ERR_SEQUENCE );
    }
    else if( bytesThisFrame > rbdImc_TpCfg.RxInfo[PduIdx].bytesRemaining )
    {
        rbdImc_TpReportRxError( PduIdx, (uint8)bytesThisFrame, RBDIMCPDUR_TP_RX, RBDIMCPDUR_ERR_LENGTH );
    }
    else
    {
        PduInfoType       info;
        BufReq_ReturnType PduRResult;
        PduLengthType     BufferSize;

        info.SduLength  = bytesThisFrame;
        info.SduDataPtr = (SduDataPtrType)&Msg[1]; /*AXIVION Line MisraC2012-11.8: cast is needed to meet function parameter requirement*/
        PduRResult = PduR_rbdImcTpCopyRxData(
                rbdImc_TpCfg.MsgCfg[PduIdx].UpperRxPduId,
                &info,
                &BufferSize
                );

        if( PduRResult == BUFREQ_OK ) /*AXIVION Line CertC-STR34 : comparison is of same types*/
        {
            rbdImc_TpCfg.RxInfo[PduIdx].bytesRemaining -= bytesThisFrame; /*AXIVION Line MisraC2012Directive-4.1, CertC-INT08: bytesThisFrame is checked to be smaller than bytesRemaining, no overflow risk */
            rbdImc_TpCfg.RxInfo[PduIdx].sequence++;      // Next sequence number
        }
        else
        {
            rbdImc_TpReportRxError( PduIdx, (uint8)bytesThisFrame, RBDIMCPDUR_TP_RX, RBDIMCPDUR_ERR_RESULT );
        }
    }
}



//----------------------------------------------------------------------------
//! \brief  Rx Indication
//!
//! This function translates the PduId from Imc to PduR numbers.
//----------------------------------------------------------------------------
void rbdImc_TpRxIndication(
        ImcIf_MsgType_et const   MsgId,
        rbdImc_Length_t  const   Length,
        uint8            const * Msg,
        rbd_ElTime_Value_t const timestamp
        )
{
    uint32 const PduIdx  = rbdImc_TpFindByImcId( MsgId );

    RBD_UNUSED(timestamp);

    if( PduIdx >= rbdImc_TpCfg.NumMsg  )
    {
        (void)Det_ReportError( DET_MODULEID_IMC, (uint8)MsgId, RBDIMCPDUR_TP_RX, RBDIMCPDUR_ERR_PDU ); /*AXIVION Line MisraC2012-2.2: could be configured as no function */
    }
    else if( NULL == Msg )
    {
        (void)Det_ReportError( DET_MODULEID_IMC, (uint8)MsgId, RBDIMCPDUR_TP_RX, RBDIMCPDUR_ERR_NULL ); /*AXIVION Line MisraC2012-2.2: could be configured as no function */
    }
    else if(   (Length < 1u)                     /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3: defensive code for input checking */
            || ((0u==Msg[0u]) && (Length<3u))      /*AXIVION Line MisraC2012-2.2 MisraC2012-14.3: defensive code for input checking */
           )
    {
        // Received message is shorter than possible
        rbdImc_TpReportRxError( PduIdx, (uint8)MsgId, RBDIMCPDUR_TP_RX, RBDIMCPDUR_ERR_LENGTH );
    }
    else
    {
        // Message appears valid
        if( 0u == Msg[0u] )
        {
            // First frame
            /*AXIVION Next Line MisraC2012-2.2 MisraC2012-14.3 : The function is empty in some compilations, but used in others */
            rbdImc_TpPrintBuf( "ImcTp Rx FF", PduIdx, Length, Msg );   // Tracing for debug
            rbdImc_TpRxFirstFrame( PduIdx, Length, Msg );
        }
        else
        {
            // Not the first frame
            /*AXIVION Next Line MisraC2012-2.2 MisraC2012-14.3 : The function is empty in some compilations, but used in others */
            rbdImc_TpPrintBuf( "ImcTp Rx CF", PduIdx, Length, Msg );   // Tracing for debug
            rbdImc_TpRxConsecutiveFrame( PduIdx, Length, Msg );
        }

        if( (0u==rbdImc_TpCfg.RxInfo[PduIdx].bytesRemaining) && (TRUE == rbdImc_TpCfg.RxInfo[PduIdx].active) )
        {
            // End of transfer
            rbdImc_TpCfg.RxInfo[PduIdx].active = FALSE;
            rbdImc_TpCfg.RxInfo[PduIdx].sequence=0u;

            (void)PduR_rbdImcTpRxIndication( rbdImc_TpCfg.MsgCfg[PduIdx].UpperRxPduId, E_OK );
        }
    }
}


//----------------------------------------------------------------------------
//! \brief  Tx Confirmation
//!
//! Mark the packet as sent. If this was the final packet, send the confirmation.
//----------------------------------------------------------------------------
void rbdImc_TpTxConfirmation(
        ImcIf_MsgType_et const   MsgId
        )
{
    uint32 const PduIdx  = rbdImc_TpFindByImcId( MsgId );

    if( PduIdx >= rbdImc_TpCfg.NumMsg  )
    {
        (void)Det_ReportError( DET_MODULEID_IMC, (uint8)MsgId, RBDIMCPDUR_TP_RX, RBDIMCPDUR_ERR_PDU ); /*AXIVION Line MisraC2012-2.2: could be configured as no function */
    }
    else
    {
        if( rbdImc_TpCfg.TxInfo[PduIdx].numPktsPending > 0u)
        {
            rbdImc_TpCfg.TxInfo[PduIdx].numPktsPending--;
        }

        if( (0u==rbdImc_TpCfg.TxInfo[PduIdx].numPktsPending) && (0u==rbdImc_TpCfg.TxInfo[PduIdx].bytesRemaining) )
        {
            // This was the last packet, so send a transmit confirmation
            PduR_rbdImcTpTxConfirmation( rbdImc_TpCfg.MsgCfg[PduIdx].UpperTxToPduR, E_OK );
        }
    }
}


//----------------------------------------------------------------------------
//! \brief  Cyclic function to send the next TP message
//!
//! If a transfer is in progress, this function tries to send the next packet.
//----------------------------------------------------------------------------
void rbdImc_TpCyclic( void )
{
    uint32 PduIdx;

    // Thread protection
    RBDIMCTP_MUTEX_ACQUIRE();

    // Loop over every channel
    for( PduIdx=0u; PduIdx<rbdImc_TpCfg.NumMsg; PduIdx++ )
    {
        // Only process channels with active transmissions
        /*AXIVION Next Line MisraC2012-18.1: will not be out of bounds */
        if( rbdImc_TpCfg.TxInfo[PduIdx].bytesRemaining > 0u )
        {
            rbdImc_Channel_et Channel = IMC_CH_COUNT;

            // Get a buffer
            /*AXIVION Next Line MisraC2012-18.1: will not be out of bounds */
            ImcIf_TxQueue_st * const QueueItem = ImcIf_GetBuffer( rbdImc_TpCfg.MsgCfg[PduIdx].ImcPduId, IMCIF_MSG_MAX_LEN, &Channel );

            if( NULL != QueueItem )
            {
                PduLengthType availableData;
                PduInfoType   PduInfo;
                uint32        bytesThisPacket;
                uint32        pos = 0u;

                /*AXIVION Next Line MisraC2012-18.1: will not be out of bounds */
                QueueItem->Msg[pos] = rbdImc_TpCfg.TxInfo[PduIdx].sequence & 0xFu;
                /*AXIVION Next Line MisraC2012-18.1: will not be out of bounds */
                if( 0u != rbdImc_TpCfg.TxInfo[PduIdx].sequence )
                {
                    // Mark consecutive frame
                    QueueItem->Msg[pos] |= 0x20u;
                }
                pos++;
                /*AXIVION Next Line MisraC2012-18.1: will not be out of bounds */
                if( 0u == rbdImc_TpCfg.TxInfo[PduIdx].sequence )
                {
                    // First packet includes the number of bytes
                    /*AXIVION Next Line MisraC2012-18.1: will not be out of bounds */
                    QueueItem->Msg[pos] = (uint8)(rbdImc_TpCfg.TxInfo[PduIdx].bytesRemaining>> 8u);
                    pos++;
                    /*AXIVION Next Line MisraC2012-18.1: will not be out of bounds */
                    QueueItem->Msg[pos] = (uint8)(rbdImc_TpCfg.TxInfo[PduIdx].bytesRemaining & 0xFFu);
                    pos++;
                }
                else
                {
                    // Mark consecutive frame

                }

                // Calculate the number of bytes to send in this packet
                bytesThisPacket = IMCIF_MSG_MAX_LEN - pos;
                /*AXIVION Next Line MisraC2012-18.1: will not be out of bounds */
                if( bytesThisPacket > rbdImc_TpCfg.TxInfo[PduIdx].bytesRemaining )
                {
                    // We can't send more bytes than are in the data stream
                    /*AXIVION Next Line MisraC2012-18.1: will not be out of bounds */
                    bytesThisPacket = rbdImc_TpCfg.TxInfo[PduIdx].bytesRemaining;
                }

                PduInfo.SduLength = (uint16)bytesThisPacket;
                PduInfo.SduDataPtr = &QueueItem->Msg[pos];
                /*AXIVION Next Line MisraC2012-18.1: will not be out of bounds */
                if( BUFREQ_OK == PduR_rbdImcTpCopyTxData(rbdImc_TpCfg.MsgCfg[PduIdx].UpperTxToPduR,&PduInfo,NULL,&availableData ) )  /*AXIVION Line CertC-STR34 : comparison is of same types*/
                {
                    pos += bytesThisPacket;
                    QueueItem->Length = (uint16)pos;    /*AXIVION Line MisraC2012Directive-4.1, CertC-INT08: max value of pos is TP packet, no truncate value risk */

                    /*AXIVION Next Line MisraC2012-18.1: will not be out of bounds */
                    rbdImc_TpCfg.TxInfo[PduIdx].sequence++;
                    /*AXIVION Next Line MisraC2012-18.1: will not be out of bounds */
                    rbdImc_TpCfg.TxInfo[PduIdx].bytesRemaining -= (uint16)bytesThisPacket;  /*AXIVION Line MisraC2012Directive-4.1, CertC-INT08: bytesThisPacket is checked to be not bigger than bytesRemaining, no truncate value risk */
                    /*AXIVION Next Line MisraC2012-18.1: will not be out of bounds */
                    rbdImc_TpCfg.TxInfo[PduIdx].numPktsPending++;

                    /*AXIVION Next Line MisraC2012-2.2 MisraC2012-14.3 : The function is empty in some compilations, but used in others */
                    rbdImc_TpPrintBuf( "ImcTp Tx", PduIdx, QueueItem->Length, QueueItem->Msg );   // Tracing for debug
                    ImcIf_QueueAdd(Channel, QueueItem);
                }
            }
        }
    }

    RBDIMCTP_MUTEX_RELEASE();
}


//-------------------- End of File -------------------------------------------
