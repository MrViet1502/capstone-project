//----------------------------------------------------------------------------
// CONFIDENTIAL
//
// COPYRIGHT Robert Bosch (Australia) Pty. Ltd. 2019.
// All rights reserved, also regarding any disposal, exploitation,
// reproduction, editing, distribution, as well as in the event of
// applications for industrial property rights.
//----------------------------------------------------------------------------

//============================================================================
// Include Files
//============================================================================
#include "PduR.h"
#include "PduR_Private.h"

//============================================================================
// Function Definitions.
//============================================================================

//-----------------------------------------------------------------------------------------------------------------
//! \brief  This function is called by the upper layer to start transmitting a tp frame.
//!
//! \param  id         Identification of the PDU.
//! \param  info       Length of and pointer to the PDU data.
//!
//! \retval  E_OK           Transmit request has been accepted.
//!          E_NOT_OK       Transmit request has not been accepted.
//-----------------------------------------------------------------------------------------------------------------
Std_ReturnType PduR_TpTransmit(PduIdType id, const PduInfoType* info)
{
    Std_ReturnType result = E_NOT_OK;

    /*AXIVION Next Line MisraC2012-2.2 MisraC2012-14.3: defensive checking */
    if ((id < (PduIdType)PDUR_PDU_TP_TX_MAX) && (NULL_PTR!=PduR_Config) && (PduR_Config->TpRouteTblTx[id].LoTpTransmit != NULL_PTR))
    {
        result = PduR_Config->TpRouteTblTx[id].LoTpTransmit(PduR_Config->TpRouteTblTx[id].LoTxPduId, info);
    }

    return result;
}

//-----------------------------------------------------------------------------------------------------------------
//! \brief  This function is called by the upper layer to cancel a tp frame transmission
//!
//! \param  id         Identification of the PDU.
//!
//! \retval  E_OK           Transmit request has been accepted.
//!          E_NOT_OK       Transmit request has not been accepted.
//-----------------------------------------------------------------------------------------------------------------
Std_ReturnType PduR_TpCancelTransmit(PduIdType id)
{
    Std_ReturnType result = E_NOT_OK;

    /*AXIVION Next Line MisraC2012-2.2 MisraC2012-14.3: defensive checking */
    if ((id < (PduIdType)PDUR_PDU_TP_TX_MAX) && (NULL_PTR!=PduR_Config) && (PduR_Config->TpRouteTblTx[id].LoTpCancelTransmit != NULL_PTR))
    {
        result = PduR_Config->TpRouteTblTx[id].LoTpCancelTransmit( PduR_Config->TpRouteTblTx[id].LoTxPduId );
    }

    return result;
}

//-----------------------------------------------------------------------------------------------------------------
//! \brief  This function is called by the low layer to get consequent transmission frame
//!
//! \param  id         Identification of the PDU.
//! \param  info       Length of and pointer to the PDU data.
//! \param  retry      Retry request.
//! \param  avail      Available data.
//!
//! \retval  BUFREQ_OK            No errors
//!          BUFREQ_E_NOT_OK      Error occurred.
//-----------------------------------------------------------------------------------------------------------------
BufReq_ReturnType PduR_TpCopyTxData(PduIdType id, PduInfoType* info, RetryInfoType* retry, PduLengthType* avail)
{
    BufReq_ReturnType result = BUFREQ_E_NOT_OK;

    /*AXIVION Next Line MisraC2012-2.2 MisraC2012-14.3: defensive checking */
    if ((id < (PduIdType)PDUR_PDU_TP_TX_MAX) && (NULL_PTR!=PduR_Config) && (PduR_Config->TpRouteTblTx[id].UpTpCopyTxData != NULL_PTR))
    {
        result = PduR_Config->TpRouteTblTx[id].UpTpCopyTxData(PduR_Config->TpRouteTblTx[id].UpTxPduId, info, retry, avail);
    }

    return result;
}

//-----------------------------------------------------------------------------------------------------------------
//! \brief  This function is called by the lower layer to confirm a tp frame transmition.
//!
//! \param  id         Identification of the PDU.
//! \param  result     Tp TxConfirmation result.
//-----------------------------------------------------------------------------------------------------------------
void PduR_TpTxConfirmation(PduIdType id, Std_ReturnType result)
{
    if ((id < (PduIdType)PDUR_PDU_TP_TX_MAX) && (NULL_PTR!=PduR_Config) && (PduR_Config->TpRouteTblTx[id].UpTpTxConfirmation != NULL_PTR))
    {
        PduR_Config->TpRouteTblTx[id].UpTpTxConfirmation(PduR_Config->TpRouteTblTx[id].UpTxPduId, result);
    }
}

BufReq_ReturnType PduR_TpStartOfReception(PduIdType id, PduInfoType* info, PduLengthType length, PduLengthType* size)
{
    BufReq_ReturnType result = BUFREQ_E_NOT_OK;

    /*AXIVION Next Line MisraC2012-2.2 MisraC2012-14.3: defensive checking */
    if ((id < (PduIdType)PDUR_PDU_TP_RX_MAX) && (NULL_PTR!=PduR_Config) && (PduR_Config->TpRouteTblRx[id].UpTpStartOfReception != NULL_PTR))
    {
        result = PduR_Config->TpRouteTblRx[id].UpTpStartOfReception(PduR_Config->TpRouteTblRx[id].UpRxPduId, info, length, size);
    }

    return result;
}

BufReq_ReturnType PduR_TpCopyRxData(PduIdType id, PduInfoType* info, PduLengthType* size)
{
    BufReq_ReturnType result = BUFREQ_E_NOT_OK;

    /*AXIVION Next Line MisraC2012-2.2 MisraC2012-14.3: defensive checking */
    if ((id < (PduIdType)PDUR_PDU_TP_RX_MAX) && (NULL_PTR!=PduR_Config) && (PduR_Config->TpRouteTblRx[id].UpTpCopyRxData != NULL_PTR))
    {
        result = PduR_Config->TpRouteTblRx[id].UpTpCopyRxData(PduR_Config->TpRouteTblRx[id].UpRxPduId, info, size);
    }

    return result;
}

void PduR_TpRxIndication(PduIdType id, Std_ReturnType result)
{
    /*AXIVION Next Line MisraC2012-2.2 MisraC2012-14.3: defensive checking */
    if ((id < (PduIdType)PDUR_PDU_TP_RX_MAX) && (NULL_PTR!=PduR_Config) && (PduR_Config->TpRouteTblRx[id].UpTpRxIndication != NULL_PTR))
    {
        PduR_Config->TpRouteTblRx[id].UpTpRxIndication(PduR_Config->TpRouteTblRx[id].UpRxPduId, result);
    }
}



