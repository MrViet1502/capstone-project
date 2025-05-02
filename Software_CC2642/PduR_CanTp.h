//----------------------------------------------------------------------------
// CONFIDENTIAL
//
// COPYRIGHT Robert Bosch (Australia) Pty. Ltd. 2019.
// All rights reserved, also regarding any disposal, exploitation,
// reproduction, editing, distribution, as well as in the event of
// applications for industrial property rights.
//----------------------------------------------------------------------------

#ifndef PDUR_CANTP_H
#define PDUR_CANTP_H

//============================================================================
// Include Files
//============================================================================
#include "PduR.h"


//============================================================================
// Public Functions
//============================================================================


LOCAL_INLINE BufReq_ReturnType PduR_CanTpCopyTxData(
        PduIdType            Id,
        PduInfoType      *   Info,
        RetryInfoType    *   Retry,
        PduLengthType    *   AvailableDataPtr
        )
{
    return PduR_TpCopyTxData( Id, Info, Retry, AvailableDataPtr );
}

LOCAL_INLINE void PduR_CanTpTxConfirmation(
        PduIdType       Id,
        Std_ReturnType  Result
        )
{
    PduR_TpTxConfirmation( Id, Result );
}

LOCAL_INLINE BufReq_ReturnType PduR_CanTpStartOfReception(
        PduIdType           Id,
        PduInfoType   * Info,
        PduLengthType       TpSduLength,
        PduLengthType *     BufferSizePtr
        )
{
    return PduR_TpStartOfReception( Id, Info, TpSduLength, BufferSizePtr );
}

LOCAL_INLINE BufReq_ReturnType PduR_CanTpCopyRxData(
        PduIdType           Id,
        PduInfoType   *     Info,
        PduLengthType *     BufferSizePtr
        )
{
    return PduR_TpCopyRxData( Id, Info, BufferSizePtr );
}

LOCAL_INLINE void PduR_CanTpRxIndication(
        PduIdType       Id,
        Std_ReturnType  Result
        )
{
    PduR_TpRxIndication( Id, Result );
}

LOCAL_INLINE Std_ReturnType PduR_CanTpTransmit(
        PduIdType          TxPduId,
        const PduInfoType * PduInfoPtr
        )
{
    return PduR_TpTransmit( TxPduId, PduInfoPtr );
}




#endif /* PDUR_CANTP_H */

